#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: hr $ $Date: 2003-03-18 16:28:40 $
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

.INCLUDE :  settings.mk

# --- Allgemein ---------------------------------------------------

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
            $(VOSLIB) \
            $(SALLIB) \
            $(SVMEMLIB)	\
            $(COMPHELPERLIB) \
            $(UNOTOOLSLIB) \
            $(SOTLIB) \
            $(VOSLIB)

.IF "$(SOLAR_JAVA)" != "TRUE"
SHL1STDLIBS+=$(SJLIB)
.ENDIF

.IF "$(GUI)"=="WNT"
#SHL1STDLIBS+=$(LIBCIMT)
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


RES1TARGET=$(PRJNAME)
SRS1FILES= \
        $(SRS)$/classes.srs

# --- TESTTOOL IDE ------------------------------------------------------
# die ressourcen werden hier gelinkt

RESLIB1NAME=stt
RESLIB1SRSFILES= \
        $(SRS)$/app.srs \
        $(SRS)$/classes.srs

# --- Targets -----------------------------------------------------------

.INCLUDE :  target.mk

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

$(SRS)$/basic.srs:
    +$(TYPE) $(SRS)$/classes.srs + $(SRS)$/runtime.srs > $@


