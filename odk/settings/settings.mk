# Global settings file for the minimal build environment of the SDK
# This file have to updated/extended for other platforms.

# test for the platform
PLATFORM := $(shell uname -s)

# special check for windows because normally we have no uname under windows
ifeq "$(PLATFORM)" ""
PLATFORM = windows
else
ifeq "$(PLATFORM)" "WINNT"
PLATFORM = windows
endif
ifeq "$(PLATFORM)" "WindowsNT"
PLATFORM = windows
endif
ifeq "$(PLATFORM)" "CYGWIN_NT-5.0"
PLATFORM = windows
endif
endif

# debug option, default is no debug
DEBUG=no
ifeq "$(MAKECMDGOALS)" "debug"
DEBUG=yes
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
    
OSEP=^<
CSEP=^>
QUOTE=

DEL=del
DELRECURSIVE=rd /S /Q
URLPREFIX=file:///

SALLIB=isal.lib
CPPULIB=icppu.lib
CPPUHELPERLIB=icppuhelper.lib
SALHELPERLIB=isalhelper.lib
STLPORTLIB=stlport_vc6.lib

BLANK= 
EMPTYSTRING=
PATH_SEPARATOR=;

# use this for release version
CC_FLAGS=-c -GX
ifeq "$(DEBUG)" "yes"
CC_FLAGS+=-Zi -MT
endif

CC_INCLUDES=-I. -I$(OUT)/inc/examples -I$(PRJ)/include
STL_INCLUDES=-I$(OO_STLPORT_HOME)/stlport
SDK_JAVA_INCLUDES = -I$(OO_SDK_JAVA_HOME)/include -I$(OO_SDK_JAVA_HOME)/include/win32

# define for used compiler necessary for UNO
# -DCPPU_ENV=msci -- windows msvc 4.x - 7.x

CC_DEFINES=-DWIN32 -DCPPU_ENV=msci
CC_OUTPUT_SWITCH=-Fo

LIBRARY_LINK_FLAGS=/NODEFAULTLIB /DLL /DEBUG:notmapped,full /DEBUGTYPE:cv
EXE_LINK_FLAGS=/MAP /OPT:NOREF /SUBSYSTEM:CONSOLE /BASE:0x1100000 /DEBUG:notmapped,full /DEBUGTYPE:cv

LINK_JAVA_LIBS=/LIBPATH:$(OO_SDK_JAVA_HOME)/lib


# use this for release version
#EXE_LINK_FLAGS=/MAP /OPT:NOREF /SUBSYSTEM:CONSOLE /BASE:0x1100000
#LIBRARY_LINK_FLAGS=/NODEFAULTLIB /DLL
endif	



###########################################################################
#
# Solaris specific settings
#
###########################################################################
ifeq "$(PLATFORM)" "SunOS"
# Settings for Solaris using Sun Workshop compiler

PROCTYPE := $(shell uname -p)

ifeq "$(PROCTYPE)" "sparc"
PLATFORM=solsparc
PACKAGE_LIB_DIR=solaris_sparc.plt
JAVA_PROC_TYPE=sparc
else
PLATFORM=solintel
PACKAGE_LIB_DIR=solaris_intel.plt
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

DEL=rm -f
DELRECURSIVE=rm -rf
COPY=cp
URLPREFIX=file://

# Include UDK version numbers
include $(PRJ)/include/udkversion.mk

SALLIB=-lsal
CPPULIB=-lcppu
CPPUHELPERLIB=-lcppuhelperC52
SALHELPERLIB=-lsalhelperC52
STLPORTLIB=-lstlport_sunpro

EMPTYSTRING=
PATH_SEPARATOR=:

CC_FLAGS=-c -KPIC
ifeq "$(DEBUG)" "yes"
CC_FLAGS+=-g
endif
CC_INCLUDES=-I. -I/usr/include -I$(OUT)/inc/examples -I$(PRJ)/include
STL_INCLUDES=-I$(OO_STLPORT_HOME)/stlport
SDK_JAVA_INCLUDES = -I$(OO_SDK_JAVA_HOME)/include -I$(OO_SDK_JAVA_HOME)/include/solaris

# define for used compiler necessary for UNO
# -DCPPU_ENV=sunpro5 -- sunpro cc 5.x solaris sparc/intel

CC_DEFINES=-DUNX -DSOLARIS -DSPARC -DCPPU_ENV=sunpro5
CC_OUTPUT_SWITCH=-o 

LIBRARY_LINK_FLAGS=-w -PIC -temp=/tmp -R /usr/lib -G -lthread -instances=static 
LINK_LIBS=-L$(OUT)/lib -L$(PRJ)/$(PLATFORM)/lib -L$(OFFICE_PROGRAM_PATH)
LINK_JAVA_LIBS=-L$(OO_SDK_JAVA_HOME)/jre/lib/$(JAVA_PROC_TYPE)

endif



###########################################################################
#
# Linux specific settings
#
###########################################################################
ifeq "$(PLATFORM)" "Linux"
# Settings for Linux using gcc compiler

PROCTYPE := $(shell uname -m)

    
# Default is linux on a intel machine    
PLATFORM=linux
PACKAGE_LIB_DIR=linux_x86.plt
JAVA_PROC_TYPE=i386
    
ifeq "$(PROCTYPE)" "ppc"
PACKAGE_LIB_DIR=linux_powerpc.plt
JAVA_PROC_TYPE=ppc
endif

OS=LINUX
PS=/
CC=gcc
LINK=gcc
LIB=gcc
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
    
DEL=rm -f
DELRECURSIVE=rm -rf
COPY=cp
URLPREFIX=file://

# Include UDK version numbers
include $(PRJ)/include/udkversion.mk

SALLIB=-lsal
CPPULIB=-lcppu
CPPUHELPERLIB=-lcppuhelper$(COMID)
SALHELPERLIB=-lsalhelper$(COMID)
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

SDK_JAVA_INCLUDES = -I$(OO_SDK_JAVA_HOME)/include -I$(OO_SDK_JAVA_HOME)/include/linux
CC_INCLUDES=-I. -I/usr/include -I$(OUT)/inc/examples -I$(PRJ)/include
STL_INCLUDES=-I$(OO_STLPORT_HOME)/stlport
CC_DEFINES=-DUNX -DGCC -DLINUX -DCPPU_ENV=$(CPPU_ENV)

# define for used compiler necessary for UNO
#-DCPPU_ENV=gcc2 -- gcc 2.91/2.95
#-DCPPU_ENV=gcc3 -- gcc3 3.0

CC_OUTPUT_SWITCH=-o

LIBRARY_LINK_FLAGS=-shared

ifeq "$(PROCTYPE)" "ppc"
LIBRARY_LINK_FLAGS+=-fPIC
endif

EXE_LINK_FLAGS=-Wl -export-dynamic
LINK_LIBS=-L$(OUT)/lib -L$(PRJ)/$(PLATFORM)/lib -L$(OFFICE_PROGRAM_PATH)
LINK_JAVA_LIBS=-L$(OO_SDK_JAVA_HOME)/jre/lib/$(JAVA_PROC_TYPE)

endif

