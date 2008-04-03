#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: unxlngmips.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 16:45:38 $
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

# mk file for unxlngmips
ASM=
AFLAGS=

SOLAR_JAVA*=
JAVAFLAGSDEBUG=-g

# filter for supressing verbose messages from linker
#not needed at the moment
#LINKOUTPUT_FILTER=" |& $(SOLARENV)$/bin$/msg_filter"

# _PTHREADS is needed for the stl
CDEFS+=$(PTHREAD_CFLAGS) -DGLIBC=2 -DMIPS -D_PTHREADS -D_REENTRANT -DNEW_SOLAR -D_USE_NAMESPACE=1 -DSTLPORT_VERSION=400

# enable visibility define in "sal/types.h"
.IF "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"
CDEFS += -DHAVE_GCC_VISIBILITY_FEATURE
.ENDIF # "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"

# this is a platform with JAVA support
.IF "$(SOLAR_JAVA)"!=""
JAVADEF=-DSOLAR_JAVA
.IF "$(debug)"==""
JAVA_RUNTIME=-ljava
.ELSE
JAVA_RUNTIME=-ljava_g
.ENDIF
.ENDIF

# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
#ARCH_FLAGS*=-mtune=pentiumpro

# name of C++ Compiler
CXX*=g++
# name of C Compiler
CC*=gcc
.IF "$(SYSBASE)"!=""
CFLAGS_SYSBASE:=-isystem $(SYSBASE)$/usr$/include
CXX+:=$(CFLAGS_SYSBASE)
CC+:=$(CFLAGS_SYSBASE)
.ENDIF          # "$(SYSBASE)"!=""
CFLAGS+=-fmessage-length=0 -c

# flags to enable build with symbols; required for crashdump feature
.IF "$(ENABLE_SYMBOLS)"=="SMALL"
CFLAGSENABLESYMBOLS=-g1
.ELSE
CFLAGSENABLESYMBOLS=-g # was temporarily commented out, reenabled before Beta

.ENDIF

# flags for the C++ Compiler
CFLAGSCC= -pipe $(ARCH_FLAGS)
# Flags for enabling exception handling
CFLAGSEXCEPTIONS=-fexceptions -fno-enforce-eh-specs
# Flags for disabling exception handling
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

# -fpermissive should be removed as soon as possible
CFLAGSCXX= -pipe $(ARCH_FLAGS)
PICSWITCH:=-fpic
.IF "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"
CFLAGSCXX += -fvisibility-inlines-hidden
.ENDIF # "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"

# Compiler flags for compiling static object in single threaded environment with graphical user interface
CFLAGSOBJGUIST=
# Compiler flags for compiling static object in single threaded environment with character user interface
CFLAGSOBJCUIST=
# Compiler flags for compiling static object in multi threaded environment with graphical user interface
CFLAGSOBJGUIMT=
# Compiler flags for compiling static object in multi threaded environment with character user interface
CFLAGSOBJCUIMT=
# Compiler flags for compiling shared object in multi threaded environment with graphical user interface
CFLAGSSLOGUIMT=$(PICSWITCH)
# Compiler flags for compiling shared object in multi threaded environment with character user interface
CFLAGSSLOCUIMT=$(PICSWITCH)
# Compiler flags for profiling
CFLAGSPROF=
# Compiler flags for debugging
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
# Compiler flags for enabling optimizations
.IF "$(PRODUCT)"!=""
CFLAGSOPT=-Os -fno-strict-aliasing		# optimizing for products
.ELSE 	# "$(PRODUCT)"!=""
CFLAGSOPT=   							# no optimizing for non products
.ENDIF	# "$(PRODUCT)"!=""
# Compiler flags for disabling optimizations
CFLAGSNOOPT=-O0
# Compiler flags for describing the output path
CFLAGSOUTOBJ=-o

# -Wshadow does not work for C with nested uses of pthread_cleanup_push:
CFLAGSWARNCC=-Wall -Wextra -Wendif-labels
CFLAGSWARNCXX=$(CFLAGSWARNCC) -Wshadow -Wno-ctor-dtor-privacy \
    -Wno-non-virtual-dtor
CFLAGSWALLCC=$(CFLAGSWARNCC)
CFLAGSWALLCXX=$(CFLAGSWARNCXX)
CFLAGSWERRCC=-Werror

# Once all modules on this platform compile without warnings, set
# COMPILER_WARN_ERRORS=TRUE here instead of setting MODULES_WITH_WARNINGS (see
# settings.mk):
MODULES_WITH_WARNINGS := \
    b_server \
    basctl \
    basebmp \
    chart2 \
    cppcanvas \
    desktop \
    devtools \
    dxcanvas \
    extensions \
    filter \
    glcanvas \
    lingu \
    r_tools \
    sc \
    sd \
    slideshow \
    starmath \
    svx \
    sw \
    writerperfect \
    xmlsecurity

# switches for dynamic and static linking
STATIC		= -Wl,-Bstatic
DYNAMIC		= -Wl,-Bdynamic

# name of linker
LINK*=$(CXX)
LINKC*=$(CC)

# default linker flags
LINKFLAGSDEFS*=-Wl,-z,defs
LINKFLAGSRUNPATH_URELIB=-Wl,-rpath,\''$$ORIGIN'\'
LINKFLAGSRUNPATH_UREBIN=-Wl,-rpath,\''$$ORIGIN/../lib:$$ORIGIN'\'
    #TODO: drop $ORIGIN once no URE executable is also shipped in OOo
LINKFLAGSRUNPATH_OOO=-Wl,-rpath,\''$$ORIGIN:$$ORIGIN/../ure-link/lib'\'
LINKFLAGSRUNPATH_BRAND=-Wl,-rpath,\''$$ORIGIN:$$ORIGIN/../basis-link/program:$$ORIGIN/../basis-link/ure-link/lib'\'
LINKFLAGSRUNPATH_OXT=
LINKFLAGS=-Wl,-z,combreloc $(LINKFLAGSDEFS)

# linker flags for linking applications
LINKFLAGSAPPGUI= -Wl,-export-dynamic -Wl,--noinhibit-exec
LINKFLAGSAPPCUI= -Wl,-export-dynamic -Wl,--noinhibit-exec

# linker flags for linking shared libraries
LINKFLAGSSHLGUI= -shared
LINKFLAGSSHLCUI= -shared

LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

# linker flags for optimization (symbol hashtable)
# for now, applied to symbol scoped libraries, only
LINKFLAGSOPTIMIZE*=-Wl,-O1
LINKVERSIONMAPFLAG=$(LINKFLAGSOPTIMIZE) -Wl,--version-script

SONAME_SWITCH=-Wl,-h

# Sequence of libs does matter !

STDLIBCPP=-lstdc++

# default objectfilenames to link
STDOBJVCL=$(L)$/salmain.o
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

.IF "$(ALLOC)" == "TCMALLOC"
STDLIBCUIST+=-ltcmalloc
STDLIBGUIMT+=-ltcmalloc
STDLIBCUIMT+=-ltcmalloc
STDLIBGUIST+=-ltcmalloc
STDSHLGUIMT+=-ltcmalloc
STDSHLCUIMT+=-ltcmalloc
STDSHLGUIST+=-ltcmalloc
STDSHLCUIST+=-ltcmalloc
.ENDIF

# libraries for linking applications
STDLIBCUIST+=-ldl -lm
STDLIBGUIMT+=-lX11 -lXau -ldl -lpthread -lm
STDLIBCUIMT+=-ldl -lpthread -lm
STDLIBGUIST+=-lX11 -lXau -ldl -lm
# libraries for linking shared libraries
STDSHLGUIMT+=-lX11 -lXau -lXext -ldl -lpthread -lm
STDSHLCUIMT+=-ldl -lpthread -lm
STDSHLGUIST+=-lX11 -lXau -lXext -ldl -lm
STDSHLCUIST+=-ldl -lm

LIBSALCPPRT*=-Wl,--whole-archive -lsalcpprt -Wl,--no-whole-archive

.IF "$(USE_STLP_DEBUG)" != ""
LIBSTLPORT=$(DYNAMIC) -lstlport_gcc_stldebug
LIBSTLPORTST=$(STATIC) -lstlport_gcc_stldebug $(DYNAMIC)
.ELSE # "$(USE_STLP_DEBUG)" != ""
LIBSTLPORT=$(DYNAMIC) -lstlport_gcc
LIBSTLPORTST=$(STATIC) -lstlport_gcc $(DYNAMIC)
.ENDIF # "$(USE_STLP_DEBUG)" != ""

#FILLUPARC=$(STATIC) -lsupc++ $(DYNAMIC)

# name of library manager
LIBMGR=ar
LIBFLAGS=-r

# tool for generating import libraries
IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

# platform specific identifier for shared libs
DLLPOSTFIX=lm
DLLPRE=lib
DLLPOST=.so

