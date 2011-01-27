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

PRJNAME=sot
TARGET=sot

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
.IF "$(GUI)$(COM)"=="WNTGCC"
LIB1ARCHIV= $(LB)$/lib$(TARGET)$(DLLPOSTFIX)_static.a
.ELSE
LIB1ARCHIV= $(LB)$/lib$(TARGET)$(DLLPOSTFIX).a
.ENDIF
LIB1FILES=	$(SLB)$/base.lib \
            $(SLB)$/sdstor.lib \
            $(SLB)$/unoolestorage.lib

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= $(TARGET)
SHL1USE_EXPORTS=name
SHL1LIBS=	$(SLB)$/$(TARGET).lib

SHL1STDLIBS=$(TOOLSLIB) $(SALLIB) $(UNOTOOLSLIB) $(CPPUHELPERLIB) $(COMPHELPERLIB) $(UCBHELPERLIB) $(CPPULIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
         $(PRJ)$/inc$/sot/absdev.hxx	\
         $(PRJ)$/inc$/sot/agg.hxx       \
         $(PRJ)$/inc$/sot$/exchange.hxx  \
         $(PRJ)$/inc$/sot$/factory.hxx   \
         $(PRJ)$/inc$/sot$/object.hxx    \
         $(PRJ)$/inc$/sot$/sotdata.hxx   \
         $(PRJ)$/inc$/sot$/sotref.hxx    \
         $(PRJ)$/inc$/sot/stg.hxx       \
         $(PRJ)$/inc$/sot$/storage.hxx   \
         $(PRJ)$/inc$/sot/storinfo.hxx
DEFLIB1NAME =$(TARGET)
DEF1DES 	=StarObjectsTools

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    $(TYPE) sot.flt > $@


ALLTAR : $(MISC)/sot.component

$(MISC)/sot.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        sot.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt sot.component
