#*************************************************************************
#
#   $RCSfile: unxsols4.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: hjs $ $Date: 2003-08-18 14:50:07 $
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


ASM=/usr/ccs/bin/as
AFLAGS=-P -q


CDEFS+=-mt -D_PTHREADS -DSYSV -DSUN -DSUN4 -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -DSTLPORT_VERSION=400

.IF "$(SOLAR_JAVA)"!=""
JAVADEF=-DSOLAR_JAVA
JAVAFLAGSDEBUG=-g
.IF "$(debug)"==""
JAVA_RUNTIME=-ljava
.ELSE
JAVA_RUNTIME=-ljava_g
.ENDIF
.ENDIF

CXX*=CC
CC*=cc

CFLAGS=$(PREENVCFLAGS) -c -temp=/tmp $(INCLUDE)
CFLAGSCC=-xCC
CFLAGSCXX=-features=no%altspell,no%localfor -library=no%Cstd

# flags to enable build with symbols; required for crashdump feature
CFLAGSENABLESYMBOLS=-g0 -xs

CFLAGSEXCEPTIONS=
CFLAGS_NO_EXCEPTIONS=-noex

CFLAGSOBJGUIST=-KPIC
CFLAGSOBJCUIST=-KPIC
CFLAGSOBJGUIMT=-KPIC
CFLAGSOBJCUIMT=-KPIC
CFLAGSSLOGUIMT=-KPIC
CFLAGSSLOCUIMT=-KPIC
CFLAGSPROF=-xpg
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
CFLAGSOPT=-xarch=v8 -xO3 -xspace
CFLAGSNOOPT=
CFLAGSOUTOBJ=-o

# Enable all warnings
CFLAGSWALL=+w2
# Set default warn level
CFLAGSDFLTWARN=

STDOBJVCL=$(L)$/salmain.o

THREADLIB=
.IF "$(PURIFY)"!=""
LINK=/usr/local/purify-4.2-solaris2/purify CC
.ELSE
LINK=CC
.ENDIF

# -z combreloc combines multiple relocation sections. Reduces overhead on startup
# -norunpath prevents the compiler from recording his own libs in the runpath
LINKFLAGSRUNPATH*=-R\''$$ORIGIN'\'
LINKFLAGS=-w -mt -z combreloc -PIC -temp=/tmp $(LINKFLAGSRUNPATH) -norunpath -library=no%Cstd

# -z text force fatal error if non PIC code is linked into shared library. Such code 
#    would be expensive on startup
CHECKFORPIC =-z text
LINKFLAGSSHLGUI=$(CHECKFORPIC) -G
LINKFLAGSSHLCUI=$(CHECKFORPIC) -G

# switches for dynamic and static linking
LINKFLAGSDEFS*= -z defs
STATIC		= -Bstatic
DIRECT		= -Bdirect $(LINKFLAGSDEFS)
DYNAMIC		= -Bdynamic

LINKFLAGSAPPGUI+=$(DIRECT)
LINKFLAGSAPPCUI+=$(DIRECT)
LINKFLAGSSHLGUI+=$(DIRECT)
LINKFLAGSSHLCUI+=$(DIRECT)

LINKFLAGSTACK=
LINKFLAGSPROF=-L$(COMPATH)/WS6U1/lib/libp -xpg -z allextract
LINKFLAGSDEBUG=
LINKFLAGSOPT=
LINKVERSIONMAPFLAG=-M

APPLINKSTATIC=$(STATIC)
APPLINKSHARED=$(DIRECT)
APP_LINKTYPE=

# reihenfolge der libs NICHT egal!
STDOBJGUI=
.IF "DBG_UTIL" != ""
STDSLOGUI=#-lpthread
.ELSE
STDSLOGUI=
.ENDIF
STDOBJCUI=
STDSLOCUI=

STDLIBGUIST=$(DYNAMIC) -lm
STDLIBCUIST=$(DYNAMIC) -lm
STDLIBGUIMT=$(DYNAMIC) -lpthread -lm
STDLIBCUIMT=$(DYNAMIC) -lpthread -lm
STDSHLGUIST=$(DYNAMIC) -lCrun -lm -lc
STDSHLCUIST=$(DYNAMIC) -lCrun -lm -lc
STDSHLGUIMT=$(DYNAMIC) -lpthread -lCrun -lm -lc
STDSHLCUIMT=$(DYNAMIC) -lpthread -lCrun -lm -lc

STDLIBGUIST+=-lX11
STDLIBGUIMT+=-lX11
STDSHLGUIST+=-lX11
STDSHLGUIMT+=-lX11

# @@@ interposer needed for -Bdirect @@@
# LIBSALCPPRT*=-z allextract -lsalcpprt -z defaultextract
LIBSALCPPRT=

LIBSTLPORT=$(DYNAMIC) -lstlport_sunpro
LIBSTLPORTST=$(STATIC) -lstlport_sunpro $(DYNAMIC)


LIBMGR=CC
LIBFLAGS=-xar -o 

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=ss

DLLPRE=lib
DLLPOST=.so

LDUMP=cppfilt /b /n /o /p

CFLAGSCXXSLO += -instances=static
CFLAGSCXXOBJ += -instances=static

LINKFLAGSAPPGUI+= -instances=static
LINKFLAGSSHLGUI+= -instances=static
LINKFLAGSAPPCUI+= -instances=static
LINKFLAGSSHLCUI+= -instances=static
