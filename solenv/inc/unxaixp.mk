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



# mak file fuer unxaixp
ASM=
AFLAGS=

ARCH_FLAGS*=

CXX=				xlC_r
CC=				xlc_r
CFLAGS=			-c -qlanglvl=extended -qchars=signed 
CDEFS+=			-D_PTHREADS
CDEFS+=			-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
CFLAGSCC=$(ARCH_FLAGS)
CFLAGSCXX=$(ARCH_FLAGS)
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=
CFLAGSSLOCUIMT=
CFLAGSPROF=
CFLAGSDEBUG=
CFLAGSDBGUTIL=
CFLAGSOPT=		-O
CFLAGSNOOPT=
CFLAGSOUTOBJ=	-o

STATIC= 		# -Bstatic
DYNAMIC= 		# -Bdynamic

#
# Zu den Linkflags von shared Libraries:
#
# -G			Erzeugen eines Shared-Objects
# -brtl			aktiviert Runtime-Linking, wird von -G mitgesetzt
# -bsymbolc		loest Symbole innerhalb einer Library zur Linkzeit auf
# -p0			??? (ENOMANPAGESINSTALLABLEFROMCD)
# -bnoquiet		verbose
#
LINK=
LINKFLAGS=
LINKFLAGSAPPCUI=	xlC_r -qlanglvl=extended -qchars=signed -brtl -bnolibpath
LINKFLAGSAPPGUI=	xlC_r -qlanglvl=extended -qchars=signed -brtl -bnolibpath
LINKFLAGSSHLCUI=	makeC++SharedLib_r -G -bsymbolic -bdynamic -bnolibpath -p0
LINKFLAGSSHLGUI=	makeC++SharedLib_r -G -bsymbolic -bdynamic -bnolibpath -p0
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=
LINKFLAGSOPT=

# Reihenfolge der libs NICHT egal!
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
# application
STDLIBGUIMT=	# -lX11 -ldl
STDLIBCUIMT=	# -lX11 -ldl
# shared library
STDSHLGUIMT=	-lX11 -ldl
STDSHLCUIMT=	-lX11 -ldl
THREADLIB=

LIBMGR=			ar
LIBFLAGS=		-r
# LIBEXT=		.so

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
.IF "$(WORK_STAMP)"=="MIX364"
DLLPOSTFIX=
.ENDIF
DLLPRE=			lib
DLLPOST=		.so


LDUMP=			cppfilt /b /n /o /p

