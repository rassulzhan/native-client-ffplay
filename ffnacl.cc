
#include <cassert>
#include <cstdio>

#include <string>

#include "ppapi/cpp/input_event.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/var.h"
#include "ppapi/utility/completion_callback_factory.h"

#include <SDL.h>
#include <SDL_nacl.h>

extern "C" {
	#include "ffplay.h"
}


namespace ffnacl
{

const char* const kMethodID_load        = "load";
const char* const kMethodID_play        = "play";
const char* const kMethodID_pause       = "pause";
const char* const kMethodID_fastforward = "fastforward";
const char* const kMethodID_rewind      = "rewind";
const char* const kMethodID_next        = "next";
const char* const kMethodID_prev        = "prev";


class FFnaclInst : public pp::Instance
{
private:
	static int 	m_num_instances;
	pthread_t  	m_ff_thread;
	int 		m_num_changed_view;
	int 		m_width;
	int 		m_height;
    std::string m_filename;
	bool		m_resReady;
	pp::CompletionCallbackFactory<FFnaclInst> m_cc_factory;

public:

	explicit FFnaclInst(PP_Instance instance)
	: pp::Instance(instance)
	, m_ff_thread(0)
	, m_num_changed_view(0)
	, m_width(0)
	, m_height(0)
	, m_filename("")
	, m_resReady(true)
	, m_cc_factory(this)
	{
	    printf("%s ===START===\n", __PRETTY_FUNCTION__);
		RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE|
						   PP_INPUTEVENT_CLASS_KEYBOARD);
		++m_num_instances;
		assert (m_num_instances == 1);
		printf("%s === END ===\n", __PRETTY_FUNCTION__);
	}

	virtual ~FFnaclInst()
	{
		if (m_ff_thread)
		{
			pthread_join(m_ff_thread, NULL);
		}
	}

	virtual bool Init(uint32_t argc, const char* argn[], const char* argv[])
	{
	    printf("%s ===START===\n", __PRETTY_FUNCTION__);
        for(int i = 0; i < argc; ++i)
        {
            printf("\targ[%d]={%s:%s}\n", i, argn[i], argv[i]);
            if (strcmp(argn[i], "width") == 0)
                m_width = atoi(argv[i]);
            if (strcmp(argn[i], "height") == 0)
                m_height = atoi(argv[i]);
            if (strcmp(argn[i], "filename") == 0)
                m_filename = argv[i];
        }
        printf("%s === END ===\n", __PRETTY_FUNCTION__);
		return true;
	}

	// This crucial function forwards PPAPI events to SDL.
	virtual bool HandleInputEvent(const pp::InputEvent& ppevent)
	{
	    //printf("%s ===START===\n", __PRETTY_FUNCTION__);
	    PP_InputEvent_Type type = ppevent.GetType();
        if (type == PP_INPUTEVENT_TYPE_KEYDOWN ||
                 type == PP_INPUTEVENT_TYPE_KEYUP ||
                 type == PP_INPUTEVENT_TYPE_CHAR)
        {
            pp::InputEvent *input_event = const_cast<pp::InputEvent*>(&ppevent);
            pp::KeyboardInputEvent *keyboard_event = reinterpret_cast<pp::KeyboardInputEvent*>(input_event);
            printf("keytype=%d\n", keyboard_event->GetType());
            printf("keymod=%d\n", keyboard_event->GetModifiers());
            printf("keycode=%d\n", keyboard_event->GetKeyCode());
            printf("keytext=%s\n", keyboard_event->GetCharacterText().AsString().c_str());
        }

	    SDL_NACL_PushEvent(ppevent);
	    //printf("%s === END ===\n", __PRETTY_FUNCTION__);
		return true;
	}

	virtual void DidChangeView(const pp::Rect& position, const pp::Rect& clip)
	{
        printf("%s ===START===\n", __PRETTY_FUNCTION__);
        printf("\t(w,h)=(%d,%d) (cw,ch)=(%d,%d)\n", m_width, m_height, clip.width(), clip.height());

		++m_num_changed_view;
		if (m_num_changed_view > 1)
			return;

		SDL_NACL_SetInstance(pp_instance(), m_width, m_height);
		SDL_Init(SDL_INIT_TIMER|SDL_INIT_AUDIO|SDL_INIT_VIDEO);
		StartFFplayInNewThread(0);
        printf("%s === END ===\n", __PRETTY_FUNCTION__);
	}

	void HandleMessage(const pp::Var& var_msg)
	{
        if (!var_msg.is_string())
            return;

        pp::Var ret_var;
        std::string str_msg = var_msg.AsString();
        printf("%s:%s\n", __PRETTY_FUNCTION__, str_msg.c_str());


        if (str_msg == kMethodID_pause) {
            printf("PAUSE\n");
            _fireKeyEvt(PP_INPUTEVENT_TYPE_KEYDOWN, 0);
            _fireKeyEvt(PP_INPUTEVENT_TYPE_CHAR, 0);
            _fireKeyEvt(PP_INPUTEVENT_TYPE_KEYUP, 0);

        } else if (str_msg == kMethodID_rewind) {
            printf("RW\n");
            _fireKeyEvt(PP_INPUTEVENT_TYPE_KEYDOWN, 37);
            _fireKeyEvt(PP_INPUTEVENT_TYPE_KEYUP, 37);
        } else if (str_msg == kMethodID_fastforward) {
            printf("FF\n");
            _fireKeyEvt(PP_INPUTEVENT_TYPE_KEYDOWN, 39);
            _fireKeyEvt(PP_INPUTEVENT_TYPE_KEYUP, 39);
        }

        //PostMessage(ret_var);
	}

private:
	void _fireKeyEvt(PP_InputEvent_Type type, uint32_t keycode)
	{
        pp::KeyboardInputEvent evt(this,
                                   PP_INPUTEVENT_TYPE_KEYDOWN,
                                   pp::Module::Get()->core()->GetTimeTicks(),
                                   0,
                                   keycode,
                                   NULL);
        SDL_NACL_PushEvent(evt);
	}
	void StartFFplayInNewThread(int32_t dummy)
	{
	    printf("%s ===START===\n", __PRETTY_FUNCTION__);

		if (m_resReady)
		{
			pthread_create(&m_ff_thread, NULL, &LaunchMain, this);
		}
		else
		{
			pp::Module::Get()->core()->CallOnMainThread(
					100,
					m_cc_factory.NewCallback(&FFnaclInst::StartFFplayInNewThread),
					0);
		}
		printf("%s === END ===\n", __PRETTY_FUNCTION__);
	}

	static void* LaunchMain(void* data)
	{
        printf("%s ===START===%s\n", __PRETTY_FUNCTION__);
		FFnaclInst* self = reinterpret_cast<FFnaclInst*>(data);
		char* argv[] = { "-i", (char*)self->m_filename.c_str() };
		ffplay_main(sizeof(argv) / sizeof(argv[0]), argv);
        printf("%s === END ===\n", __PRETTY_FUNCTION__);
		return 0;
	}

};

int FFnaclInst::m_num_instances = 0;
}

class FFnaclMod : public pp::Module
{
public:
	FFnaclMod():pp::Module() {}
	virtual ~FFnaclMod() 	{}

	virtual pp::Instance* CreateInstance(PP_Instance instance)
	{
		return new ffnacl::FFnaclInst(instance);
	}
};

namespace pp
{
	Module* CreateModule()
	{
	  return new FFnaclMod();
	}
}
