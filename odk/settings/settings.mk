#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************

# Global settings file for the minimal build environment of the SDK
# This file have to updated/extended for other platforms.

# test for the platform
PLATFORM := $(shell $(PRJ)/config.guess | cut -d"-" -f3,4)

# config.guess is missing for windows. We rely on getting "" in this case.
ifeq "$(PLATFORM)" ""
PLATFORM = windows
endif

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

JAVABIN=bin

###########################################################################
#
# Windows specific settings
#
###########################################################################
ifeq "$(PLATFORM)" "windows"
# Settings for Windows using Microsoft compiler/linker

PROCTYPE := $(shell $(PRJ)/config.guess | cut -d"-" -f1 | sed -e 's/^i.86$$/i386/')

OS=WIN
PS=\\
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
REGLIB=ireg.lib
STORELIB=istore.lib

BLANK= 
EMPTYSTRING=
PATH_SEPARATOR=;

# use this for release version
CC_FLAGS_JNI=-c -MT -Zm500 -Zc:wchar_t- -wd4251 -wd4275 -wd4290 -wd4675 -wd4786 -wd4800 -GR -EHa
CC_FLAGS=-c -MT -Zm500 -Zc:wchar_t- -wd4251 -wd4275 -wd4290 -wd4675 -wd4786 -wd4800 -GR -EHa
ifeq "$(CPP_MANIFEST)" "true"
#CC_FLAGS+=-EHa -Zc:wchar_t-
LINK_MANIFEST=mt -manifest $@.manifest "-outputresource:$@;2"
else
#CC_FLAGS+=
LINK_MANIFEST=
endif
ifeq "$(DEBUG)" "yes"
CC_FLAGS_JNI+=-Zi
CC_FLAGS+=-Zi
endif

CC_INCLUDES=-I. -I$(OUT)/inc -I$(OUT)/inc/examples -I$(PRJ)/include
STL_INCLUDES=-I"$(OO_SDK_HOME)/include/stl"
SDK_JAVA_INCLUDES = -I"$(OO_SDK_JAVA_HOME)/include" -I"$(OO_SDK_JAVA_HOME)/include/win32"

# define for used compiler necessary for UNO
# -DCPPU_ENV=msci -- windows msvc 4.x - 7.x

ifeq "$(PROCTYPE)" "i386"
CC_DEFINES_JNI=-DWIN32 -DWNT -D_DLL -DCPPU_ENV=msci
CC_DEFINES=-DWIN32 -DWNT -D_DLL -DCPPU_ENV=msci
endif
ifeq "$(PROCTYPE)" "x86_64"
CC_DEFINES_JNI=-DWIN32 -DWNT -D_DLL -DCPPU_ENV=mscx
CC_DEFINES=-DWIN32 -DWNT -D_DLL -DCPPU_ENV=mscx
endif
CC_OUTPUT_SWITCH=-Fo

LIBRARY_LINK_FLAGS=/NODEFAULTLIB /DLL /DEBUGTYPE:cv
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS) /DEF:$(PRJ)/settings/component.uno.def
EXE_LINK_FLAGS=/MAP /OPT:NOREF /SUBSYSTEM:CONSOLE /BASE:0x1b000000 /DEBUGTYPE:cv /NODEFAULTLIB  msvcrt.lib msvcprt.lib kernel32.lib
ifeq "$(DEBUG)" "yes"
LIBRARY_LINK_FLAGS+=/DEBUG
EXE_LINK_FLAGS+=/DEBUG
endif

LINK_JAVA_LIBS=/LIBPATH:"$(OO_SDK_JAVA_HOME)/lib"

# use this for release version
#EXE_LINK_FLAGS=/MAP /OPT:NOREF /SUBSYSTEM:CONSOLE /BASE:0x1100000
#LIBRARY_LINK_FLAGS=/NODEFAULTLIB /DLL
endif	



###########################################################################
#
# Solaris specific settings
#
###########################################################################
ifneq (,$(findstring solaris,$(PLATFORM)))
# Settings for Solaris using Sun Workshop compiler

PROCTYPE := $(shell $(PRJ)/config.guess | cut -d"-" -f1)$(shell /usr/ccs/bin/elfdump -e "$(OFFICE_PROGRAM_PATH)/libuno_sal.so.3" | /usr/xpg4/bin/grep -q -w ELFCLASS64 && echo 64)

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
CC=CC
LINK=CC
LIB=CC
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

# Include UDK version numbers
include $(PRJ)/include/udkversion.mk

SALLIB=-luno_sal
CPPULIB=-luno_cppu
CPPUHELPERLIB=-luno_cppuhelperC52
SALHELPERLIB=-luno_salhelperC52
REGLIB=-lreg
STORELIB=-lstore

EMPTYSTRING=
PATH_SEPARATOR=:

CC_FLAGS_JNI=-c -KPIC
CC_FLAGS=-c -KPIC -xldscope=hidden
ifeq "$(DEBUG)" "yes"
CC_FLAGS_JNI+=-g
CC_FLAGS+=-g
endif
CC_INCLUDES=-I. -I$(OUT)/inc -I$(OUT)/inc/examples -I$(PRJ)/include
STL_INCLUDES=-I"$(OO_SDK_HOME)/include/stl"
SDK_JAVA_INCLUDES = -I"$(OO_SDK_JAVA_HOME)/include" -I"$(OO_SDK_JAVA_HOME)/include/solaris"

# define for used compiler necessary for UNO
# -DCPPU_ENV=sunpro5 -- sunpro cc 5.x solaris sparc/intel

CC_DEFINES_JNI=-DUNX -DSOLARIS -DCPPU_ENV=sunpro5
CC_DEFINES=-DUNX -DSOLARIS -DSPARC -DCPPU_ENV=sunpro5  -DHAVE_GCC_VISIBILITY_FEATURE
CC_OUTPUT_SWITCH=-o 

LIBRARY_LINK_FLAGS=-w -mt -z combreloc -PIC -temp=/tmp '-R$$ORIGIN' -z text -norunpath -G -Bdirect -Bdynamic -lpthread -lCrun -lc -lm
# means if used CC is lower then version 5.5 use option -instance=static
ifeq ($(OO_SDK_CC_55_OR_HIGHER),)
LIBRARY_LINK_FLAGS+=-instances=static
endif
#COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS) -M $(PRJ)/settings/component.uno.map
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS)

EXE_LINK_FLAGS=-w -mt -z combreloc -PIC -temp=/tmp -norunpath -Bdirect -z defs
LINK_LIBS=-L"$(OUT)/lib" -L"$(OO_SDK_HOME)/lib" -L"$(OO_SDK_OFFICE_LIB_DIR)"
LINK_JAVA_LIBS=-L"$(OO_SDK_JAVA_HOME)/jre/lib/$(JAVA_PROC_TYPE)"

ifeq "$(PROCTYPE)" "sparc64"
CC_FLAGS+=-m64
LIBRARY_LINK_FLAGS+=-m64
EXE_LINK_FLAGS+=-m64
endif

endif



###########################################################################
#
# Linux specific settings
#
###########################################################################
ifneq (,$(findstring linux,$(PLATFORM)))
# Settings for Linux using gcc compiler

PROCTYPE := $(shell $(PRJ)/config.guess | cut -d "-" -f1 | sed -e 's/^i.86$$/i386/')
PLATFORM=linux

UNOPKG_PLATFORM=Linux_$(PROCTYPE)
JAVA_PROC_TYPE=$(PROCTYPE)

ifeq "$(PROCTYPE)" "i386"
UNOPKG_PLATFORM=Linux_x86
JAVA_PROC_TYPE=i386
endif

ifeq "$(PROCTYPE)" "powerpc"
UNOPKG_PLATFORM=Linux_PowerPC
JAVA_PROC_TYPE=ppc
endif

ifeq "$(PROCTYPE)" "sparc"
UNOPKG_PLATFORM=Linux_SPARC
JAVA_PROC_TYPE=sparc
endif

ifeq "$(PROCTYPE)" "x86_64"
JAVA_PROC_TYPE=amd64
endif

ifeq "$(PROCTYPE)" "powerpc64"
JAVA_PROC_TYPE=ppc64
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

# Include UDK version numbers
include $(PRJ)/include/udkversion.mk

SALLIB=-luno_sal
CPPULIB=-luno_cppu
CPPUHELPERLIB=-luno_cppuhelper$(COMID)
SALHELPERLIB=-luno_salhelper$(COMID)
REGLIB=-lreg
STORELIB=-lstore

EMPTYSTRING=
PATH_SEPARATOR=:

CC_FLAGS_JNI=-c -fpic
CC_FLAGS=-c -fpic -fvisibility=hidden
# -O is necessary for inlining (see gcc documentation)
ifeq "$(DEBUG)" "yes"
CC_FLAGS_JNI+=-g
CC_FLAGS+=-g
else
CC_FLAGS_JNI+=-O
CC_FLAGS+=-O
endif

ifeq "$(PROCTYPE)" "ppc"
CC_FLAGS+=-fPIC
endif

SDK_JAVA_INCLUDES = -I"$(OO_SDK_JAVA_HOME)/include" -I"$(OO_SDK_JAVA_HOME)/include/linux"
CC_INCLUDES=-I. -I$(OUT)/inc -I$(OUT)/inc/examples -I$(PRJ)/include
STL_INCLUDES=-I"$(OO_SDK_HOME)/include/stl"
CC_DEFINES_JNI=-DUNX -DGCC -DLINUX -DCPPU_ENV=$(CPPU_ENV)
CC_DEFINES=-DUNX -DGCC -DLINUX -DCPPU_ENV=$(CPPU_ENV) -DHAVE_GCC_VISIBILITY_FEATURE

# define for used compiler necessary for UNO
#-DCPPU_ENV=gcc2 -- gcc 2.91/2.95
#-DCPPU_ENV=gcc3 -- gcc3 3.0

CC_OUTPUT_SWITCH=-o

LIBRARY_LINK_FLAGS=-shared '-Wl,-rpath,$$ORIGIN'

ifeq "$(PROCTYPE)" "ppc"
LIBRARY_LINK_FLAGS+=-fPIC
endif
#COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS) -Wl,--version-script,$(PRJ)/settings/component.uno.map
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS)

#EXE_LINK_FLAGS=-Wl,--allow-shlib-undefined -Wl,-export-dynamic -Wl,-z,defs -Wl,--whole-archive -lsalcpprt -Wl,--no-whole-archive
EXE_LINK_FLAGS=-Wl,--allow-shlib-undefined -Wl,-export-dynamic -Wl,-z,defs -Wl,--no-whole-archive
LINK_LIBS=-L"$(OUT)/lib" -L"$(OO_SDK_HOME)/lib" -L"$(OO_SDK_OFFICE_LIB_DIR)"
LINK_JAVA_LIBS=-L"$(OO_SDK_JAVA_HOME)/jre/lib/$(JAVA_PROC_TYPE)"

endif

###########################################################################
#
# MacOSX/Darwin specific settings
#
###########################################################################
ifneq (,$(findstring darwin,$(PLATFORM)))
# Settings for MacOSX using LLVM version 4.2 (clang-425.0.28)

PROCTYPE := $(shell $(PRJ)/config.guess | cut -d"-" -f1)

# Default is MacOSX on a Intel machine    
PLATFORM=macosx

ifeq "$(PROCTYPE)" "x86_64"
UNOPKG_PLATFORM=MacOSX_x86_64
endif

JAVABIN=Commands

GCC_ARCH_OPTION=-arch x86_64

OS=MACOSX
PS=/
ICL=\$$
CC=`xcrun -f clang` -arch x86_64
CXX=`xcrun -f clang++` -arch x86_64 -std=c++11
LINK=`xcrun -f clang++` -arch x86_64 -std=c++11
LIB=`xcrun -f clang++` -arch x86_64 -std=c++11
INSTALLTOOL=`xcrun -f install_name_tool`

ECHO=@echo
MKDIR=mkdir -p
CAT=cat
OBJ_EXT=o
SHAREDLIB_EXT=dylib
SHAREDLIB_PRE=lib
SHAREDLIB_OUT=$(OUT_LIB)

COMID=gcc3
CPPU_ENV=gcc3
COMID=s5abi
CPPU_ENV=s5abi

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

# Include UDK version numbers
include $(PRJ)/include/udkversion.mk

SALLIB=-luno_sal
CPPULIB=-luno_cppu
CPPUHELPERLIB=-luno_cppuhelper$(COMID)
SALHELPERLIB=-luno_salhelper$(COMID)
REGLIB=-lreg
STORELIB=-lstore

SALDYLIB=-Wl,-dylib_file,@_______URELIB/libuno_sal.dylib:'$(OO_SDK_OFFICE_LIB_DIR)/libuno_sal.dylib'
CPPUDYLIB=-Wl,-dylib_file,@_______URELIB/libuno_cppu.dylib:'$(OO_SDK_OFFICE_LIB_DIR)/libuno_cppu.dylib'
CPPUHELPERDYLIB=-Wl,-dylib_file,@_______URELIB/libuno_cppuhelper'$(COMID).dylib:$(OO_SDK_OFFICE_LIB_DIR)/libuno_cppuhelper$(COMID).dylib'
SALHELPERDYLIB=-Wl,-dylib_file,@_______URELIB/libuno_salhelper$(COMID).dylib:'$(OO_SDK_OFFICE_LIB_DIR)/libuno_salhelper$(COMID).dylib'
REGDYLIB=-Wl,-dylib_file,@_______URELIB/libreg.dylib:'$(OO_SDK_OFFICE_LIB_DIR)/libreg.dylib'
STOREDYLIB=-Wl,-dylib_file,@_______URELIB/libstore.dylib:'$(OO_SDK_OFFICE_LIB_DIR)/libstore.dylib'

INSTALL_NAME_URELIBS=$(INSTALLTOOL) -change @_______URELIB/libuno_sal.dylib @executable_path/libuno_sal.dylib -change  @_______URELIB/libuno_cppu.dylib @executable_path/libuno_cppu.dylib -change @_______URELIB/libuno_cppuhelper$(COMID).dylib @executable_path/libuno_cppuhelper$(COMID).dylib -change @_______URELIB/libuno_salhelper$(COMID).dylib @executable_path/libuno_salhelper$(COMID).dylib -change @_______URELIB/libreg.dylib @executable_path/libreg.dylib -change @_______URELIB/libstore.dylib @executable_path/libstore.dylib

INSTALL_NAME_URELIBS_BIN=$(INSTALLTOOL) -change @_______URELIB/libuno_sal.dylib libuno_sal.dylib -change  @_______URELIB/libuno_cppu.dylib libuno_cppu.dylib -change @_______URELIB/libuno_cppuhelper$(COMID).dylib libuno_cppuhelper$(COMID).dylib -change @_______URELIB/libuno_salhelper$(COMID).dylib libuno_salhelper$(COMID).dylib -change @_______URELIB/libreg.dylib libreg.dylib -change @_______URELIB/libstore.dylib libstore.dylib

EMPTYSTRING=
PATH_SEPARATOR=:

CC_FLAGS_JNI=-c -fPIC -fno-common $(GCC_ARCH_OPTION)
CC_FLAGS=-c -fPIC -fno-common $(GCC_ARCH_OPTION) -fvisibility=hidden

# -O is necessary for inlining (see gcc documentation)
ifeq "$(DEBUG)" "yes"
CC_FLAGS_JNI+=-g
CC_FLAGS+=-g
else
CC_FLAGS_JNI+=-O
CC_FLAGS+=-O
endif

SDK_JAVA_INCLUDES = -I/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers -I/System/Library/Frameworks/JavaVM.framework/Headers
CC_INCLUDES=-I. -I$(OUT)/inc -I$(OUT)/inc/examples -I$(PRJ)/include
STL_INCLUDES=-I"$(OO_SDK_HOME)/include/stl"
CC_DEFINES_JNI=-DUNX -DGCC -DMACOSX -DCPPU_ENV=$(CPPU_ENV)
CC_DEFINES=-DUNX -DGCC -DMACOSX -DCPPU_ENV=$(CPPU_ENV) -DHAVE_GCC_VISIBILITY_FEATURE

CC_OUTPUT_SWITCH=-o

LIBRARY_LINK_FLAGS=-dynamiclib -single_module -Wl,-multiply_defined,suppress $(GCC_ARCH_OPTION)
#-fPIC -fno-common

# install_name '@executable_path$/(@:f)'
#COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS)  -Wl,-exported_symbols_list $(COMP_MAPFILE)
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS)

EXE_LINK_FLAGS=$(GCC_ARCH_OPTION) -Wl,-multiply_defined,suppress
LINK_LIBS=-L$(OUT)/lib -L$(OO_SDK_OUT)/$(PLATFORM)/lib -L"$(OO_SDK_OFFICE_LIB_DIR)"
LINK_JAVA_LIBS=-framework JavaVM
#LINK_JAVA_LIBS=-L"$(OO_SDK_JAVA_HOME)/Libraries"

endif

###########################################################################
#
# FreeBSD specific settings
#
###########################################################################
ifneq (,$(findstring freebsd,$(PLATFORM)))
# Settings for FreeBSD using gcc compiler

PROCTYPE := $(shell $(PRJ)/config.guess | cut -d"-" -f1)

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
ifeq "$(PROCTYPE)" "powerpc"
UNOPKG_PLATFORM=FreeBSD_ppc
endif
endif

ifeq "$(PROCTYPE)" "x86_64"
JAVA_PROC_TYPE=amd64
else
JAVA_PROC_TYPE=i386
endif

ifeq "$(PROCTYPE)" "powerpc64"
JAVA_PROC_TYPE=ppc64
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

# Include UDK version numbers
include $(PRJ)/include/udkversion.mk

SALLIB=-luno_sal
CPPULIB=-luno_cppu
CPPUHELPERLIB=-luno_cppuhelper$(COMID)
SALHELPERLIB=-luno_salhelper$(COMID)
REGLIB=-lreg
STORELIB=-lstore

EMPTYSTRING=
PATH_SEPARATOR=:

CC_FLAGS_JNI=-c -g -fPIC -DPIC $(PTHREAD_CFLAGS)
CC_FLAGS=-c -g -fPIC -DPIC $(PTHREAD_CFLAGS) -fvisibility=hidden
# -O is necessary for inlining (see gcc documentation)
ifeq "$(DEBUG)" "yes"
CC_FLAGS_JNI+=-g
CC_FLAGS+=-g
else
CC_FLAGS_JNI+=-O
CC_FLAGS+=-O
endif

SDK_JAVA_INCLUDES = -I"$(OO_SDK_JAVA_HOME)/include" -I"$(OO_SDK_JAVA_HOME)/include/freebsd"
CC_INCLUDES=-I. -I$(OUT)/inc -I$(OUT)/inc/examples -I$(PRJ)/include
STL_INCLUDES=-I"$(OO_SDK_HOME)/include/stl"
CC_DEFINES_JNI=-DUNX -DGCC -DFREEBSD -DCPPU_ENV=$(CPPU_ENV)
CC_DEFINES=-DUNX -DGCC -DFREEBSD -DCPPU_ENV=$(CPPU_ENV) -DHAVE_GCC_VISIBILITY_FEATURE

CC_OUTPUT_SWITCH=-o

LIBRARY_LINK_FLAGS=-shared '-Wl,-rpath,$$ORIGIN'
#COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS) -Wl,--version-script,$(PRJ)/settings/component.uno.map
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS)

EXE_LINK_FLAGS=-Wl,--allow-shlib-undefined 
#EXE_LINK_FLAGS+=-Wl,-export-dynamic -Wl,-z,defs
LINK_LIBS=-L"$(OUT)/lib" -L"$(OO_SDK_HOME)/lib" -L"$(OO_SDK_OFFICE_LIB_DIR)" $(PTHREAD_LIBS)
LINK_JAVA_LIBS=-L"$(OO_SDK_JAVA_HOME)/jre/lib/$(JAVA_PROC_TYPE)"

endif

# add additional boost specific settings
ifneq "$(OO_SDK_BOOST_HOME)" ""
STL_INCLUDES+= -I"$(OO_SDK_BOOST_HOME)"
endif

# Add OSL_DEBUG_LEVEL to compiler the flags (for OSL_TRACE et. al.)
ifeq "$(DEBUG)" "yes"
CC_FLAGS += -DOSL_DEBUG_LEVEL=2
endif
