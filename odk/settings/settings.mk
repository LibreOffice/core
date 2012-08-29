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
CC_FLAGS_JNI=-c -MT -Zm500 -Zc:forScope,wchar_t- -wd4251 -wd4275 -wd4290 -wd4675 -wd4786 -wd4800 -Zc:forScope -GR -EHa
CC_FLAGS=-c -MT -Zm500 -Zc:forScope,wchar_t- -wd4251 -wd4275 -wd4290 -wd4675 -wd4786 -wd4800 -Zc:forScope -GR -EHa
ifeq "$(CPP_MANIFEST)" "true"
LINK_MANIFEST=mt -manifest $@.manifest "-outputresource:$@;2"
else
LINK_MANIFEST=
endif
ifeq "$(DEBUG)" "yes"
CC_FLAGS_JNI+=-Zi
CC_FLAGS+=-Zi
endif

CC_INCLUDES=-I. -I$(OUT)/inc -I$(OUT)/inc/examples -I$(PRJ)/include
SDK_JAVA_INCLUDES = -I"$(OO_SDK_JAVA_HOME)/include" -I"$(OO_SDK_JAVA_HOME)/include/win32"

# define for used compiler necessary for UNO
# -DCPPU_ENV=msci -- windows msvc 4.x - 7.x

CC_DEFINES_JNI=-DWIN32 -DWNT -D_DLL -DCPPU_ENV=msci
CC_DEFINES=-DWIN32 -DWNT -D_DLL -DCPPU_ENV=msci
CC_OUTPUT_SWITCH=-Fo

LIBRARY_LINK_FLAGS=/MD /DLL /DEBUGTYPE:cv
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS) /DEF:$(PRJ)/settings/component.uno.def
EXE_LINK_FLAGS=/MAP /OPT:NOREF /SUBSYSTEM:CONSOLE /BASE:0x1b000000 /DEBUGTYPE:cv /MD  msvcrt.lib kernel32.lib
ifeq "$(DEBUG)" "yes"
LIBRARY_LINK_FLAGS+=/DEBUG
EXE_LINK_FLAGS+=/DEBUG
endif

LINK_JAVA_LIBS=/LIBPATH:"$(OO_SDK_JAVA_HOME)/lib"

ifneq "$(OO_SDK_URE_HOME)" ""
URE_MISC=$(OO_SDK_URE_HOME)\misc
endif

# use this for release version
#EXE_LINK_FLAGS=/MAP /OPT:NOREF /SUBSYSTEM:CONSOLE /BASE:0x1100000
#LIBRARY_LINK_FLAGS=/MD /DLL
endif	



###########################################################################
#
# Solaris specific settings
#
###########################################################################
ifneq (,$(findstring solaris,$(PLATFORM)))
# Settings for Solaris using Sun Workshop compiler

PROCTYPE := $(shell $(PRJ)/config.guess | cut -d"-" -f1)$(shell /usr/ccs/bin/elfdump -e "$(OO_SDK_URE_HOME)/lib/libuno_sal.so.3" | /usr/xpg4/bin/grep -q -w ELFCLASS64 && echo 64)

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

COMID=gcc3
CPPU_ENV=gcc3

# Include UDK version numbers
include $(PRJ)/include/udkversion.mk

#SALLIB=-luno_sal
#CPPULIB=-luno_cppu
#CPPUHELPERLIB=-luno_cppuhelperC52
#SALHELPERLIB=-luno_salhelperC52
#REGLIB=-lreg
#STORELIB=-lstore
SALLIB=-luno_sal
CPPULIB=-luno_cppu
CPPUHELPERLIB=-luno_cppuhelper$(COMID)
SALHELPERLIB=-luno_salhelper$(COMID)
REGLIB=-lreg
STORELIB=-lstore

EMPTYSTRING=
PATH_SEPARATOR=:

#CC_FLAGS_JNI=-c -KPIC
#CC_FLAGS=-c -KPIC -xldscope=hidden
CC_FLAGS_JNI=-c -fpic
CC_FLAGS=-c -fpic -fvisibility=hidden
ifeq "$(DEBUG)" "yes"
CC_FLAGS_JNI+=-g
CC_FLAGS+=-g
endif
CC_INCLUDES=-I. -I$(OUT)/inc -I$(OUT)/inc/examples -I$(PRJ)/include
SDK_JAVA_INCLUDES = -I"$(OO_SDK_JAVA_HOME)/include" -I"$(OO_SDK_JAVA_HOME)/include/solaris"

# define for used compiler necessary for UNO
# -DCPPU_ENV=sunpro5 -- sunpro cc 5.x solaris sparc/intel

#CC_DEFINES_JNI=-DUNX -DSOLARIS -DCPPU_ENV=sunpro5
CC_DEFINES_JNI=-DUNX -DSOLARIS -DCPPU_ENV=$(CPPU_ENV) -DGCC -DGXX_INCLUDE_PATH=$(SDK_GXX_INCLUDE_PATH)
#CC_DEFINES=-DUNX -DSOLARIS -DSPARC -DCPPU_ENV=sunpro5  -DHAVE_GCC_VISIBILITY_FEATURE
CC_DEFINES=-DUNX -DSOLARIS -DSPARC -DCPPU_ENV=$(CPPU_ENV)  -DHAVE_GCC_VISIBILITY_FEATURE -DGCC -DGXX_INCLUDE_PATH=$(SDK_GXX_INCLUDE_PATH)
CC_OUTPUT_SWITCH=-o 

#LIBRARY_LINK_FLAGS=-w -mt -z combreloc -PIC -temp=/tmp '-R$$ORIGIN' -z text -norunpath -G -Bdirect -Bdynamic -lpthread -lCrun -lc -lm
LIBRARY_LINK_FLAGS=-w -mt -z combreloc -fPIC -PIC -temp=/tmp '-R$$ORIGIN' -z text -norunpath -G -Bdirect -Bdynamic -lpthread -lCrun -lc -lm
# means if used CC is lower then version 5.5 use option -instance=static
ifeq ($(OO_SDK_CC_55_OR_HIGHER),)
LIBRARY_LINK_FLAGS+=-instances=static
endif
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS)

EXE_LINK_FLAGS=-w -mt -z combreloc -PIC -temp=/tmp -norunpath -Bdirect -z defs
LINK_LIBS=-L"$(OUT)/lib" -L"$(OO_SDK_HOME)/lib" -L"$(OO_SDK_URE_LIB_DIR)"
LINK_JAVA_LIBS=-L"$(OO_SDK_JAVA_HOME)/jre/lib/$(JAVA_PROC_TYPE)"

ifeq "$(PROCTYPE)" "sparc64"
CC_FLAGS+=-m64
LIBRARY_LINK_FLAGS+=-m64
EXE_LINK_FLAGS+=-m64
endif

ifneq "$(OO_SDK_URE_HOME)" ""
URE_MISC=$(OO_SDK_URE_HOME)/share/misc
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
CC_DEFINES_JNI=-DUNX -DGCC -DLINUX -DCPPU_ENV=$(CPPU_ENV) -DGXX_INCLUDE_PATH=$(SDK_GXX_INCLUDE_PATH)
CC_DEFINES=-DUNX -DGCC -DLINUX -DCPPU_ENV=$(CPPU_ENV) -DGXX_INCLUDE_PATH=$(SDK_GXX_INCLUDE_PATH) -DHAVE_GCC_VISIBILITY_FEATURE

CC_OUTPUT_SWITCH=-o

LIBRARY_LINK_FLAGS=-shared '-Wl,-z,origin -Wl,-rpath,$$ORIGIN'

ifeq "$(PROCTYPE)" "ppc"
LIBRARY_LINK_FLAGS+=-fPIC
endif
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS)

EXE_LINK_FLAGS=-Wl,--allow-shlib-undefined -Wl,-export-dynamic -Wl,-z,defs -Wl,--no-whole-archive
LINK_LIBS=-L"$(OUT)/lib" -L"$(OO_SDK_HOME)/lib" -L"$(OO_SDK_URE_LIB_DIR)"
LINK_JAVA_LIBS=-L"$(OO_SDK_JAVA_HOME)/jre/lib/$(JAVA_PROC_TYPE)"

ifneq "$(OO_SDK_URE_HOME)" ""
URE_MISC=$(OO_SDK_URE_HOME)/share/misc
endif

endif

###########################################################################
#
# MacOSX/Darwin specific settings
#
###########################################################################
ifneq (,$(findstring darwin,$(PLATFORM)))
# Settings for MacOSX using gcc 4.0.1 compiler

PROCTYPE := $(shell $(PRJ)/config.guess | cut -d"-" -f1)

# Default is MacOSX on a Intel machine    
PLATFORM=macosx

ifeq "$(PROCTYPE)" "i386"
UNOPKG_PLATFORM=MacOSX_x86
JAVA_PROC_TYPE=x86
else
ifeq "$(PROCTYPE)" "x86_64"
UNOPKG_PLATFORM=MacOSX_x86
JAVA_PROC_TYPE=x86
else
UNOPKG_PLATFORM=MacOSX_PowerPC
JAVA_PROC_TYPE=ppc
endif
endif
JAVABIN=Commands

GCC_VERSION =$(shell gcc -dumpversion| cut -d"." -f1,2)
ifeq "$(GCC_VERSION)" "4.2"
GCC_ARCH_OPTION=-arch i386
endif

OS=MACOSX
PS=/
ICL=\$$
CC=gcc-$(GCC_VERSION)
LINK=g++-$(GCC_VERSION)
LIB=g++-$(GCC_VERSION)
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

# Include UDK version numbers
include $(PRJ)/include/udkversion.mk

SALLIB=-luno_sal
CPPULIB=-luno_cppu
CPPUHELPERLIB=-luno_cppuhelper$(COMID)
SALHELPERLIB=-luno_salhelper$(COMID)
REGLIB=-lreg
STORELIB=-lstore

SALDYLIB=-Wl,-dylib_file,@__________________________________________________URELIB/libuno_sal.dylib.3:'$(OO_SDK_URE_LIB_DIR)/libuno_sal.dylib'
CPPUDYLIB=-Wl,-dylib_file,@__________________________________________________URELIB/libuno_cppu.dylib.3:'$(OO_SDK_URE_LIB_DIR)/libuno_cppu.dylib'
CPPUHELPERDYLIB=-Wl,-dylib_file,@__________________________________________________URELIB/libuno_cppuhelper'$(COMID).dylib.3:$(OO_SDK_URE_LIB_DIR)/libuno_cppuhelper$(COMID).dylib'
SALHELPERDYLIB=-Wl,-dylib_file,@__________________________________________________URELIB/libuno_salhelper$(COMID).dylib.3:'$(OO_SDK_URE_LIB_DIR)/libuno_salhelper$(COMID).dylib'
REGDYLIB=-Wl,-dylib_file,@__________________________________________________URELIB/libreg.dylib.3:'$(OO_SDK_URE_LIB_DIR)/libreg.dylib'
STOREDYLIB=-Wl,-dylib_file,@__________________________________________________URELIB/libstore.dylib.3:'$(OO_SDK_URE_LIB_DIR)/libstore.dylib'

INSTALL_NAME_URELIBS=install_name_tool -change @__________________________________________________URELIB/libuno_sal.dylib.3 @executable_path/urelibs/libuno_sal.dylib.3 -change  @__________________________________________________URELIB/libuno_cppu.dylib.3 @executable_path/urelibs/libuno_cppu.dylib.3 -change @__________________________________________________URELIB/libuno_cppuhelper$(COMID).dylib.3 @executable_path/urelibs/libuno_cppuhelper$(COMID).dylib.3 -change @__________________________________________________URELIB/libuno_salhelper$(COMID).dylib.3 @executable_path/urelibs/libuno_salhelper$(COMID).dylib.3 -change @__________________________________________________URELIB/libreg.dylib.3 @executable_path/urelibs/libreg.dylib.3 -change @__________________________________________________URELIB/libstore.dylib.3 @executable_path/urelibs/libstore.dylib.3

INSTALL_NAME_URELIBS_BIN=install_name_tool -change @__________________________________________________URELIB/libuno_sal.dylib.3 libuno_sal.dylib.3 -change  @__________________________________________________URELIB/libuno_cppu.dylib.3 libuno_cppu.dylib.3 -change @__________________________________________________URELIB/libuno_cppuhelper$(COMID).dylib.3 libuno_cppuhelper$(COMID).dylib.3 -change @__________________________________________________URELIB/libuno_salhelper$(COMID).dylib.3 libuno_salhelper$(COMID).dylib.3 -change @__________________________________________________URELIB/libreg.dylib.3 libreg.dylib.3 -change @__________________________________________________URELIB/libstore.dylib.3 libstore.dylib.3

EMPTYSTRING=
PATH_SEPARATOR=:

CC_FLAGS_JNI=-malign-natural -c -fPIC -fno-common $(GCC_ARCH_OPTION)
CC_FLAGS=-malign-natural -c -fPIC -fno-common $(GCC_ARCH_OPTION) -fvisibility=hidden
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
CC_DEFINES_JNI=-DUNX -DGCC -DMACOSX -DCPPU_ENV=$(CPPU_ENV) -DGXX_INCLUDE_PATH=$(SDK_GXX_INCLUDE_PATH)
CC_DEFINES=-DUNX -DGCC -DMACOSX -DCPPU_ENV=$(CPPU_ENV) -DGXX_INCLUDE_PATH=$(SDK_GXX_INCLUDE_PATH) -DHAVE_GCC_VISIBILITY_FEATURE

CC_OUTPUT_SWITCH=-o

LIBRARY_LINK_FLAGS=-dynamiclib -single_module -Wl,-multiply_defined,suppress $(GCC_ARCH_OPTION)

# install_name '@executable_path$/(@:f)'
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS)

EXE_LINK_FLAGS=$(GCC_ARCH_OPTION) -Wl,-multiply_defined,suppress
LINK_LIBS=-L$(OUT)/lib -L$(OO_SDK_OUT)/$(PLATFORM)/lib -L"$(OO_SDK_URE_LIB_DIR)"
LINK_JAVA_LIBS=-framework JavaVM

ifneq "$(OO_SDK_URE_HOME)" ""
URE_MISC=$(OO_SDK_URE_HOME)/share/misc
endif

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
CC_DEFINES_JNI=-DUNX -DGCC -DFREEBSD -DCPPU_ENV=$(CPPU_ENV) -DGXX_INCLUDE_PATH=$(SDK_GXX_INCLUDE_PATH)
CC_DEFINES=-DUNX -DGCC -DFREEBSD -DCPPU_ENV=$(CPPU_ENV) -DGXX_INCLUDE_PATH=$(SDK_GXX_INCLUDE_PATH) -DHAVE_GCC_VISIBILITY_FEATURE

CC_OUTPUT_SWITCH=-o

LIBRARY_LINK_FLAGS=-shared '-Wl,-z,origin -Wl,-rpath,$$ORIGIN'
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS)

EXE_LINK_FLAGS=-Wl,--allow-shlib-undefined 
LINK_LIBS=-L"$(OUT)/lib" -L"$(OO_SDK_HOME)/lib" -L"$(OO_SDK_URE_LIB_DIR)" $(PTHREAD_LIBS)
LINK_JAVA_LIBS=-L"$(OO_SDK_JAVA_HOME)/jre/lib/$(JAVA_PROC_TYPE)"

ifneq "$(OO_SDK_URE_HOME)" ""
URE_MISC=$(OO_SDK_URE_HOME)/share/misc
endif

endif
