#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: unxmacx.mk,v $
#
#   $Revision: 1.29 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:11:14 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

##########################################################################
# Platform MAKEFILE for Mac OS X and Darwin on both PowerPC and Intel
##########################################################################

# PROCESSOR_DEFINES and DLLPOSTFIX are defined in the particular platform file

# DARWIN_VERSION holds the Darwin version in the format: 000000. For example,
# if the Darwin version is 1.3.7, DARWIN_VERSION will be set to 010307.
# Not used now, comment it out. Remove it after some time.
# DARWIN_VERSION=$(shell -/bin/sh -c "uname -r | sed 's/\./ /g' | xargs printf %2.2i%2.2i%2.2i")

ASM=
AFLAGS=
LINKOUTPUT_FILTER=

# Definitions that we may need on the compile line.
# -D_PTHREADS and -D_REENTRANT are needed for STLport, and must be specified when
#  compiling STLport sources too, either internally or externally.
CDEFS+=-DGLIBC=2 -D_PTHREADS -D_REENTRANT -DNO_PTHREAD_PRIORITY $(PROCESSOR_DEFINES) -DSTLPORT_VERSION=$(STLPORT_VER) -D_USE_NAMESPACE=1
.IF "$(GUIBASE)"=="unx"
CDEFS+= -DX_LOCALE
.ENDIF

# Name of library where static data members are initialized
# STATICLIBNAME=static$(DLLPOSTFIX)
# STATICLIB=-l$(STATICLIBNAME)

# enable visibility define in "sal/types.h"
.IF "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"
CDEFS += -DHAVE_GCC_VISIBILITY_FEATURE
.ENDIF # "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"


# MacOS X specific Java compilation/link flags
SOLAR_JAVA*=TRUE
.IF "$(SOLAR_JAVA)"!=""
    JAVADEF=-DSOLAR_JAVA
    JAVAFLAGSDEBUG=-g
    JAVA_RUNTIME=-framework JavaVM
.ENDIF

# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
ARCH_FLAGS*=

# Specify the compiler to use.  NOTE:  MacOS X should always specify
# c++ for C++ compilation as it does certain C++ specific things
# behind the scenes for us.
# CC = C++ compiler to use
# cc = C compiler to use
# objc = Objective C compiler to use
# objcpp = Objective C++ compiler to use
CXX*=g++
CC*=gcc
objc*=gcc
objcpp*=g++

CFLAGS=-fsigned-char -fmessage-length=0 -malign-natural -c $(EXTRA_CFLAGS)

.IF "$(DISABLE_DEPRECATION_WARNING)" == "TRUE"
CFLAGS+=-Wno-deprecated-declarations
.ENDIF
# ---------------------------------
#  Compilation flags
# ---------------------------------
# Normal C compilation flags
CFLAGSCC=-pipe -fsigned-char -malign-natural $(ARCH_FLAGS)

# Normal Objective C compilation flags
#OBJCFLAGS=-no-precomp
OBJCFLAGS=-fobjc-exceptions
OBJCXXFLAGS=-x objective-c++ -fobjc-exceptions

# Comp Flags for files that need exceptions enabled (C and C++)
CFLAGSEXCEPTIONS=-fexceptions -fno-enforce-eh-specs

# Comp Flags for files that do not need exceptions enabled (C and C++)
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

# Normal C++ compilation flags
CFLAGSCXX=-pipe -malign-natural -fsigned-char -Wno-long-double $(ARCH_FLAGS)
CFLAGSCXX+= -Wno-ctor-dtor-privacy

PICSWITCH:=-fPIC
# Other flags
CFLAGSOBJGUIMT=$(PICSWITCH) -fno-common
CFLAGSOBJCUIMT=$(PICSWITCH) -fno-common
CFLAGSSLOGUIMT=$(PICSWITCH) -fno-common
CFLAGSSLOCUIMT=$(PICSWITCH) -fno-common
CFLAGSPROF=

# Flag for including debugging information in object files
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=

# Flag to specify output file to compiler/linker
CFLAGSOUTOBJ=-o

# Flags to enable precompiled headers
CFLAGS_CREATE_PCH=-x c++-header -I$(INCPCH) -DPRECOMPILED_HEADERS
CFLAGS_USE_PCH=-I$(SLO)$/pch -DPRECOMPILED_HEADERS -Winvalid-pch
CFLAGS_USE_EXCEPTIONS_PCH=-I$(SLO)$/pch_ex -DPRECOMPILED_HEADERS -Winvalid-pch

# ---------------------------------
#  Optimization flags
# ---------------------------------
CFLAGSOPT=-O2 -fno-strict-aliasing
CFLAGSNOOPT=-O0

# -Wshadow does not work for C with nested uses of pthread_cleanup_push:
# -Wshadow does not work for C++ as /usr/include/c++/4.0.0/ext/hashtable.h
# l. 717 contains a declaration of __cur2 shadowing the declaration at l. 705,
# in template code for which a #pragma gcc system_header would not work:
# -Wextra doesn not work for gcc-3.3
CFLAGSWARNCC=-Wall -Wendif-labels
CFLAGSWARNCXX=$(CFLAGSWARNCC) -Wno-ctor-dtor-privacy -Wno-non-virtual-dtor
CFLAGSWALLCC=$(CFLAGSWARNCC)
CFLAGSWALLCXX=$(CFLAGSWARNCXX)
CFLAGSWERRCC=-Werror

# Once all modules on this platform compile without warnings, set
# COMPILER_WARN_ERRORS=TRUE here instead of setting MODULES_WITH_WARNINGS (see
# settings.mk):
MODULES_WITH_WARNINGS := 

#special settings form environment
CDEFS+=$(EXTRA_CDEFS)

STDLIBCPP=-lstdc++

# ---------------------------------
#  STLport library names
# ---------------------------------
.IF "$(USE_STLP_DEBUG)" != ""
.IF "$(STLPORT_VER)" >= "500"
LIBSTLPORT=-lstlportstlg
LIBSTLPORTST=$(STATIC) -lstlportstlg
.ELSE
LIBSTLPORT=-lstlport_gcc_stldebug
LIBSTLPORTST=$(SOLARVERSION)$/$(INPATH)$/lib$/libstlport_gcc_stldebug.a
.ENDIF
.ELSE # "$(USE_STLP_DEBUG" != ""
.IF "$(STLPORT_VER)" >= "500"
LIBSTLPORT=-lstlport
LIBSTLPORTST=$(STATIC) -lstlport
.ELSE
LIBSTLPORT=-lstlport_gcc
LIBSTLPORTST=$(SOLARVERSION)$/$(INPATH)$/lib$/libstlport_gcc.a
.ENDIF
.ENDIF # "$(USE_STLP_DEBUG" != ""

# ---------------------------------
#  Link stage flags
# ---------------------------------
# always link with gcc since you may be linking c code and don't want -lstdc++ linked in!

##  ericb 04 mars 2005

LINK*=$(CXX)
LINKC*=$(CC)

LINKFLAGSDEFS*=-Wl,-multiply_defined,suppress
# Very long install_names are needed so that install_name_tool -change later on
# does not complain that "larger updated load commands do not fit:"
LINKFLAGSRUNPATH_URELIB=-install_name '@__________________________________________________URELIB$/$(@:f)'
LINKFLAGSRUNPATH_UREBIN=
LINKFLAGSRUNPATH_OOO=-install_name '@__________________________________________________OOO$/$(@:f)'
LINKFLAGSRUNPATH_BRAND=
LINKFLAGSRUNPATH_OXT=
LINKFLAGS=$(LINKFLAGSDEFS)

# [ed] 5/14/02 If we're building for aqua, add in the objc runtime library into our link line
.IF "$(GUIBASE)" == "aqua"
    LINKFLAGS+=-lobjc
    # Sometimes we still use files that would be in a GUIBASE="unx" specific directory
    # because they really aren't GUIBASE specific, so we've got to account for that here.
    INCGUI+= -I$(PRJ)$/unx/inc
.ENDIF

#special settings form environment
LINKFLAGS+=$(EXTRA_LINKFLAGS)

# Random link flags dealing with different cases of linking

LINKFLAGSAPPGUI=-bind_at_load
LINKFLAGSSHLGUI=-dynamiclib -single_module
LINKFLAGSAPPCUI=-bind_at_load
LINKFLAGSSHLCUI=-dynamiclib -single_module
LINKFLAGSTACK=
LINKFLAGSPROF=

# Flag to add debugging information to final products
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

# ---------------------------------
#  MacOS X shared library specifics
# ---------------------------------

# Tag to identify an output file as a library
DLLPRE=lib
# File extension to identify dynamic shared libraries on MacOS X
DLLPOST=.dylib
# Precompiled header file extension
PCHPOST=.gch

# We don't use mapping on MacOS X
#LINKVERSIONMAPFLAG=-Wl,--version-script
LINKVERSIONMAPFLAG=-Wl,-exported_symbols_list

SONAME_SWITCH=-Wl,-h

STDLIBCPP=-lstdc++

STDOBJVCL=$(L)$/salmain.o
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

.IF "$(GUIBASE)" == "aqua"
    STDLIBCUIMT=CPPRUNTIME -lm
    STDLIBGUIMT=-framework Carbon -framework Cocoa -lpthread CPPRUNTIME -lm
    STDSHLCUIMT=-lpthread CPPRUNTIME -lm
    STDSHLGUIMT=-framework Carbon -framework CoreFoundation -framework Cocoa -lpthread CPPRUNTIME -lm
    PSPLIB=-lpsp
.ELSE
    STDLIBCUIMT= CPPRUNTIME -lm
    STDLIBGUIMT=-lX11 -lpthread CPPRUNTIME -lm
    STDSHLCUIMT=-lpthread CPPRUNTIME -lm
    STDSHLGUIMT=-lX11 -lXext -lpthread CPPRUNTIME -lm -framework CoreFoundation
.ENDIF

LIBMGR=ar
LIBFLAGS=-r

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=
