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



# mk file for unxbsdi2
ASM=
AFLAGS=

SOLAR_JAVA=TRUE
JAVAFLAGSDEBUG=-g

# filter for supressing verbose messages from linker
#not needed at the moment
#LINKOUTPUT_FILTER=" |& $(SOLARENV)/bin/msg_filter"

# _PTHREADS is needed for the stl
CDEFS+= -DX86 -D_PTHREADS -D_REENTRANT -DNEW_SOLAR -D_USE_NAMESPACE=1 -DSTLPORT_VERSION=$(STLPORT_VER)

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
# flags for C and C++ Compiler
CFLAGS+=-fmessage-length=0 -c

# flags to enable build with symbols; required for crashdump feature
.IF "$(ENABLE_SYMBOLS)"=="SMALL"
CFLAGSENABLESYMBOLS=-g1
.ELSE
CFLAGSENABLESYMBOLS=-g
.ENDIF

# flags for the C++ Compiler
CFLAGSCC= -pipe $(ARCH_FLAGS)
# Flags for enabling exception handling
CFLAGSEXCEPTIONS=-fexceptions -fno-enforce-eh-specs
# Flags for disabling exception handling
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

CFLAGSCXX= -pipe -frtti $(ARCH_FLAGS)
PICSWITCH:=-fpic

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
# CFLAGSOPT=-O2
# reduce to -O1 to avoid optimization problems
CFLAGSOPT=-O1
# Compiler flags for disabling optimizations
CFLAGSNOOPT=-O
# Compiler flags for describing the output path
CFLAGSOUTOBJ=-o

CFLAGSWARNCC=
CFLAGSWARNCXX=$(CFLAGSWARNCC) -Wno-ctor-dtor-privacy
# -Wshadow does not work for C with nested uses of pthread_cleanup_push:
CFLAGSWALLCC=-Wall -Wextra -Wendif-labels
CFLAGSWALLCXX=$(CFLAGSWALLCC) -Wshadow -Wno-ctor-dtor-privacy
CFLAGSWERRCC=-Werror

# switches for dynamic and static linking
STATIC		= -Wl,-Bstatic
DYNAMIC		= -Wl,-Bdynamic

# name of linker
LINK*=$(CC)

# default linker flags
LINKFLAGSDEFS*=-z defs
LINKFLAGSRUNPATH_URELIB=-Wl,-rpath,\''$$ORIGIN'\'
LINKFLAGSRUNPATH_UREBIN=-Wl,-rpath,\''$$ORIGIN/../lib:$$ORIGIN'\'
    #TODO: drop $ORIGIN once no URE executable is also shipped in OOo
LINKFLAGSRUNPATH_OOO=-Wl,-rpath,\''$$ORIGIN:$$ORIGIN/../ure-link/lib'\'
LINKFLAGSRUNPATH_SDK=-Wl,-rpath,\''$$ORIGIN/../../ure-link/lib'\'
LINKFLAGSRUNPATH_BRAND=-Wl,-rpath,\''$$ORIGIN:$$ORIGIN/../basis-link/program:$$ORIGIN/../basis-link/ure-link/lib'\'
LINKFLAGSRUNPATH_OXT=
LINKFLAGSRUNPATH_NONE=
LINKFLAGS=-z combreloc $(LINKFLAGSDEFS)

# linker flags for linking applications
LINKFLAGSAPPGUI= -Wl,-export-dynamic 
LINKFLAGSAPPCUI= -Wl,-export-dynamic 

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

# libraries for linking applications
STDLIBGUIMT=-lX11 -lpthread -lm
STDLIBCUIMT=-lpthread -lm
# libraries for linking shared libraries
STDSHLGUIMT=-lX11 -lXext -lpthread -lm
STDSHLCUIMT=-lpthread -lm

LIBSALCPPRT*=-Wl,--whole-archive -lsalcpprt -Wl,--no-whole-archive

.IF "$(STLPORT_VER)" >= "500"
LIBSTLPORT=$(DYNAMIC) -lstlport -lstdc++
LIBSTLPORTST=$(STATIC) -lstlport $(DYNAMIC)
.ELSE
LIBSTLPORT=$(DYNAMIC) -lstlport_gcc -lstdc++
LIBSTLPORTST=$(STATIC) -lstlport_gcc $(DYNAMIC)
.ENDIF

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
DLLPOSTFIX=
DLLPRE=lib
DLLPOST=.so

