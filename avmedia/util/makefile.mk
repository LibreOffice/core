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
PRJNAME=avmedia
TARGET=avmedia
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Resources ---------------------------------

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=			\
    $(SRS)$/viewer.srs		\
    $(SRS)$/framework.srs

# --- Files -------------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=\
    $(SLB)$/viewer.lib		\
    $(SLB)$/framework.lib

# ==========================================================================

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB=i$(TARGET)
SHL1STDLIBS=$(UNOTOOLSLIB) $(TOOLSLIB) $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB) $(VCLLIB) $(COMPHELPERLIB) $(SVTOOLLIB) $(SVLLIB) $(SFXLIB)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(SLB)$/$(TARGET).lib

DEF1NAME=$(SHL1TARGET)
DEF1DEPN=$(MISC)$/$(SHL1TARGET).flt $(LIB1TARGET)
DEF1DES=Avmedia
DEFLIB1NAME	=$(TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk

.IF "$(depend)"==""
$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo _Impl>$@
    @echo WEP>>$@
    @echo LIBMAIN>>$@
    @echo LibMain>>$@
    @echo CT>>$@
.ENDIF

ALLTAR : $(MISC)/avmedia.component

$(MISC)/avmedia.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        avmedia.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt avmedia.component
