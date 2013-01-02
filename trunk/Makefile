# Copyright (c) 2012 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#
# GNU Make based build file.  For details on GNU Make see:
# http://www.gnu.org/software/make/manual/make.html
#

#
# Project information
#
# These variables store project specific settings for the project name
# build flags, files to copy or install.  In the examples it is typically
# only the list of sources and project name that will actually change and
# the rest of the makefile is boilerplate for defining build rules.
#
PROJECT:= ffnacl
LDFLAGS:=-lppapi_cpp -lppapi -lpthread

#
# Get pepper directory for toolchain and includes.
#
# If NACL_SDK_ROOT is not set, then assume it can be found a two directories up,
# from the default example directory location.
#
THIS_MAKEFILE:=$(abspath $(lastword $(MAKEFILE_LIST)))
NACL_SDK_ROOT?=$(abspath $(dir $(THIS_MAKEFILE))../..)

# Project Build flags
WARNINGS:=-Wno-long-long -Wall -Wswitch-enum -pedantic
CCFLAGS:=-O0 -g -pthread -std=gnu++98 $(WARNINGS)
FF_FLAGS:=-DHAVE_AV_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 -std=c99 -fomit-frame-pointer -g -Wdeclaration-after-statement -Wall -Wno-switch -Wdisabled-optimization -Wpointer-arith -Wredundant-decls -Wno-pointer-sign -Wcast-qual -Wwrite-strings -Wtype-limits -Wundef -fno-math-errno -fno-signed-zeros

CFLAGS += -O0 -g $(FF_FLAGS)
#
# Compute tool paths
#
#
OSNAME:=$(shell python $(NACL_SDK_ROOT)/tools/getos.py)
TC_PATH:=$(abspath $(NACL_SDK_ROOT)/toolchain/$(OSNAME)_x86_newlib)
CXX:=$(TC_PATH)/bin/i686-nacl-g++
CC :=$(TC_PATH)/bin/i686-nacl-gcc

#
# Disable DOS PATH warning when using Cygwin based tools Windows
#
CYGWIN ?= nodosfilewarning
export CYGWIN

NACL_USR_INC = $(TC_PATH)/i686-nacl/usr/include
NACL_USR_LIB = $(TC_PATH)/i686-nacl/usr/lib
NACLPORTS_SRC_DIR ?= ../../../../naclports/src
NACLPORTS_FFMPEG_32 = $(NACLPORTS_SRC_DIR)/out/repository-i686/ffmpeg-0.5
NACLPORTS_FFMPEG_64 = $(NACLPORTS_SRC_DIR)/out/repository-x86_64/ffmpeg-0.5

FFMPEG_INCS_32 = \
          -I$(NACLPORTS_FFMPEG_32) \
          -I$(NACLPORTS_FFMPEG_32)/ffmpeg-0.5-build \
          -I$(NACLPORTS_FFMPEG_32)/libavcodec \
          -I$(NACLPORTS_FFMPEG_32)/libavdevice \
          -I$(NACLPORTS_FFMPEG_32)/libavformat \
          -I$(NACLPORTS_FFMPEG_32)/libavutil \
          -I$(NACLPORTS_FFMPEG_32)/libavfilter \
          -I$(NACLPORTS_FFMPEG_32)/libpostproc \
          -I$(NACLPORTS_FFMPEG_32)/libswscale
FFMPEG_INCS_64 = \
          -I$(NACLPORTS_FFMPEG_64) \
          -I$(NACLPORTS_FFMPEG_64)/ffmpeg-0.5-build \
          -I$(NACLPORTS_FFMPEG_64)/libavcodec \
          -I$(NACLPORTS_FFMPEG_64)/libavdevice \
          -I$(NACLPORTS_FFMPEG_64)/libavformat \
          -I$(NACLPORTS_FFMPEG_64)/libavutil \
          -I$(NACLPORTS_FFMPEG_64)/libavfilter \
          -I$(NACLPORTS_FFMPEG_64)/libpostproc \
          -I$(NACLPORTS_FFMPEG_64)/libswscale

FFMPEG_LIBS = -lavdevice -lavformat -lavcodec -lavutil -lswscale -lavfilter -lpostproc
EXTLIBS = -lz -lbz2 -lmp3lame -lm -ltheora -logg -lvorbisenc -lvorbis -lnosys
SDLINCS = -I$(NACL_USR_INC)/SDL
SDLLIBS = -lSDL -lSDLmain

INCS = $(SDLINCS)
LIBS = -L$(NACL_USR_LIB) $(SDLLIBS) $(FFMPEG_LIBS)
SRCS = $(PROJECT).cc $(NACLPORTS_FFMPEG_32)/cmdutils.c ffplay.c

x86_32_OBJS:=$(patsubst %.cc,%_32.o,$(patsubst %.c,%_32.o,$(SRCS)))
x86_64_OBJS:=$(patsubst %.cc,%_64.o,$(patsubst %.c,%_64.o,$(SRCS)))

all: $(PROJECT)_x86_32.nexe

$(PROJECT)_x86_32.nexe : $(x86_32_OBJS)
	$(CXX) $(CCFLAGS) $(INCS) -m32 -o $@ $^ $(LIBS) $(LDFLAGS) $(EXTLIBS)

$(PROJECT)_x86_64.nexe : $(x86_64_OBJS)
	$(CXX) $(CCFLAGS) $(INCS) -m64 -o $@ $^ $(LIBS) $(LDFLAGS) $(EXTLIBS)
	
%_32.o : %.cc
	$(CXX) $(CCFLAGS) $(INCS) $(FFMPEG_INCS_32) -m32 -o $@ -c $<
	
%_32.o : %.c
	$(CC) $(CFLAGS) $(INCS) $(FFMPEG_INCS_32) -m32 -o $@ -c $<

%_64.o : %.cc
	$(CXX) $(CCFLAGS) $(INCS) $(FFMPEG_INCS_64) -m64 -o $@ -c $<
	
%_64.o : %.c
	$(CC) $(CFLAGS) $(INCS) $(FFMPEG_INCS_64) -m64 -o $@ -c $<

clean:
	-rm -rf *.o $(x86_32_OBJS) $(x86_64_OBJS) *.nexe

# Define a phony rule so it always runs, to build nexe and start up server.
.PHONY: RUN 
RUN: all
	python ../httpd.py
