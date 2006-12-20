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


###########################################################################
#
# Windows specific settings
#
###########################################################################
ifeq "$(PLATFORM)" "windows"
# Settings for Windows using Microsoft compiler/linker

OS=WIN
PS=\\
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
PACKAGE_LIB_DIR=windows.plt
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
STLPORTLIB=stlport_vc71.lib

BLANK= 
EMPTYSTRING=
PATH_SEPARATOR=;

# use this for release version
CC_FLAGS=-c -MT -Zm500 -wd4251 -wd4275 -wd4290 -wd4675 -wd4786 -wd4800 -Zc:forScope -GR
ifeq "$(CPP_VC8)" "true"
CC_FLAGS+=-EHa -Zc:wchar_t-
LINK_MANIFEST_VC8_ONLY=mt -manifest $@.manifest -outputresource:$@;2
else
CC_FLAGS+=-GX
LINK_MANIFEST_VC8_ONLY=
endif
ifeq "$(DEBUG)" "yes"
CC_FLAGS+=-Zi
endif

CC_INCLUDES=-I. -I$(OUT)/inc/examples -I$(PRJ)/include
STL_INCLUDES=-I"$(OO_SDK_HOME)/include/stl"
SDK_JAVA_INCLUDES = -I"$(OO_SDK_JAVA_HOME)/include" -I"$(OO_SDK_JAVA_HOME)/include/win32"

# define for used compiler necessary for UNO
# -DCPPU_ENV=msci -- windows msvc 4.x - 7.x

CC_DEFINES=-DWIN32 -DWNT -DCPPU_ENV=msci
CC_OUTPUT_SWITCH=-Fo

LIBRARY_LINK_FLAGS=/NODEFAULTLIB /DLL /DEBUGTYPE:cv
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS) /DEF:$(PRJ)/settings/component.uno.def
EXE_LINK_FLAGS=/MAP /OPT:NOREF /SUBSYSTEM:CONSOLE /BASE:0x1100000 /DEBUGTYPE:cv
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
#LIBRARY_LINK_FLAGS=/NODEFAULTLIB /DLL
endif	



###########################################################################
#
# Solaris specific settings
#
###########################################################################
ifneq (,$(findstring solaris,$(PLATFORM)))
# Settings for Solaris using Sun Workshop compiler

PROCTYPE := $(shell $(PRJ)/config.guess | cut -d"-" -f1)

ifeq "$(PROCTYPE)" "sparc"
PLATFORM=solsparc
PACKAGE_LIB_DIR=solaris_sparc.plt
UNOPKG_PLATFORM=Solaris_SPARC
JAVA_PROC_TYPE=sparc
else
PLATFORM=solintel
PACKAGE_LIB_DIR=solaris_x86.plt
UNOPKG_PLATFORM=Solaris_x86
JAVA_PROC_TYPE=i386
endif

OS=SOLARIS
PS=/
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
STLPORTLIB=-lstlport_sunpro

EMPTYSTRING=
PATH_SEPARATOR=:

CC_FLAGS=-c -KPIC
ifeq "$(DEBUG)" "yes"
CC_FLAGS+=-g
endif
CC_INCLUDES=-I. -I$(OUT)/inc/examples -I$(PRJ)/include
STL_INCLUDES=-I"$(OO_SDK_HOME)/include/stl"
SDK_JAVA_INCLUDES = -I"$(OO_SDK_JAVA_HOME)/include" -I"$(OO_SDK_JAVA_HOME)/include/solaris"

# define for used compiler necessary for UNO
# -DCPPU_ENV=sunpro5 -- sunpro cc 5.x solaris sparc/intel

CC_DEFINES=-DUNX -DSOLARIS -DSPARC -DCPPU_ENV=sunpro5
CC_OUTPUT_SWITCH=-o 

LIBRARY_LINK_FLAGS=-w -mt -z combreloc -PIC -temp=/tmp '-R $$ORIGIN' -z text -norunpath -G -Bdirect -Bdynamic -lpthread -lCrun -lc -lm
# means if used CC is lower then version 5.5 use option -instance=static
ifeq ($(OO_SDK_CC_55_OR_HIGHER),)
LIBRARY_LINK_FLAGS+=-instances=static
endif
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS) -M $(PRJ)/settings/component.uno.map

LINK_LIBS=-L$(OUT)/lib -L$(PRJ)/$(PLATFORM)/lib -L"$(OFFICE_PROGRAM_PATH)"
LINK_JAVA_LIBS=-L"$(OO_SDK_JAVA_HOME)/jre/lib/$(JAVA_PROC_TYPE)"

ifneq "$(OO_SDK_URE_HOME)" ""
URE_MISC=$(OO_SDK_URE_HOME)/share/misc
endif

endif



###########################################################################
#
# Linux specific settings
#
###########################################################################
ifeq "$(PLATFORM)" "linux-gnu"
# Settings for Linux using gcc compiler

PROCTYPE := $(shell $(PRJ)/config.guess | cut -d "-" -f1)

# Default is linux on a intel machine    
PLATFORM=linux
PACKAGE_LIB_DIR=linux_x86.plt
UNOPKG_PLATFORM=Linux_x86
JAVA_PROC_TYPE=i386

ifeq "$(PROCTYPE)" "x86_64"
PACKAGE_LIB_DIR=linux_x86_64.plt
UNOPKG_PLATFORM=Linux_x86_64
# needs deeper investigation for intel 64 bit
JAVA_PROC_TYPE=amd64
endif

ifeq "$(PROCTYPE)" "powerpc"
PACKAGE_LIB_DIR=linux_powerpc.plt
UNOPKG_PLATFORM=Linux_PowerPC
JAVA_PROC_TYPE=ppc
endif

OS=LINUX
PS=/
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
STLPORTLIB=-lstlport_gcc

EMPTYSTRING=
PATH_SEPARATOR=:

# -O is necessary for inlining (see gcc documentation)
ifeq "$(DEBUG)" "yes"
CC_FLAGS=-c -g -fpic
else
CC_FLAGS=-c -O -fpic
endif

ifeq "$(PROCTYPE)" "ppc"
CC_FLAGS+=-fPIC
endif

SDK_JAVA_INCLUDES = -I"$(OO_SDK_JAVA_HOME)/include" -I"$(OO_SDK_JAVA_HOME)/include/linux"
CC_INCLUDES=-I. -I$(OUT)/inc/examples -I$(PRJ)/include
STL_INCLUDES=-I"$(OO_SDK_HOME)/include/stl"
CC_DEFINES=-DUNX -DGCC -DLINUX -DCPPU_ENV=$(CPPU_ENV) -DGXX_INCLUDE_PATH=$(SDK_GXX_INCLUDE_PATH)

# define for used compiler necessary for UNO
#-DCPPU_ENV=gcc2 -- gcc 2.91/2.95
#-DCPPU_ENV=gcc3 -- gcc3 3.0

CC_OUTPUT_SWITCH=-o

LIBRARY_LINK_FLAGS=-shared -Wl,-rpath,$$ORIGIN

ifeq "$(PROCTYPE)" "ppc"
LIBRARY_LINK_FLAGS+=-fPIC
endif
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS) -Wl,--version-script,$(PRJ)/settings/component.uno.map

EXE_LINK_FLAGS=-Wl,--allow-shlib-undefined -Wl,-export-dynamic -Wl,-z,defs -Wl,--whole-archive -lsalcpprt -Wl,--no-whole-archive
LINK_LIBS=-L$(OUT)/lib -L$(PRJ)/$(PLATFORM)/lib -L"$(OFFICE_PROGRAM_PATH)"
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
# Settings for MacOSX using gcc 3.3 compiler

# Default is MacOSX on a ppc machine    
PLATFORM=macosx
PACKAGE_LIB_DIR=macosx_ppc.plt
UNOPKG_PLATFORM=MacOSX_PowerPC
JAVA_PROC_TYPE=ppc

OS=MACOSX
PS=/
CC=gcc
LINK=g++
LIB=g++
ECHO=@echo
MKDIR=mkdir -p
CAT=cat
OBJ_EXT=o
SHAREDLIB_EXT=dylib
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
STLPORTLIB=-lstlport_gcc -lstdc++

EMPTYSTRING=
PATH_SEPARATOR=:

# -O is necessary for inlining (see gcc documentation)
ifeq "$(DEBUG)" "yes"
CC_FLAGS=-malign-natural -c -g -fPIC -fno-common
else
CC_FLAGS=-malign-natural -c -O -fPIC -fno-common
endif

SDK_JAVA_INCLUDES = -I/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers -I/System/Library/Frameworks/JavaVM.framework/Headers
CC_INCLUDES=-I. -I$(OUT)/inc/examples -I$(PRJ)/include
STL_INCLUDES=-I"$(OO_SDK_HOME)/include/stl"
CC_DEFINES=-DUNX -DGCC -DMACOSX -DCPPU_ENV=$(CPPU_ENV) -DGXX_INCLUDE_PATH=$(SDK_GXX_INCLUDE_PATH)

CC_OUTPUT_SWITCH=-o

LIBRARY_LINK_FLAGS=-dynamiclib -single_module -fPIC -fno-common

# install_name '@executable_path$/(@:f)'
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS) 
#-Wl,--version-script,$(PRJ)/settings/component.uno.map

#EXE_LINK_FLAGS=-Wl,--allow-shlib-undefined -Wl,-export-dynamic -Wl,-z,defs
LINK_LIBS=-L$(OUT)/lib -L$(PRJ)/$(PLATFORM)/lib -L"$(OFFICE_PROGRAM_PATH)"
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

# Default is freebsd on a intel machine    
PLATFORM=freebsd
PACKAGE_LIB_DIR=freebsd_x86.plt
UNOPKG_PLATFORM=FreeBSD_x86
JAVA_PROC_TYPE=i386

OS=FREEBSD
PS=/
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

ifeq "$(shell echo $(GCC_VERSION) | cut -c 1)" "3"
COMID=gcc3
CPPU_ENV=gcc3
else
COMID=GCC
CPPU_ENV=gcc2
endif

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
STLPORTLIB=-lstlport_gcc

EMPTYSTRING=
PATH_SEPARATOR=:

# -O is necessary for inlining (see gcc documentation)
ifeq "$(DEBUG)" "yes"
CC_FLAGS=-c -g -fPIC -DPIC $(PTHREAD_CFLAGS)
else
CC_FLAGS=-c -O -fPIC -DPIC $(PTHREAD_CFLAGS)
endif

SDK_JAVA_INCLUDES = -I"$(OO_SDK_JAVA_HOME)/include" -I"$(OO_SDK_JAVA_HOME)/include/freebsd"
CC_INCLUDES=-I. -I$(OUT)/inc/examples -I$(PRJ)/include
STL_INCLUDES=-I"$(OO_SDK_HOME)/include/stl"
CC_DEFINES=-DUNX -DGCC -DFREEBSD -DCPPU_ENV=$(CPPU_ENV) -DGXX_INCLUDE_PATH=$(SDK_GXX_INCLUDE_PATH)

CC_OUTPUT_SWITCH=-o

LIBRARY_LINK_FLAGS=-shared '-Wl,-rpath,$$ORIGIN'
COMP_LINK_FLAGS=$(LIBRARY_LINK_FLAGS) -Wl,--version-script,$(PRJ)/settings/component.uno.map

EXE_LINK_FLAGS=-Wl,--allow-shlib-undefined 
#EXE_LINK_FLAGS+=-Wl,-export-dynamic -Wl,-z,defs
LINK_LIBS=-L$(OUT)/lib -L$(PRJ)/$(PLATFORM)/lib -L"$(OFFICE_PROGRAM_PATH)" $(PTHREAD_LIBS)
LINK_JAVA_LIBS=-L"$(OO_SDK_JAVA_HOME)/jre/lib/$(JAVA_PROC_TYPE)"

ifneq "$(OO_SDK_URE_HOME)" ""
URE_MISC=$(OO_SDK_URE_HOME)/share/misc
endif

endif
