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




# mak file fuer wnticci
ASM=
AFLAGS=

CDEFS+=-D_PTHREADS -D_REENTRANT 
CDEFS+=-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE -DX86 -DNEW_SOLAR
# kann c++ was c braucht??

# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
ARCH_FLAGS*=-mpentium

CXX*=/nw386/dev/s/solenv/unxscoi/bin/g++
CC*=/nw386/dev/s/solenv/unxscoi/bin/gcc
CFLAGS=-c
CFLAGSCC=$(ARCH_FLAGS)
CFLAGSEXCEPTIONS=-fexceptions
CFLAGS_NO_EXCEPTIONS=-fno-exceptions
CFLAGSCXX=-fguiding-decls -frtti $(ARCH_FLAGS)
PICSWITCH:=-fPIC
CFLAGSOBJGUIMT=$(PICSWITCH)
CFLAGSOBJCUIMT=$(PICSWITCH)
CFLAGSSLOGUIMT=$(PICSWITCH)
CFLAGSSLOCUIMT=$(PICSWITCH)
CFLAGSPROF=
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
CFLAGSOPT=-O2
CFLAGSNOOPT=-O
CFLAGSOUTOBJ=-o

CFLAGSWARNCC=
CFLAGSWARNCXX=$(CFLAGSWARNCC) -Wno-ctor-dtor-privacy
# -Wshadow does not work for C with nested uses of pthread_cleanup_push:
CFLAGSWALLCC=-Wall -Wextra -Wendif-labels
CFLAGSWALLCXX=$(CFLAGSWALLCC) -Wshadow -Wno-ctor-dtor-privacy
CFLAGSWERRCC=-Werror

STATIC		= -Wl,-Bstatic
DYNAMIC		= -Wl,-Bdynamic

THREADLIB=
LINK=/nw386/dev/s/solenv/unxscoi/bin/gcc
LINKFLAGS=
# SCO hat grosse Probleme mit fork/exec und einigen shared libraries
# rsc2 muss daher statisch gelinkt werden
.IF "$(PRJNAME)"=="rsc"
LINKFLAGSAPPGUI=-L/nw386/dev/s/solenv/unxscoi/lib $(STATIC) -lpthread_init $(DYNAMIC)
LINKFLAGSAPPCUI=-L/nw386/dev/s/solenv/unxscoi/lib $(STATIC) -lpthread_init $(DYNAMIC)
.ELSE
LINKFLAGSAPPGUI=-L/nw386/dev/s/solenv/unxscoi/lib -lpthread_init
LINKFLAGSAPPCUI=-L/nw386/dev/s/solenv/unxscoi/lib -lpthread_init
.ENDIF
LINKFLAGSSHLGUI=-G -W,l,-Bsymbolic
LINKFLAGSSHLCUI=-G -W,l,-Bsymbolic
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

# standard C++ Library
#
# das statische dazulinken der libstdc++ macht jede shared library um 50k
# (ungestrippt) oder so groesser, auch wenn sie ueberhaupt nicht gebraucht
# wird. Da muessen wir uns was besseres ueberlegen.
STDLIBCPP=-Wl,-Bstatic -lstdc++ -Wl,-Bdynamic

# reihenfolge der libs NICHT egal!
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
.IF "$(PRJNAME)"=="rsc"
STDLIBGUIMT=-lXext -lX11 $(STATIC) -lpthread $(DYNAMIC) -ldl -lsocket -lm
STDLIBCUIMT=$(STATIC) -lpthread $(DYNAMIC) -ldl -lsocket -lm 
.ELSE
STDLIBGUIMT=-lXext -lX11 -lpthread -ldl -lsocket -lm
STDLIBCUIMT=-lpthread -ldl -lsocket -lm 
.ENDIF
#STDSHLGUIMT=-lXext -lX11 -lpthread -ldl -lsocket -lm
#STDSHLCUIMT=-lpthread -ldl -lsocket -lm 

STDLIBCPP= -lstdc++
SHLLINKARCONLY=yes

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

LDUMP=cppfilt /b /n /o /p

