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

# 2 == Unicode
MAJOR_VERSION=2

PRJ=..
PRJNAME=package
TARGET=package

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(L10N_framework)"==""

# --- General ----------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	\
    $(SLB)$/zipapi.lib \
    $(SLB)$/zippackage.lib \
    $(SLB)$/manifest.lib

# --- Shared-Library -----------------------------------------------

SHL1TARGET=$(TARGET)$(MAJOR_VERSION)
SHL1IMPLIB=i$(TARGET)
SHL1USE_EXPORTS=name

SHL1STDLIBS=\
    $(CPPULIB)		\
    $(UCBHELPERLIB)		\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)		\
    $(SALLIB)		\
    $(ZLIB3RDLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)
DEF1NAME=$(SHL1TARGET)

.ENDIF # L10N_framework

# --- Targets ----------------------------------------------------------

.INCLUDE :  target.mk


ALLTAR : $(MISC)/package2.component

$(MISC)/package2.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        package2.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt package2.component
