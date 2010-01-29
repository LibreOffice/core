#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: unxlngx6.mk,v $
#
# $Revision: 1.19.12.1 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

# mk file for unxlngx6
ASM=
AFLAGS=

SOLAR_JAVA*=
JAVAFLAGSDEBUG=-g

# filter for supressing verbose messages from linker
#not needed at the moment
#LINKOUTPUT_FILTER=" |& $(SOLARENV)$/bin$/msg_filter"

# _PTHREADS is needed for the stl
CDEFS+=$(PTHREAD_CFLAGS) -DGLIBC=2 -DX86_64 -D_PTHREADS -D_REENTRANT -DNEW_SOLAR -D_USE_NAMESPACE=1 -DSTLPORT_VERSION=$(STLPORT_VER)

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
ARCH_FLAGS*=

# name of C++ Compiler
CXX*=g++
# name of C Compiler
CC*=gcc
.IF "$(SYSBASE)"!=""
CFLAGS_SYSBASE:=-isystem $(SYSBASE)$/usr$/include
CXX+:=$(CFLAGS_SYSBASE)
CC+:=$(CFLAGS_SYSBASE)
.ENDIF          # "$(SYSBASE)"!=""
CFLAGS+=-Wreturn-type -fmessage-length=0 -c
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
CFLAGSCXX+= -Wno-ctor-dtor-privacy
CFLAGSCXX+= -fno-use-cxa-atexit
PICSWITCH:=-fpic
.IF "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"
CFLAGSCXX += -fvisibility-inlines-hidden
.ENDIF # "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"

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
CFLAGSOPT=-O2 -fno-strict-aliasing		# optimizing for products
CFLAGSOPT+=-Wuninitialized				# not supported without optimization
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
    soldep \
    svx

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
LINKFLAGSRUNPATH_SDK=-Wl,-rpath,\''$$ORIGIN/../../ure-link/lib'\'
LINKFLAGSRUNPATH_BRAND=-Wl,-rpath,\''$$ORIGIN:$$ORIGIN/../basis-link/program:$$ORIGIN/../basis-link/ure-link/lib'\'
LINKFLAGSRUNPATH_OXT=
LINKFLAGSRUNPATH_NONE=
LINKFLAGS=-Wl,-z,combreloc $(LINKFLAGSDEFS)
.IF "$(HAVE_LD_BSYMBOLIC_FUNCTIONS)"  == "TRUE"
LINKFLAGS += -Wl,-Bsymbolic-functions -Wl,--dynamic-list-cpp-new -Wl,--dynamic-list-cpp-typeinfo
.ENDIF

.IF "$(HAVE_LD_HASH_STYLE)"  == "TRUE"
LINKFLAGS += -Wl,--hash-style=both
.ELSE
LINKFLAGS += -Wl,-zdynsort
.ENDIF

# linker flags for linking applications
LINKFLAGSAPPGUI= -Wl,-export-dynamic -Wl,--noinhibit-exec \
    -Wl,-rpath-link,$(LB):$(SOLARLIBDIR)
LINKFLAGSAPPCUI= -Wl,-export-dynamic -Wl,--noinhibit-exec \
    -Wl,-rpath-link,$(LB):$(SOLARLIBDIR)

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

# libraries for linking applications
STDLIBGUIMT+=-Wl,--as-needed -lX11 -ldl -lpthread -lm -Wl,--no-as-needed
STDLIBCUIMT+=-Wl,--as-needed -ldl -lpthread -lm -Wl,--no-as-needed
# libraries for linking shared libraries
STDSHLGUIMT+=-Wl,--as-needed -lX11 -lXext -ldl -lpthread -lm -Wl,--no-as-needed
STDSHLCUIMT+=-Wl,--as-needed -ldl -lpthread -lm -Wl,--no-as-needed

LIBSALCPPRT*=-Wl,--whole-archive -lsalcpprt -Wl,--no-whole-archive

.IF "$(USE_STLP_DEBUG)" != ""
.IF "$(STLPORT_VER)" >= "500"
LIBSTLPORT=$(DYNAMIC) -lstlportstlg
LIBSTLPORTST=$(STATIC) -lstlportstlg $(DYNAMIC)
.ELSE
LIBSTLPORT=$(DYNAMIC) -lstlport_gcc_stldebug
LIBSTLPORTST=$(STATIC) -lstlport_gcc_stldebug $(DYNAMIC)
.ENDIF
.ELSE # "$(USE_STLP_DEBUG)" != ""
.IF "$(STLPORT_VER)" >= "500"
LIBSTLPORT=$(DYNAMIC) -lstlport
LIBSTLPORTST=$(STATIC) -lstlport $(DYNAMIC)
.ELSE
LIBSTLPORT=$(DYNAMIC) -lstlport_gcc
LIBSTLPORTST=$(STATIC) -lstlport_gcc $(DYNAMIC)
.ENDIF
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
DLLPOSTFIX=lx
DLLPRE=lib
DLLPOST=.so
PCHPOST=.gch

