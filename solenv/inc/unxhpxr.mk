#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: unxhpxr.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: kz $ $Date: 2008-03-07 16:55:36 $
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

# mak file fuer unxhpxr
ASM=
AFLAGS=

CDEFS+=-D_PTHREADS -D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE -DSYSV -D_POSIX_PTHREAD_SEMANTICS  -D_HPUX_SOURCE -DRWSTD_MULTI_THREAD -D_REENTRANT -D__HPACC_NOEH -D__STL_NO_EXCEPTIONS

# kann c++ was c braucht??

# obernervige Warnungen abschalten
# 67  = invalid pragma name
# 251 = An object cannot be deleted using a pointer of type 'void *' since the 
#		type of the object allocated is unknown.
# 370 = The friend declaration is not in the correct form for either a function or a class.
#       (future error)	
# 600 = Type specifier is omitted; "int" is no longer assumed.
DISWARN=+W67,251,370,600

# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
ARCH_FLAGS*=

#CXX=/opt/aCC/bin/aCC
#CC=/opt/ansic/bin/cc
CXX=aCC
CC=cc
CFLAGS= -c
CFLAGSCC= -Aa +e +Z $(ARCH_FLAGS)
CFLAGSCXX=$(DISWARN) +Z $(ARCH_FLAGS)
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=
CFLAGSSLOCUIMT=
CFLAGSPROF=
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
CFLAGSOPT=-O
CFLAGSNOOPT=
CFLAGSOUTOBJ=-o


THREADLIB=-L/opt/dce/lib -ldce
# use native aCC to build or link with shared libs, for the sake of
# global class-instances
LINK=/opt/aCC/bin/aCC
LINKFLAGS=
LINKFLAGSAPPGUI=-Wl,+s
LINKFLAGSSHLGUI=-b -L/usr/solar/lib -L/usr/contrib/X11R6/lib 
LINKFLAGSAPPCUI=-Wl,+s
LINKFLAGSSHLCUI=-b -L/usr/contrib/X11R6/lib
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=
LINKFLAGSOPT=

# reihenfolge der libs NICHT egal!
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
#STDLIBGUIMT= -lX11 -lXext -lcma -lsec -lcxx -ldld -lc_r -lm
STDLIBGUIMT= -lX11 -lXext -lpthread
STDLIBCUIMT= -lcma
#STDSHLGUIMT= -lX11 -lXext -lcma -lsec -lcxx -ldld -lc_r
STDSHLCUIMT=
# -L/opt/dce/lib -ldce -lsec -lcxx

LIBMGR=ar
LIBFLAGS=-r
# LIBEXT=.so

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=
RCFLAGS=
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=hr
DLLPRE=lib
DLLPOST=.sl

LDUMP=c++filt

.IF "$(WORK_STAMP)"!="MIX364"
DLLPOSTFIX=ss
.ELSE
DLLPOSTFIX=
.ENDIF


.IF "$(PRJNAME)"=="uno" || "$(PRJNAME)"=="vos"
#CFLAGS += -D_PTHREADS  -U_REENTRANT -U_POSIX_PTHREAD_SEMANTICS
#CFLAGSSLO += -instances=extern -template=wholeclass -ptr$(SLO)
#CFLAGSOBJ += -instances=global -template=wholeclass -ptr$(SLO)
#LINKFLAGS+= -temp=/tmp -PIC -instances=extern -ptr$(SLO)
.ENDIF

#SHLLINKARCONLY=yes

