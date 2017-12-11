#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# Global settings file for the minimal build environment of the SDK
# This file have to updated/extended for other platforms.

include $(OO_SDK_HOME)/settings/dk.mk

# debug option, default is no debug
DEBUG=no
ifeq "$(MAKECMDGOALS)" "debug"
DEBUG=yes
endif

###########################################################################
#
# Java settings
#
###########################################################################
JAVAC_FLAGS=

ifeq "$(DEBUG)" "yes"
JAVAC_FLAGS+=-g
endif

###########################################################################
#
# Windows specific settings
#
###########################################################################
ifeq "$(PLATFORM)" "windows"
# Settings for Windows using Microsoft compiler/linker

OS=WIN
PS:=$(strip \ )
ICL=$$
CC=cl
LINK=link
BUILDLIB=lib
ECHO=@echo
MKDIR=mkdir
MV=move
CAT=type
OBJ_EXT=obj
EXE_EXT=.exe
COPY=copy
SHAREDLIB_EXT=dll
SHAREDLIB_OUT=$(OUT_BIN)
UNOPKG_PLATFORM=Windows

OSEP=^<
CSEP=^>
QUOTE=
QM=
SQM=
ECHOLINE=@echo.
P2BG=

DEL=del
DELRECURSIVE=rd /S /Q
URLPREFIX=file:///

SALLIB=isal.lib
CPPULIB=icppu.lib
CPPUHELPERLIB=icppuhelper.lib
SALHELPERLIB=isalhelper.lib
PURPENVHELPERLIB=ipurpenvhelper.lib

BLANK= 
EMPTYSTRING=
PATH_SEPARATOR=;

ifeq "$(LIBO_SDK_ENABLE_DBGUTIL)" "TRUE"
LIBO_SDK_DETAIL_CFLAGS_MSVCRT = -MDd
LIBO_SDK_DETAIL_LDFLAGS_MSVCRT = msvcrtd.lib
else
LIBO_SDK_DETAIL_CFLAGS_MSVCRT = -MD
LIBO_SDK_DETAIL_LDFLAGS_MSVCRT = msvcrt.lib
endif

# use this for release version
ifeq "$(DEBUG)" "yes"
OPT_FLAGS=-Zi
endif
CC_FLAGS_JNI=-c $(LIBO_SDK_DETAIL_CFLAGS_MSVCRT) -Zm500 -Zc:wchar_t- -wd4251 -wd4275 -wd4290 -wd4675 -wd4786 -wd4800 -GR -EHa $(OPT_FLAGS)
CC_FLAGS=-c $(LIBO_SDK_DETAIL_CFLAGS_MSVCRT) -Zm500 -Zc:wchar_t- -wd4251 -wd4275 -wd4290 -wd4675 -wd4786 -wd4800 -GR -EHa $(OPT_FLAGS)
ifeq "$(CPP_MANIFEST)" "true"
LINK_MANIFEST=mt -manifest $@.manifest "-outputresource:$@;2"
else
LINK_MANIFEST=
endif

CC_INCLUDES=-I. -I$(OUT)/inc -I$(OUT)/inc/examples -I$(PRJ)/include
SDK_JAVA_INCLUDES = -I"$(OO_SDK_JAVA_HOME)/include" -I"$(OO_SDK_JAVA_HOME)/include/win32"

# define for used compiler necessary for UNO
# -DCPPU_ENV=msci -- windows msvc 4.x - 7.x

ifeq "$(PROCTYPE)" "x86_64"
CPPU_ENV=mscx
else
CPPU_ENV=msci
endif

CC_DEFINES_JNI=-DWIN32 -DWNT -D_DLL -DCPPU_ENV=$(CPPU_ENV)
CC_DEFINES=-DWIN32 -DWNT -D_DLL -DCPPU_ENV=$(CPPU_ENV)
CC_OUTPUT_SWITCH=-Fo

LIBO_SDK_LDFLAGS_STDLIBS = $(LIBO_SDK_DETAIL_LDFLAGS_MSVCRT) kernel32.lib

LIBRARY_LINK_FLAGS=/DLL /DEBUGTYPE:cv
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS) /DEF:$(PRJ)/settings/component.uno.def
EXE_LINK_FLAGS=/MAP /OPT:NOREF /SUBSYSTEM:CONSOLE /BASE:0x1b000000 /DEBUGTYPE:cv $(LIBO_SDK_LDFLAGS_STDLIBS)
ifeq "$(DEBUG)" "yes"
LIBRARY_LINK_FLAGS+=/DEBUG
EXE_LINK_FLAGS+=/DEBUG
endif

LINK_JAVA_LIBS=/LIBPATH:"$(OO_SDK_JAVA_HOME)/lib"

URE_MISC=${OFFICE_PROGRAM_PATH}

# use this for release version
#EXE_LINK_FLAGS=/MAP /OPT:NOREF /SUBSYSTEM:CONSOLE /BASE:0x1100000
#LIBRARY_LINK_FLAGS=/DLL
endif	



###########################################################################
#
# Solaris specific settings
#
###########################################################################
ifeq "$(PLATFORM)" "solaris"
# Settings for Solaris using GCC

ifeq "$(PROCTYPE)" "sparc"
PLATFORM=solsparc
UNOPKG_PLATFORM=Solaris_SPARC
JAVA_PROC_TYPE=sparc
else
ifeq "$(PROCTYPE)" "sparc64"
PLATFORM=solsparc
UNOPKG_PLATFORM=Solaris_SPARC64
JAVA_PROC_TYPE=sparcv9
else
PLATFORM=solintel
UNOPKG_PLATFORM=Solaris_x86
JAVA_PROC_TYPE=i386
endif
endif

OS=SOLARIS
PS=/
ICL=\$$
CC=g++
LINK=g++
LIB=g++
ECHO=@echo
MKDIR=mkdir -p
CAT=cat
OBJ_EXT=o
SHAREDLIB_EXT=so
SHAREDLIB_PRE=lib
SHAREDLIB_OUT=$(OUT_LIB)

OSEP=\<
CSEP=\>
QUOTE=$(subst S,\,S)
QM=\"
SQM='
ECHOLINE=@echo
P2BG=&

DEL=rm -f
DELRECURSIVE=rm -rf
COPY=cp
URLPREFIX=file://

COMID=gcc3
CPPU_ENV=gcc3

SALLIB=-luno_sal
CPPULIB=-luno_cppu
CPPUHELPERLIB=-luno_cppuhelper$(COMID)
SALHELPERLIB=-luno_salhelper$(COMID)
PURPENVHELPERLIB=-luno_purpenvhelper$(COMID)

EMPTYSTRING=
PATH_SEPARATOR=:

# -O is necessary for inlining (see gcc documentation)
ifeq "$(DEBUG)" "yes"
OPT_FLAGS=-g
else
OPT_FLAGS=-O
endif
CC_FLAGS_JNI=-c -fpic $(OPT_FLAGS)
CC_FLAGS=-c -fpic -fvisibility=hidden $(OPT_FLAGS)
CC_INCLUDES=-I. -I$(OUT)/inc -I$(OUT)/inc/examples -I$(PRJ)/include
SDK_JAVA_INCLUDES = -I"$(OO_SDK_JAVA_HOME)/include" -I"$(OO_SDK_JAVA_HOME)/include/solaris"

# define for used compiler necessary for UNO

CC_DEFINES_JNI=-DUNX -DSOLARIS -DCPPU_ENV=$(CPPU_ENV) -DGCC
CC_DEFINES=-DUNX -DSOLARIS -DSPARC -DCPPU_ENV=$(CPPU_ENV) -DGCC
CC_OUTPUT_SWITCH=-o 

LIBO_SDK_LDFLAGS_STDLIBS =

LIBRARY_LINK_FLAGS=-w -mt -z combreloc -fPIC -PIC -temp=/tmp '-R$$ORIGIN' -z text -norunpath -G -Bdirect -Bdynamic -lpthread -lCrun -lc -lm
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS)

EXE_LINK_FLAGS=-w -mt -z combreloc -PIC -temp=/tmp -norunpath -Bdirect -z defs
LINK_LIBS=-L"$(OUT)/lib" -L"$(OO_SDK_HOME)/lib" -L"$(OO_SDK_URE_LIB_DIR)"
LINK_JAVA_LIBS=-L"$(OO_SDK_JAVA_HOME)/jre/lib/$(JAVA_PROC_TYPE)"

ifeq "$(PROCTYPE)" "sparc64"
CC_FLAGS+=-m64
LIBRARY_LINK_FLAGS+=-m64
EXE_LINK_FLAGS+=-m64
endif

URE_MISC=$(OFFICE_PROGRAM_PATH)

endif



###########################################################################
#
# Linux specific settings
#
###########################################################################
ifeq "$(PLATFORM)" "linux"
# Settings for Linux using gcc compiler

UNOPKG_PLATFORM=Linux_$(PROCTYPE)
JAVA_PROC_TYPE=$(PROCTYPE)

ifeq "$(PROCTYPE)" "x86"
JAVA_PROC_TYPE=i386
endif

ifeq "$(PROCTYPE)" "powerpc"
UNOPKG_PLATFORM=Linux_PowerPC
JAVA_PROC_TYPE=ppc
endif

ifeq "$(PROCTYPE)" "sparc"
UNOPKG_PLATFORM=Linux_SPARC
endif

ifeq "$(PROCTYPE)" "x86_64"
JAVA_PROC_TYPE=amd64
endif

ifeq "$(PROCTYPE)" "powerpc64"
JAVA_PROC_TYPE=ppc64
endif

ifeq "$(PROCTYPE)" "powerpc64_le"
JAVA_PROC_TYPE=ppc64le
endif

ifeq "$(PROCTYPE)" "arm_eabi"
JAVA_PROC_TYPE=arm
endif

ifeq "$(PROCTYPE)" "arm_oabi"
JAVA_PROC_TYPE=arm
endif

ifeq "$(PROCTYPE)" "mips_eb"
JAVA_PROC_TYPE=mips
endif

ifeq "$(PROCTYPE)" "mips_el"
JAVA_PROC_TYPE=mipsel
endif

ifeq "$(PROCTYPE)" "mips64_el"
JAVA_PROC_TYPE=mips64el
endif

ifeq "$(PROCTYPE)" "sparc64"
UNOPKG_PLATFORM=Linux_SPARC64
JAVA_PROC_TYPE=sparcv9
endif

OS=LINUX
PS=/
ICL=\$$
CC=gcc
LINK=g++
LIB=g++
ECHO=@echo
MKDIR=mkdir -p
CAT=cat
OBJ_EXT=o
SHAREDLIB_EXT=so
SHAREDLIB_PRE=lib
SHAREDLIB_OUT=$(OUT_LIB)

GCC_VERSION=$(shell $(CC) -dumpversion)

COMID=gcc3
CPPU_ENV=gcc3

OSEP=\<
CSEP=\>
QUOTE=$(subst S,\,S)
QM=\"
SQM='
ECHOLINE=@echo
P2BG=&

DEL=rm -f
DELRECURSIVE=rm -rf
COPY=cp
URLPREFIX=file://

SALLIB=-luno_sal
CPPULIB=-luno_cppu
CPPUHELPERLIB=-luno_cppuhelper$(COMID)
SALHELPERLIB=-luno_salhelper$(COMID)
PURPENVHELPERLIB=-luno_purpenvhelper$(COMID)

EMPTYSTRING=
PATH_SEPARATOR=:

# -O is necessary for inlining (see gcc documentation)
ifeq "$(DEBUG)" "yes"
OPT_FLAGS=-g
else
OPT_FLAGS=-O
endif
CC_FLAGS_JNI=-c -fpic $(OPT_FLAGS)
CC_FLAGS=-c -fpic -fvisibility=hidden $(OPT_FLAGS)

ifeq "$(PROCTYPE)" "powerpc"
CC_FLAGS+=-fPIC
endif

ifeq "$(PROCTYPE)" "x86"
CC_FLAGS+=-m32
endif

SDK_JAVA_INCLUDES = -I"$(OO_SDK_JAVA_HOME)/include" -I"$(OO_SDK_JAVA_HOME)/include/linux"
CC_INCLUDES=-I. -I$(OUT)/inc -I$(OUT)/inc/examples -I$(PRJ)/include
CC_DEFINES_JNI=-DUNX -DGCC -DLINUX -DCPPU_ENV=$(CPPU_ENV)
CC_DEFINES=-DUNX -DGCC -DLINUX -DCPPU_ENV=$(CPPU_ENV)

CC_OUTPUT_SWITCH=-o

LIBO_SDK_LDFLAGS_STDLIBS =

LIBRARY_LINK_FLAGS=-shared -Wl,-z,origin '-Wl,-rpath,$$ORIGIN'

ifeq "$(PROCTYPE)" "powerpc"
LIBRARY_LINK_FLAGS+=-fPIC
endif

ifeq "$(PROCTYPE)" "x86"
LIBRARY_LINK_FLAGS+=-m32
endif

COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS)

EXE_LINK_FLAGS=-Wl,--allow-shlib-undefined -Wl,-export-dynamic -Wl,-z,defs -Wl,--no-whole-archive

ifeq "$(PROCTYPE)" "x86"
EXE_LINK_FLAGS+=-m32
endif

LINK_LIBS=-L"$(OUT)/lib" -L"$(OO_SDK_HOME)/lib" -L"$(OO_SDK_URE_LIB_DIR)"
LINK_JAVA_LIBS=-L"$(OO_SDK_JAVA_HOME)/jre/lib/$(JAVA_PROC_TYPE)" -L"$(OO_SDK_JAVA_HOME)/lib"

URE_MISC=$(OFFICE_PROGRAM_PATH)

endif

###########################################################################
#
# MacOSX/Darwin specific settings
#
###########################################################################
ifeq "$(PLATFORM)" "macosx"

UNOPKG_PLATFORM=MacOSX_x86_64

OS=MACOSX
PS=/
ICL=\$$
CC=clang++
LINK=clang++
LIB=clang++
ECHO=@echo
MKDIR=mkdir -p
CAT=cat
OBJ_EXT=o
SHAREDLIB_EXT=dylib
SHAREDLIB_PRE=lib
SHAREDLIB_OUT=$(OUT_LIB)

COMID=gcc3
CPPU_ENV=gcc3

OSEP=\<
CSEP=\>
QUOTE=$(subst S,\,S)
QM=\"
SQM='
ECHOLINE=@echo
P2BG=&
    
DEL=rm -f
DELRECURSIVE=rm -rf
COPY=cp
URLPREFIX=file://

SALLIB=-luno_sal
CPPULIB=-luno_cppu
CPPUHELPERLIB=-luno_cppuhelper$(COMID)
SALHELPERLIB=-luno_salhelper$(COMID)
PURPENVHELPERLIB=-luno_purpenvhelper$(COMID)

INSTALL_NAME_URELIBS=$(XCRUN) install_name_tool -change @__________________________________________________URELIB/libuno_sal.dylib.3 @executable_path/urelibs/libuno_sal.dylib.3 -change  @__________________________________________________URELIB/libuno_cppu.dylib.3 @executable_path/urelibs/libuno_cppu.dylib.3 -change @__________________________________________________URELIB/libuno_cppuhelper$(COMID).dylib.3 @executable_path/urelibs/libuno_cppuhelper$(COMID).dylib.3 -change @__________________________________________________URELIB/libuno_salhelper$(COMID).dylib.3 @executable_path/urelibs/libuno_salhelper$(COMID).dylib.3

INSTALL_NAME_URELIBS_BIN=$(XCRUN) install_name_tool -change @__________________________________________________URELIB/libuno_sal.dylib.3 libuno_sal.dylib.3 -change  @__________________________________________________URELIB/libuno_cppu.dylib.3 libuno_cppu.dylib.3 -change @__________________________________________________URELIB/libuno_cppuhelper$(COMID).dylib.3 libuno_cppuhelper$(COMID).dylib.3 -change @__________________________________________________URELIB/libuno_salhelper$(COMID).dylib.3 libuno_salhelper$(COMID).dylib.3

EMPTYSTRING=
PATH_SEPARATOR=:

# -O is necessary for inlining (see gcc documentation)
ifeq "$(DEBUG)" "yes"
OPT_FLAGS=-g
else
OPT_FLAGS=-O
endif
CC_FLAGS_JNI=-c -fPIC -fno-common $(GCC_ARCH_OPTION) $(OPT_FLAGS)
CC_FLAGS=-c -fPIC -fno-common $(GCC_ARCH_OPTION) -fvisibility=hidden $(OPT_FLAGS)

SDK_JAVA_INCLUDES = -I/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers -I/System/Library/Frameworks/JavaVM.framework/Headers
CC_INCLUDES=-I. -I$(OUT)/inc -I$(OUT)/inc/examples -I$(PRJ)/include
CC_DEFINES_JNI=-DUNX -DGCC -DMACOSX -DCPPU_ENV=$(CPPU_ENV)
CC_DEFINES=-DUNX -DGCC -DMACOSX -DCPPU_ENV=$(CPPU_ENV)

CC_OUTPUT_SWITCH=-o

LIBO_SDK_LDFLAGS_STDLIBS =

LIBRARY_LINK_FLAGS=-dynamiclib -single_module -Wl,-multiply_defined,suppress $(GCC_ARCH_OPTION)

# install_name '@executable_path$/(@:f)'
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS)

EXE_LINK_FLAGS=$(GCC_ARCH_OPTION) -Wl,-multiply_defined,suppress
LINK_LIBS=-L$(OUT)/lib -L$(OO_SDK_OUT)/$(PLATFORM)/lib -L"$(OO_SDK_URE_LIB_DIR)"
LINK_JAVA_LIBS=-framework JavaVM

URE_MISC=$(OFFICE_HOME)/Contents/Resources/ure/share/misc

endif

###########################################################################
#
# FreeBSD specific settings
#
###########################################################################
ifneq (,$(findstring freebsd,$(PLATFORM)))
# Settings for FreeBSD using gcc compiler

ifeq (kfreebsd,$(findstring kfreebsd,$(PLATFORM)))
PLATFORM=kfreebsd
ifeq "$(PROCTYPE)" "x86_64"
UNOPKG_PLATFORM=kFreeBSD_x86_64
else
UNOPKG_PLATFORM=kFreeBSD_x86
endif
else
PLATFORM=freebsd
ifeq "$(PROCTYPE)" "x86_64"
UNOPKG_PLATFORM=FreeBSD_x86_64
else
UNOPKG_PLATFORM=FreeBSD_x86
endif
endif

ifeq "$(PROCTYPE)" "x86_64"
JAVA_PROC_TYPE=amd64
else
JAVA_PROC_TYPE=i386
endif

ifeq (kfreebsd,$(findstring kfreebsd,$(PLATFORM)))
OS=LINUX
else
OS=FREEBSD
endif
PS=/
ICL=\$$
CC=gcc
LINK=g++
LIB=g++
ECHO=@echo
MKDIR=mkdir -p
CAT=cat
OBJ_EXT=o
SHAREDLIB_EXT=so
SHAREDLIB_PRE=lib
SHAREDLIB_OUT=$(OUT_LIB)

GCC_VERSION=$(shell $(CC) -dumpversion)

COMID=gcc3
CPPU_ENV=gcc3

OSEP=\<
CSEP=\>
QUOTE=$(subst S,\,S)
QM=\"
SQM='
ECHOLINE=@echo
P2BG=&

DEL=rm -f
DELRECURSIVE=rm -rf
COPY=cp
URLPREFIX=file://

SALLIB=-luno_sal
CPPULIB=-luno_cppu
CPPUHELPERLIB=-luno_cppuhelper$(COMID)
SALHELPERLIB=-luno_salhelper$(COMID)
PURPENVHELPERLIB=-luno_purpenvhelper$(COMID)

EMPTYSTRING=
PATH_SEPARATOR=:

# -O is necessary for inlining (see gcc documentation)
ifeq "$(DEBUG)" "yes"
OPT_FLAGS=-g
else
OPT_FLAGS=-O
endif
CC_FLAGS_JNI=-c -g -fPIC -DPIC $(PTHREAD_CFLAGS) $(OPT_FLAGS)
CC_FLAGS=-c -g -fPIC -DPIC $(PTHREAD_CFLAGS) -fvisibility=hidden $(OPT_FLAGS)

SDK_JAVA_INCLUDES = -I"$(OO_SDK_JAVA_HOME)/include" -I"$(OO_SDK_JAVA_HOME)/include/freebsd"
CC_INCLUDES=-I. -I$(OUT)/inc -I$(OUT)/inc/examples -I$(PRJ)/include
CC_DEFINES_JNI=-DUNX -DGCC -DFREEBSD -DCPPU_ENV=$(CPPU_ENV)
CC_DEFINES=-DUNX -DGCC -DFREEBSD -DCPPU_ENV=$(CPPU_ENV)

CC_OUTPUT_SWITCH=-o

LIBO_SDK_LDFLAGS_STDLIBS =

LIBRARY_LINK_FLAGS=-shared -Wl,-z,origin '-Wl,-rpath,$$ORIGIN'
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS)

EXE_LINK_FLAGS=-Wl,--allow-shlib-undefined 
LINK_LIBS=-L"$(OUT)/lib" -L"$(OO_SDK_HOME)/lib" -L"$(OO_SDK_URE_LIB_DIR)" $(PTHREAD_LIBS)
LINK_JAVA_LIBS=-L"$(OO_SDK_JAVA_HOME)/jre/lib/$(JAVA_PROC_TYPE)"

URE_MISC=$(OFFICE_PROGRAM_PATH)

endif
