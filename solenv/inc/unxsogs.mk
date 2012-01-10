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



# mk file for unxsogs
ASM=/usr/ccs/bin/as
AFLAGS=-P

SOLAR_JAVA=TRUE
JAVAFLAGSDEBUG=-g

# _PTHREADS is needed for the stl
CDEFS+=-D_PTHREADS -D_REENTRANT -DSYSV -DSUN -DSUN4 -D_POSIX_PTHREAD_SEMANTICS -DSTLPORT_VERSION=$(STLPORT_VER) -D_USE_NAMESPACE=1 


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

CXX*=g++
CC*=gcc
CFLAGS=-c
CFLAGSCC= -pipe $(ARCH_FLAGS)

CFLAGSEXCEPTIONS=-fexceptions
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

CFLAGSCXX= -pipe $(ARCH_FLAGS)
PICSWITCH:=-fPIC
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=$(PICSWITCH)
CFLAGSSLOCUIMT=$(PICSWITCH)
CFLAGSPROF=
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
CFLAGSOPT=-O2
CFLAGSNOOPT=
CFLAGSOUTOBJ=-o

STATIC		= -Wl,-Bstatic
DYNAMIC		= -Wl,-Bdynamic

LINK*=$(CXX)
LINKC*=$(CC)

LINKFLAGS=
LINKFLAGSAPPGUI=-Wl,-export-dynamic
LINKFLAGSSHLGUI=-shared
LINKFLAGSAPPCUI=-Wl,-export-dynamic
LINKFLAGSSHLCUI=-shared
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

LINKVERSIONMAPFLAG=-Wl,--version-script

# enable visibility define in "sal/types.h"
.IF "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"
CDEFS += -DHAVE_GCC_VISIBILITY_FEATURE
.ENDIF # "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"

# Reihenfolge der libs NICHT egal!

STDLIBCPP=-lstdc++

STDOBJVCL=$(L)/salmain.o
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

STDLIBGUIMT=$(DYNAMIC) -lpthread -lthread -lm
STDLIBCUIMT=$(DYNAMIC) -lpthread -lthread -lm
# libraries for linking shared libraries
STDSHLGUIMT=$(DYNAMIC) -lpthread -lthread -lm
STDSHLCUIMT=$(DYNAMIC) -lpthread -lthread -lm

STDLIBGUIMT+=-lX11

.IF "$(STLPORT_VER)" >= "500"
LIBSTLPORT=$(DYNAMIC) -lstlport
LIBSTLPORTST=$(STATIC) -lstlport $(DYNAMIC)
.ELSE
LIBSTLPORT=$(DYNAMIC) -lstlport_gcc
LIBSTLPORTST=$(STATIC) -lstlport_gcc $(DYNAMIC)
.ENDIF

LIBMGR=ar
LIBFLAGS=-r
# LIBEXT=.so

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=
DLLPRE=lib
DLLPOST=.so
