#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

# generic mk file for gcc on unix systems

ASM*=
AFLAGS*=
SOLAR_JAVA*=
# default optimization level for product code
CDEFAULTOPT*=-O2
# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
ARCH_FLAGS*=
# position independent code switch
PICSWITCH*:=-fpic
JAVAFLAGSDEBUG=-g

# filter for supressing verbose messages from linker
#not needed at the moment
#LINKOUTPUT_FILTER=" |& $(SOLARENV)/bin/msg_filter"

# _PTHREADS is needed for the stl
CDEFS+=-D_PTHREADS -D_REENTRANT -DNEW_SOLAR -D_USE_NAMESPACE=1

# enable visibility define in "sal/types.h"
.IF "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"
CDEFS += -DHAVE_GCC_VISIBILITY_FEATURE
.ENDIF # "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"

.IF "$(HAVE_SFINAE_ANONYMOUS_BROKEN)" == "TRUE"
CDEFS += -DHAVE_SFINAE_ANONYMOUS_BROKEN
.ENDIF # "$(HAVE_SFINAE_ANONYMOUS_BROKEN)" == "TRUE"

# this is a platform with JAVA support
.IF "$(SOLAR_JAVA)"!=""
JAVADEF=-DSOLAR_JAVA
.IF "$(debug)"==""
JAVA_RUNTIME=-ljava
.ELSE
JAVA_RUNTIME=-ljava_g
.ENDIF
.ENDIF

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

# flags to enable build with symbols
CFLAGSENABLESYMBOLS=-g

# flags for the C++ Compiler
CFLAGSCC= -pipe $(ARCH_FLAGS)
# Flags for enabling exception handling
CFLAGSEXCEPTIONS=-fexceptions
.IF "$(dbgutil)"==""
.IF "$(COM_GCC_IS_CLANG)" != "TRUE"
CFLAGSEXCEPTIONS+=-fno-enforce-eh-specs
.ENDIF
.ENDIF
# Flags for disabling exception handling
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

# -fpermissive should be removed as soon as possible
CFLAGSCXX= -pipe $(ARCH_FLAGS)
.IF "$(HAVE_THREADSAFE_STATICS)" != "TRUE"
CFLAGSCXX += -fno-threadsafe-statics
.END
.IF "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE" && "$(HAVE_GCC_VISIBILITY_BROKEN)" != "TRUE"
CFLAGSCXX+=-fvisibility-inlines-hidden
.ENDIF # "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"
.IF "$(HAVE_CXX0X)" == "TRUE"
CFLAGSCXX+=-std=gnu++0x
.IF "$(GCCNUMVER)" <= "000400059999"
CFLAGSCXX+=-Wno-deprecated-declarations
.ENDIF
.ENDIF # "$(HAVE_CXX0X)" == "TRUE"

.IF "$(PRODUCT)"!="full"
# enable debug STL
CFLAGSCXX += -D_GLIBCXX_DEBUG
.ENDIF # !PRODUCT

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

GCCNUMVERSION_CMD=-dumpversion $(PIPEERROR) $(AWK) -v num=true -f $(SOLARENV)/bin/getcompver.awk
GCCNUMVER:=$(shell @-$(CXX) $(GCCNUMVERSION_CMD))

# Compiler flags for enabling optimizations
.IF "$(PRODUCT)"!=""
CFLAGSOPT=$(CDEFAULTOPT) # optimizing for products
.IF "$(GCCNUMVER)" <= "000400059999"
#At least SLED 10.2 gcc 4.3 overly agressively optimizes uno::Sequence into
#junk, so only strict-alias on >= 4.6.0
CFLAGSOPT+=-fno-strict-aliasing
.ENDIF
.ELSE 	# "$(PRODUCT)"!=""
CFLAGSOPT=   							# no optimizing for non products
.ENDIF	# "$(PRODUCT)"!=""

# Compiler flags for disabling optimizations
CFLAGSNOOPT=-O0
# Compiler flags for describing the output path
CFLAGSOUTOBJ=-o

# -Wshadow does not work for C with nested uses of pthread_cleanup_push:
CFLAGSWARNBOTH=-Wall -Wextra -Wendif-labels
CFLAGSWARNCC=$(CFLAGSWARNBOTH) -Wdeclaration-after-statement
CFLAGSWARNCXX=$(CFLAGSWARNBOTH) -Wshadow -Wno-ctor-dtor-privacy
CFLAGSWALLCC=$(CFLAGSWARNCC)
CFLAGSWALLCXX=$(CFLAGSWARNCXX)
CFLAGSWERRCC=-Werror -DLIBO_WERROR

.IF "$(COM_GCC_IS_CLANG)" != "TRUE"
# Only GCC 4.6 has a fix for <http://gcc.gnu.org/bugzilla/show_bug.cgi?id=7302>
# "-Wnon-virtual-dtor should't complain of protected dtor" and supports #pragma
# GCC diagnostic push/pop required e.g. in cppuhelper/propertysetmixin.hxx to
# silence warnings about a protected, non-virtual dtor in a class with virtual
# members and friends:
.IF "$(GCCNUMVER)" <= "000400059999"
CFLAGSWARNCXX += -Wno-non-virtual-dtor
.ELSE
CFLAGSWARNCXX += -Wnon-virtual-dtor
.END
.ELSE
CFLAGSWARNCXX += -Wnon-virtual-dtor
.END

COMPILER_WARN_ERRORS=TRUE

# switches for dynamic and static linking
STATIC		= -Wl,-Bstatic
DYNAMIC		= -Wl,-Bdynamic

# name of linker
LINK*=$(CXX)
LINKC*=$(CC)

# default linker flags
.IF "$(SYSBASE)"!=""
LINKFLAGS_SYSBASE:=-Wl,--sysroot=$(SYSBASE)
.ENDIF          # "$(SYSBASE)"!=""
#
# The DT RPATH value is used first, before any other path, specifically before
# the path defined in the LD_LIBRARY_PATH environment variable. This is
# problematic since it does not allow the user to overwrite the value.
# Therefore DT_RPATH is deprecated. The introduction of the new variant,
# DT_RUNPATH, corrects this oversight by requiring the value is used after the
# path in LD_LIBRARY_PATH.
#
# The linker option --enable-new-dtags must be used to also add DT_RUNPATH
# entry. This will cause both, DT_RPATH and DT_RUNPATH entries, to be created
#
LINKFLAGSDEFS*=-Wl,-z,defs
LINKFLAGSRUNPATH_URELIB=-Wl,-z,origin -Wl,-rpath,\''$$ORIGIN'\',--enable-new-dtags
LINKFLAGSRUNPATH_UREBIN=-Wl,-z,origin -Wl,-rpath,\''$$ORIGIN/../lib:$$ORIGIN'\',--enable-new-dtags
    #TODO: drop $ORIGIN once no URE executable is also shipped in OOo
LINKFLAGSRUNPATH_OOO=-Wl,-z,origin -Wl,-rpath,\''$$ORIGIN:$$ORIGIN/../ure-link/lib'\',--enable-new-dtags
LINKFLAGSRUNPATH_SDKBIN=-Wl,-z,origin -Wl,-rpath,\''$$ORIGIN/../../ure-link/lib'\',--enable-new-dtags
LINKFLAGSRUNPATH_OXT=
LINKFLAGSRUNPATH_NONE=
# flag -Wl,-z,noexecstack sets the NX bit on the stack
LINKFLAGS=-Wl,-z,noexecstack -Wl,-z,combreloc $(LINKFLAGSDEFS) $(LINKFLAGS_SYSBASE)
.IF "$(HAVE_LD_BSYMBOLIC_FUNCTIONS)"  == "TRUE"
LINKFLAGS += -Wl,-Bsymbolic-functions -Wl,--dynamic-list-cpp-new -Wl,--dynamic-list-cpp-typeinfo
.ENDIF

# linker flags for linking applications
LINKFLAGSAPPGUI= -Wl,-export-dynamic \
    -Wl,-rpath-link,$(LB):$(SOLARLIBDIR):$(SYSBASE)/lib:$(SYSBASE)/usr/lib
LINKFLAGSAPPCUI= -Wl,-export-dynamic \
    -Wl,-rpath-link,$(LB):$(SOLARLIBDIR):$(SYSBASE)/lib:$(SYSBASE)/usr/lib

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
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

.IF "$(ALLOC)" == "TCMALLOC"
STDLIBGUIMT+=-ltcmalloc
STDLIBCUIMT+=-ltcmalloc
STDSHLGUIMT+=-ltcmalloc
STDSHLCUIMT+=-ltcmalloc
.ENDIF

.IF "$(ALLOC)" == "JEMALLOC"
STDLIBGUIMT+=-ljemalloc
STDLIBCUIMT+=-ljemalloc
STDSHLGUIMT+=-ljemalloc
STDSHLCUIMT+=-ljemalloc
.ENDIF

.IF "$(HAVE_LD_HASH_STYLE)"  == "TRUE"
LINKFLAGS += -Wl,--hash-style=$(WITH_LINKER_HASH_STYLE)
.ENDIF

# libraries for linking applications
STDLIBGUIMT+=-Wl,--as-needed $(DL_LIB) $(PTHREAD_LIBS) -lm -Wl,--no-as-needed
STDLIBCUIMT+=-Wl,--as-needed $(DL_LIB) $(PTHREAD_LIBS) -lm -Wl,--no-as-needed
# libraries for linking shared libraries
STDSHLGUIMT+=-Wl,--as-needed $(DL_LIB) $(PTHREAD_LIBS) -lm -Wl,--no-as-needed
STDSHLCUIMT+=-Wl,--as-needed $(DL_LIB) $(PTHREAD_LIBS) -lm -Wl,--no-as-needed

X11LINK_DYNAMIC = -Wl,--as-needed -lXext -lX11 -Wl,--no-as-needed

LIBSALCPPRT*=-Wl,--whole-archive -lsalcpprt -Wl,--no-whole-archive

# name of library manager
LIBMGR*=$(AR)
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
DLLPRE=lib
DLLPOST=.so
