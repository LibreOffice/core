#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: fs $ $Date: 2000-11-02 12:03:49 $
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

PRJNAME=basic
TARGET=sb
#basic.hid generieren
GEN_HID=TRUE

# --- Settings ---------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

.IF "$(GUI)"=="WIN"
.IF "$(product)" != ""
LINKFLAGS=$(LINKFLAGS) /NOPACKC
.ENDIF
.ENDIF

.IF "$(depend)" == ""

# --- Allgemein ---------------------------------------------------

USE_LDUMP2=TRUE

.IF "$(header)" == ""

#SRSFILES=	$(SRS)$/app.srs $(SRS)$/testtool.srs


LIB1TARGET=$(SLB)$/sb.lib
LIB1FILES=		\
    $(SLB)$/basicmgr.lib   \
    $(SLB)$/classes.lib	 \
    $(SLB)$/comp.lib 	 \
    $(SLB)$/runtime.lib

SHL1TARGET= sb$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB= basic

SHL1STDLIBS= \
            $(CPPULIB) \
            $(CPPUHELPERLIB) \
            $(TOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB)	\
            $(SVLIB) \
            $(SJLIB) \
            $(VOSLIB) \
            $(SALLIB) \
            $(SVMEMLIB)	\
            $(COMPHELPERLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=$(LIBCIMT)
.ENDIF

.IF "$(SO3)" != ""
SHL1STDLIBS+=\
            $(SOTLIB) \
            $(VOSLIB)
.ENDIF



.IF "$(GUI)" != "UNX"
SHL1OBJS=	\
    $(SLO)$/sb.obj
.ENDIF

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=	$(SLB)$/sb.lib

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=	\
    $(MISC)$/$(SHL1TARGET).flt

DEFLIB1NAME	=sb
DEF1DES		=StarBasic

# --- SBASIC IDE --------------------------------------------------------

APP1TARGET=$(PRJNAME)
.IF "$(GUI)" != "MAC"
APP1STDLIBS= \
            $(SALLIB) \
            $(TOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB) \
            $(SVLIB) \
            $(SO2LIB) \
            $(COMPHELPERLIB) \
            $(UCBHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(SJLIB) \
            $(VOSLIB) \
            $(SVMEMLIB)
.IF "$(GUI)"=="WNT" || "$(COM)"=="GCC"
APP1STDLIBS+=$(CPPULIB)
.ENDIF
.ELSE
APP1STDLIBS= \
            $(SOLARLIBDIR)$/SALMAIN.OBJ \
            $(SOLARLIBDIR)$/NOSHAREDMAIN.LIB \
            $(TOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLIB) \
            $(SJLIB) \
            $(SO2LIB)
.ENDIF
.IF "$(GUI)"=="UNX"
APP1STDLIBS+= \
            $(VOSLIB) \
            $(SALLIB)
.ENDIF

APP1LIBS= \
            $(LIBPRE) $(LB)$/basic.lib \
            $(LIBPRE) $(LB)$/app.lib \
            $(LIBPRE) $(LB)$/sample.lib
.IF "$(GUI)"=="UNX"
APP1STDLIBS+=	\
            $(BASICLIB)
.ENDIF


.IF "$(SO3)" != ""
APP1STDLIBS+=\
            $(SOTLIB)
.ENDIF

.IF "$(GUI)" != "MAC"
APP1DEPN=	$(L)$/itools.lib $(SVLIBDEPEND) $(LB)$/basic.lib $(LB)$/app.lib $(LB)$/sample.lib
.ELSE
APP1DEPN = $(APP1STDLIBS) $(APP1LIBS)
MACRES = $(SV_RES)SV.R $(SV_RES)SV_DEMO.R $(SV_RES)SV_POWER.R
.ENDIF

APP1OBJS = $(OBJ)$/ttbasic.obj 

.IF "$(GUI)" != "UNX"
APP1OBJS+=	\
            $(OBJ)$/app.obj

.IF "$(COM)"!="ICC"
APP1OBJS+=	\
            $(SLO)$/sbintern.obj
.ENDIF
.ENDIF

#APP1DEF=	$(MISC)$/$(PRJNAME).def
#APP1RES=	$(RES)$/sb.res

RES1TARGET=$(PRJNAME)
SRS1FILES= \
        $(SRS)$/classes.srs

# --- TESTTOOL IDE ------------------------------------------------------
# die ressourcen werden hier gelinkt

RESLIB1NAME=stt
RESLIB1SRSFILES= \
        $(SRS)$/app.srs \
        $(SRS)$/classes.srs

# --- jstest ------------------------------------------------------

#.IF "$(GUI)" != "MAC"

#APP4TARGET=jstest
#APP4STDLIBS= \
#			$(TOOLSLIB) \
#			$(SVTOOLLIB) \
#			$(SVLIB) \
#			$(SVMEMLIB) \
#			$(SO2LIB)	\
#			$(SALLIB) \
#			$(ONELIB)	\
#			$(LB)$/basic.lib

#.IF "$(GUI)"=="UNX"
#APP4STDLIBS+= \
#			$(VOSLIB) $(SALLIB)
#.ENDIF


#APP4DEPN=\
#		$(L)$/svtool.lib \
#		$(L)$/itools.lib \
#		$(SVLIBDEPEND) \
#		$(OBJ)$/jstest.obj

#APP4OBJS=		$(OBJ)$/jstest.obj

#.ENDIF

# --- Targets -----------------------------------------------------------

ALL: $(LIB1TARGET)		   \
     $(LB)$/basic.lib       \
     ALLTAR


#-------------------------------------------------------------------------
#								Windows 3.x
#-------------------------------------------------------------------------


.IF "$(GUI)" == "WIN"

LINKFLAGS+=/NOCV /IG
LINK=$(DEVROOT)$/bin\optlinks\optlinks

$(MISC)$/$(PRJNAME).def: makefile.mk
    echo NAME		 BASIC											 >$@
    echo DESCRIPTION 'StarBASIC DevSystem (C)1994 STAR DIVISION GmbH'>>$@
    echo EXETYPE	 WINDOWS										 >>$@
    echo PROTMODE													 >>$@
    echo STUB		 'winSTUB.EXE'                                   >>$@
    echo CODE		 LOADONCALL MOVEABLE							 >>$@
    echo DATA		 PRELOAD MULTIPLE MOVEABLE						 >>$@
    echo HEAPSIZE	 4096											 >>$@
    echo STACKSIZE	 30000											 >>$@
.ENDIF # GUI == WIN

#-------------------------------------------------------------------------
#								MAC
#-------------------------------------------------------------------------

.IF "$(GUI)" == "MAC"

$(MISC)$/$(PRJNAME).def: makefile.mk
    echo Kein def-File fuer Applikationen auf Mac
.ENDIF # GUI == MAC

#-------------------------------------------------------------------------
#										OS/2
#-------------------------------------------------------------------------

.IF "$(GUI)" == "OS2"

$(MISC)$/$(PRJNAME).def: makefile.mk
.IF "$(COM)"!="WTC"
    echo NAME		 BASIC WINDOWAPI								  >$@
    echo DESCRIPTION 'StarBASIC DevSystem (C)1993 STAR DIVISION GmbH' >>$@
    echo EXETYPE	 OS2											  >>$@
    echo PROTMODE													  >>$@
    echo STUB		 'OS2STUB.EXE'                                    >>$@
    echo CODE		 LOADONCALL 									  >>$@
    echo DATA		 PRELOAD MULTIPLE								  >>$@
    echo HEAPSIZE	 4096											  >>$@
    echo STACKSIZE	 30000											  >>$@
.ELSE
    @echo option DESCRIPTION 'StarBasic DLL'                           >$@
    @echo name $(BIN)$/$(SHL1TARGET).dll                             >>$@
#    @ldump -E1 -A -F$(MISC)$/$(SHL1TARGET).flt $(SLB)$/sb.lib    >>temp.def
    @ldump -E1 -A -F$(MISC)$/$(SHL1TARGET).flt $(LIB1TARGET)    >>temp.def
    @awk -f s:\util\exp.awk temp.def
    del temp.def
.ENDIF

.ENDIF # GUI == OS2

#-------------------------------------------------------------------------
#								Windows NT
#-------------------------------------------------------------------------
#
#					default targets aus target.mk
#

# --- Basic-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo Sbi >> $@
    @echo SvRTL >> $@
    @echo SbRtl_ >> $@
    @echo exception >> $@
    @echo bad_alloc >> $@
    @echo __CT >> $@
.IF "$(GUI)"=="OS2"
    @echo __alloc	>> $@
    @echo __malloc	>> $@
.ENDIF

.ENDIF

# ------------------------------------------------------------------------
.ENDIF

.INCLUDE :  target.mk

$(SRS)$/basic.srs:
    +$(TYPE) $(SRS)$/classes.srs + $(SRS)$/runtime.srs > $@


