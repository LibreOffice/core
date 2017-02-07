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



# generic mk file for unxlng (unix linux glibc)
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

# filter for suppressing verbose messages from linker
#not needed at the moment
#LINKOUTPUT_FILTER=" |& $(SOLARENV)/bin/msg_filter"

# _PTHREADS is needed for the stl
CDEFS+=-DGLIBC=2 -D_PTHREADS -D_REENTRANT -DNEW_SOLAR -D_USE_NAMESPACE=1
CDEFS+=-DBOOST_DETAIL_NO_CONTAINER_FWD

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

# name of C++ Compiler
CXX*=g++
# name of C Compiler
CC*=gcc
.IF "$(SYSBASE)"!=""
CFLAGS_SYSBASE:=--sysroot=$(SYSBASE)
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
.IF "$(COM)"=="CLANG"
CFLAGSEXCEPTIONS=-fexceptions
.ELSE
CFLAGSEXCEPTIONS=-fexceptions -fno-enforce-eh-specs
.ENDIF
# Flags for disabling exception handling
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

# -fpermissive should be removed as soon as possible
CFLAGSCXX= -pipe $(ARCH_FLAGS)
.IF "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"
CFLAGSCXX += -fvisibility-inlines-hidden
.ENDIF # "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"

CFLAGS_CREATE_PCH=-x c++-header -I$(INCPCH) -DPRECOMPILED_HEADERS
CFLAGS_USE_PCH=-I$(SLO)$/pch -DPRECOMPILED_HEADERS -Winvalid-pch
CFLAGS_USE_EXCEPTIONS_PCH=-I$(SLO)$/pch_ex -DPRECOMPILED_HEADERS -Winvalid-pch

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
CFLAGSOPT=$(CDEFAULTOPT) -fno-strict-aliasing		# optimizing for products
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
MODULES_WITH_WARNINGS := 

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
LINKFLAGSDEFS*=-Wl,-z,defs
LINKFLAGSRUNPATH_URELIB=-Wl,-rpath,\''$$ORIGIN'\'
LINKFLAGSRUNPATH_UREBIN=-Wl,-rpath,\''$$ORIGIN'\'
#LINKFLAGSRUNPATH_UREBIN=-Wl,-rpath,\''$$ORIGIN/../lib:$$ORIGIN'\'
    #TODO: drop $ORIGIN once no URE executable is also shipped in OOo
LINKFLAGSRUNPATH_OOO=-Wl,-rpath,\''$$ORIGIN'\'
LINKFLAGSRUNPATH_SDK=-Wl,-rpath,\''$$ORIGIN'\'
LINKFLAGSRUNPATH_BRAND=-Wl,-rpath,\''$$ORIGIN'\'
#LINKFLAGSRUNPATH_OOO=-Wl,-rpath,\''$$ORIGIN:$$ORIGIN/../ure-link/lib'\'
#LINKFLAGSRUNPATH_SDK=-Wl,-rpath,\''$$ORIGIN/../../ure-link/lib'\'
#LINKFLAGSRUNPATH_BRAND=-Wl,-rpath,\''$$ORIGIN:$$ORIGIN/../basis-link/program:$$ORIGIN/../basis-link/ure-lin
LINKFLAGSRUNPATH_OXT=
LINKFLAGSRUNPATH_BOXT=-Wl,-rpath,\''$$ORIGIN'\'
#LINKFLAGSRUNPATH_BOXT=-Wl,-rpath,\''$$ORIGIN/../../../basis-link/program'\'
LINKFLAGSRUNPATH_NONE=
LINKFLAGS=-Wl,-z,combreloc $(LINKFLAGSDEFS) $(LINKFLAGS_SYSBASE)
.IF "$(HAVE_LD_BSYMBOLIC_FUNCTIONS)"  == "TRUE"
LINKFLAGS += -Wl,-Bsymbolic-functions -Wl,--dynamic-list-cpp-new -Wl,--dynamic-list-cpp-typeinfo
.ENDIF

# linker flags for linking applications
LINKFLAGSAPPGUI= -Wl,-export-dynamic -Wl,--noinhibit-exec \
    -Wl,-rpath-link,$(LB):$(SOLARLIBDIR):$(SYSBASE)/lib:$(SYSBASE)/usr/lib
LINKFLAGSAPPCUI= -Wl,-export-dynamic -Wl,--noinhibit-exec \
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
STDOBJVCL=$(L)$/salmain.o
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
LINKFLAGS += -Wl,--hash-style=both
.ELSE
LINKFLAGS += -Wl,-zdynsort
.ENDIF

# libraries for linking applications
STDLIBGUIMT+=-Wl,--as-needed -ldl -lpthread -lm -Wl,--no-as-needed
STDLIBCUIMT+=-Wl,--as-needed -ldl -lpthread -lm -Wl,--no-as-needed
# libraries for linking shared libraries
STDSHLGUIMT+=-Wl,--as-needed -ldl -lpthread -lm -Wl,--no-as-needed
STDSHLCUIMT+=-Wl,--as-needed -ldl -lpthread -lm -Wl,--no-as-needed

X11LINK_DYNAMIC = -Wl,--as-needed -lXext -lX11 -Wl,--no-as-needed

LIBSALCPPRT*=-Wl,--whole-archive -lsalcpprt -Wl,--no-whole-archive

#FILLUPARC=$(STATIC) -lsupc++ $(DYNAMIC)

# name of library manager
LIBMGR*=ar
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
PCHPOST=.gch
