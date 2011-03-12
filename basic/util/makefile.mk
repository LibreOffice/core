#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
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
    $(SLB)$/basicmgr.lib \
    $(SLB)$/classes.lib \
    $(SLB)$/comp.lib \
    $(SLB)$/uno.lib \
    $(SLB)$/runtime.lib \
    $(SLB)$/sbx.lib

SHL1TARGET= sb$(DLLPOSTFIX)
SHL1IMPLIB= basic

SHL1STDLIBS= \
            $(CPPULIB) \
            $(CPPUHELPERLIB) \
            $(TOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB)	\
            $(VCLLIB) \
            $(SALLIB) \
            $(SALHELPERLIB) \
            $(COMPHELPERLIB) \
            $(UNOTOOLSLIB) \
            $(SOTLIB) \
            $(XMLSCRIPTLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=	\
    $(UWINAPILIB)	\
    $(OLEAUT32LIB)
.ENDIF # WNT

.IF "$(GUI)" != "UNX"
.IF "$(COM)" != "GCC"
SHL1OBJS=	\
    $(SLO)$/sb.obj
.ENDIF
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
        $(SRS)$/classes.srs \
        $(SRS)$/sbx.srs

# --- TESTTOOL IDE ------------------------------------------------------
# The resources are linked here

RESLIB1NAME=stt
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES= \
        $(SRS)$/app.srs
        
RESLIB2NAME=sb
RESLIB2IMAGES=$(PRJ)$/res
RESLIB2SRSFILES= \
        $(SRS)$/classes.srs \
        $(SRS)$/sbx.srs
########## remove sbx.srx (and this line) when issue i53795 is fixed

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
    $(TYPE) $(SRS)$/classes.srs + $(SRS)$/runtime.srs + $(SRS)$/sbx.srs > $@

ALLTAR : $(MISC)/sb.component

$(MISC)/sb.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        sb.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt sb.component
