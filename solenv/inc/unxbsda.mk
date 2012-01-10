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



# mak file fuer unxbsda

ASM=
AFLAGS=

ARCH_FLAGS*=

CC=				gcc -c
CXX=			g++ -c
CDEFS+=			-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
CDEFS+=			-D_REENTRANT 		# -D_PTHREADS -DPOSIX_PTHREAD_SEMANTICS
CDEFS+= 		-DPRINTER_DUMMY
CFLAGSCC=		-fsigned-char $(ARCH_FLAGS)
CFLAGSCXX=		-fsigned-char $(ARCH_FLAGS)
PICSWITCH:=-fPIC
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=$(PICSWITCH)
CFLAGSSLOCUIMT=$(PICSWITCH)
CFLAGSPROF=
CFLAGSDEBUG=	-g
CFLAGSDBGUTIL=
CFLAGSOPT=		-O2
CFLAGSNOOPT=
CFLAGSOUTOBJ=	-o

STATIC=			-Bstatic
DYNAMIC=		-Bdynamic

THREADLIB=		pthread
LINK=			ld -e start -dc -dp
LINKFLAGS=
LINKFLAGSAPPGUI=
LINKFLAGSSHLGUI=
LINKFLAGSAPPCUI=
LINKFLAGSSHLCUI=
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=
LINKFLAGSOPT=

STDOBJGUI=		/usr/lib/crt0.o
STDSLOGUI=		/usr/lib/crt0.o
STDOBJCUI=		/usr/lib/crt0.o
STDSLOCUI=		/usr/lib/crt0.o
STDLIBGUIMT=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc
STDLIBCUIMT=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc
STDSHLGUIMT=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc
STDSHLCUIMT=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc

LIBMGR=			ar
LIBFLAGS=		-r
LIBEXT=			.a

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=		-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=
DLLPRE=			lib
DLLPOST=		.so

LDUMP=

