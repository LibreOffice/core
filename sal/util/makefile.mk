#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.1.1 $
#
#   last change: $Author: hr $ $Date: 2000-09-18 15:17:30 $
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

PRJ=..

$(PRJPCH)=

PRJNAME=sal
TARGET=sal
USE_LDUMP2=TRUE

.IF "$(GUI)"!="OS2"
USE_DEFFILE=TRUE
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :  ..$/version.mk

.IF "$(depend)" == ""
# --- Files --------------------------------------------------------

.IF "$(header)" == ""
LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=$(SLB)$/cpposl.lib $(SLB)$/oslall.lib $(SLB)$/cpprtl.lib
.IF "$(GUI)"=="WIN"
LIB1FILES+=$(SLB)$/shell9x.lib $(SLB)$/kernel9x.lib $(SLB)$/user9x.lib $(SLB)$/comdlg9x.lib $(SLB)$/tools32.lib
.ELSE
LIB1FILES+=$(SLB)$/textenc.lib
.ENDIF

LIB3TARGET=$(LB)$/a$(TARGET).lib
LIB3ARCHIV=$(LB)$/lib$(TARGET)$(SAL_MAJOR)$(DLLPOSTFIX).a
LIB3FILES=$(LB)$/cpposl.lib $(LB)$/oslall.lib $(LB)$/cpprtl.lib

.IF "$(UPDATER)"!=""
.IF "$(GUI)"!="WIN"
LIB3FILES+=$(LB)$/textenc.lib
.ENDIF
.ENDIF

SHL1TARGET= $(SAL_TARGET)$(SAL_MAJOR)
SHL1IMPLIB= i$(TARGET)

.IF "$(GUI)"=="WNT"
.IF "$(COM)"!="GCC"
SHL1STDLIBS=	\
                advapi32.lib\
                advapi9x.lib\
                kernel9x.lib\
                shell9x.lib\
                user9x.lib\
                comdlg9x.lib\
                wsock32.lib\
                mpr.lib\
                ole32.lib\
                shell32.lib\
                tools32.lib\
                comdlg32.lib\
                user32.lib
.ELSE
SHL1STDLIBS= -ladvapi32 -lwsock32 -lmpr -lole32
.ENDIF
.ENDIF

.IF "$(GUI)"=="WIN"
SHL1STDLIBS= winsock.lib
.ENDIF



.IF "$(GUI)"=="MAC"
SHL1STDLIBS=-L$(shell $(UNIX2MACPATH) $(MW_HOME)$/Metrowerks\ CodeWarrior$/MacOS\ Support$/OpenTransport$/Open\ Tpt\ Client\ Developer$/PPC\ Libraries) \
  -weakimport -lOpenTransportLib -weakimport -lOpenTptInternetLib \
  -lOpenTransportExtnPPC.o -lOpenTptInetPPC.o
SHL1STDLIBS+=-init InitLibrary -term ExitLibrary
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="SOLARIS"
SHL1STDLIBS= -lpthread -lposix4
.IF "$(COM)" == "C50"
SHL1STDLIBS+= -z allextract -staticlib=Crun -z defaultextract
.ENDIF
.ENDIF
.ENDIF

.IF "$(GUI)"=="OS2"
SHL1STDLIBS=n:\toolkit4\lib\so32dll.lib\
            n:\toolkit4\lib\tcp32dll.lib\
    os2286.lib

.ENDIF

SHL1LIBS=   $(SLB)$/$(TARGET).lib

.IF "$(linkinc)" != ""
SHL11FILE=$(MISC)$/sal.slo
.ELSE
.IF "$(GUI)"=="UNX"
SHL1OBJS=
.ELSE
.IF "$(GUI)"=="MAC"
SHL1OBJS= \
    $(OBJ)$/dllentry.obj
.ELSE
SHL1OBJS= \
    $(SLO)$/dllentry.obj
.ENDIF
.ENDIF
.ENDIF

SHL1DEPN=
SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
.IF "$(GUI)"=="WIN"
DEF1EXPORTFILE=	sal.w16
.ELSE
DEF1EXPORTFILE=	sal.dxp 
.ENDIF

.IF "$(GUI)"=="OS2"
DEF1EXPORT1=SignalHandlerFunction
.ENDIF

# --- tec ---

#SALLIB= isal.lib

#.IF "$(GUI)"!="WIN"
#LIB2TARGET= 	$(SLB)$/tec.lib
#LIB2ARCHIV= 	$(LB)$/libtec$(UPD)$(DLLPOSTFIX).a
#LIB2FILES=		$(SLB)$/textenc.lib

#.IF "$(UPDATER)"=="YES"
#LIB2TARGET= 	$(LB)$/atec.lib
#LIB2ARCHIV= 	$(LB)$/libtec$(UPD)$(DLLPOSTFIX).a
#LIB2FILES=		$(LB)$/textenc.lib
#.ENDIF

#SHL2TARGET= 	tec$(UPD)$(DLLPOSTFIX)
#SHL2IMPLIB= 	itec

#SHL2STDLIBS=	$(SALLIB)

#SHL2LIBS=		$(SLB)$/textenc.lib
#.IF "$(linkinc)" != ""
#SHL21FILE=		$(MISC)$/tec.slo
#.ENDIF

#SHL2DEPN=
#SHL2DEF=		$(MISC)$/$(SHL2TARGET).def

#DEF2NAME=		$(SHL2TARGET)
#DEF2EXPORTFILE= tec.dxp
#.ENDIF


# --- Targets ------------------------------------------------------

.ENDIF

.ENDIF			# $(depend)!=""

.INCLUDE :  target.mk

