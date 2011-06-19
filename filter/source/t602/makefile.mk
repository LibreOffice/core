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
PRJ	= ..$/..
PRJNAME = filter
TARGET  =t602filter
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

SRS1NAME = $(TARGET)
SRC1FILES = $(SRS1NAME).src

RESLIB1NAME=$(SRS1NAME)
RESLIB1SRSFILES= $(SRS)$/$(RESLIB1NAME).srs

SLOFILES=$(SLO)$/t602filter.obj \
         $(SLO)$/filterenv.obj

LIBNAME=$(TARGET)
SHL1TARGETDEPN=makefile.mk
SHL1OBJS=$(SLOFILES)
SHL1TARGET=$(LIBNAME)$(DLLPOSTFIX)
SHL1IMPLIB=i$(LIBNAME)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS= \
    $(XMLOFFLIB) \
    $(CPPULIB) \
    $(CPPUHELPERLIB) \
    $(SALLIB) \
    $(TOOLSLIB)

# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

ALLTAR : $(MISC)/t602filter.component

$(MISC)/t602filter.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        t602filter.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt t602filter.component
