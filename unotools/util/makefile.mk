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
PRJNAME=unotools
TARGET=utl
TARGETTYPE=CUI
USE_LDUMP2=TRUE

USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :	settings.mk

# --- Library -----------------------------------

LIB1TARGET=$(SLB)$/untools.lib
LIB1FILES=\
        $(SLB)$/i18n.lib \
        $(SLB)$/misc.lib \
        $(SLB)$/streaming.lib \
        $(SLB)$/config.lib \
        $(SLB)$/ucbhelp.lib \
        $(SLB)$/procfact.lib \
        $(SLB)$/property.lib \
        $(SLB)$/accessibility.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1IMPLIB=iutl
SHL1USE_EXPORTS=name

SHL1STDLIBS= \
        $(SALHELPERLIB) \
        $(COMPHELPERLIB) \
        $(UCBHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(I18NISOLANGLIB) \
        $(TOOLSLIB) \
        $(SALLIB)

SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1DEPN=$(LIB1TARGET)

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN        =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME     =untools
DEF1DES         =unotools

# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo CLEAR_THE_FILE		> $@
    @echo _TI					>> $@
    @echo _real					>> $@
    @echo NodeValueAccessor			>> $@
    @echo SubNodeAccess				>> $@
    @echo UpdateFromConfig			>> $@
    @echo UpdateToConfig				>> $@
        @echo _Impl >> $@


ALLTAR : $(MISC)/utl.component

$(MISC)/utl.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        utl.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt utl.component
