#*************************************************************************
#
#   $RCSfile: unxhpgr.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: hr $ $Date: 2003-03-27 11:48:16 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************


# mak file fuer unxhpxr
ASM=
AFLAGS=

CDEFS+=-D_PTHREADS -D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
CDEFS+=-DNO_AUDIO -DSYSV -D_POSIX_PTHREAD_SEMANTICS  -D_HPUX_SOURCE -DRWSTD_MULTI_THREAD -D_REENTRANT -D__HPACC_NOEH -D__STL_NO_EXCEPTIONS
 #CDEFS+= -D__hppa__

# obernervige Warnungen abschalten
# 67  = invalid pragma name
# 251 = An object cannot be deleted using a pointer of type 'void *' since the 
#		type of the object allocated is unknown.
# 370 = The friend declaration is not in the correct form for either a function or a class.
#       (future error)	
# 600 = Type specifier is omitted; "int" is no longer assumed.
DISWARN=+W67,251,370,600

#CXX=/opt/aCC/bin/aCC
#CC=/opt/ansic/bin/cc
CXX=g++
CC=gcc
CFLAGS= -Wall -w -nostdinc -c $(INCLUDE) -I/nw386/dev/s/solenv/unxhpgr/lib/gcc-lib/hppa1.1-hp-hpux10.20/egcs-2.91.57/include
CFLAGSCC=-pipe -fguiding-decls
CFLAGSCXX=-pipe -fguiding-decls
CFLAGSOBJGUIST=
CFLAGSOBJCUIST=
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=-fPIC
CFLAGSSLOCUIMT=-fPIC
CFLAGSPROF=
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
CFLAGSOPT=-O2
CFLAGSNOOPT=
CFLAGSOUTOBJ=-o


 #THREADLIB=-L/opt/dce/lib -ldce
# use native aCC to build or link with shared libs, for the sake of
# global class-instances

STATIC		= -Bstatic
DYNAMIC		= -Bdynamic

#LINK=$(SOLARENV)/unxhpgr/lib/gcc-lib/hppa1.1-hp-hpux10.20/egcs-2.90.29/ld
LINK=ld
LINKFLAGS=-L/nw386/dev/s/solenv/unxhpgr/lib/gcc-lib/hppa1.1-hp-hpux10.20/egcs-2.91.57/threads -L/nw386/dev/s/solenv/unxhpgr/lib/gcc-lib/hppa1.1-hp-hpux10.20/egcs-2.91.57
LINKFLAGSAPPGUI= /usr/lib/crt0.o
LINKFLAGSSHLGUI= -b -noinhibit-exec -Bsymbolic -G $(THREADLIB)
LINKFLAGSAPPCUI= /usr/lib/crt0.o
LINKFLAGSSHLCUI= -b -noinhibit-exec -warn-once -Bsymbolic -G
LINKFLAGSTACK=
APPLINKSTATIC=-Bstatic
APPLINKSHARED=-Bsymbolic
APP_LINKTYPE=

# reihenfolge der libs NICHT egal!
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
STDLIBGUIST= -lX11 -lXext -lsec -lcxx -ldld -lm
STDLIBCUIST= -lsec -lcxx -ldld -lm
#STDLIBGUIMT= -lX11 -lXext -lcma -lsec -lcxx -ldld -lc_r -lm
STDLIBGUIMT= -lX11 -lXext -ldce
STDLIBCUIMT= -lc_r -lgcc -ldce
#STDSHLGUIMT= -lX11 -lXext -lcma -lsec -lcxx -ldld -lc_r
STDSHLGUIMT= -lcma -lsec -ldld -lc_r
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

DLLPOSTFIX=hg
DLLPRE=lib
DLLPOST=.sl

LDUMP=c++filt

.IF "$(PRJNAME)"=="uno" || "$(PRJNAME)"=="vos"
#CFLAGS += -D_PTHREADS  -U_REENTRANT -U_POSIX_PTHREAD_SEMANTICS
#CFLAGSSLO += -instances=extern -template=wholeclass -ptr$(SLO)
#CFLAGSOBJ += -instances=global -template=wholeclass -ptr$(SLO)
#LINKFLAGS+= -temp=/tmp -PIC -instances=extern -ptr$(SLO)
.ENDIF

#SHLLINKARCONLY=yes

