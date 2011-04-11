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

# UCP Version - Increase, if UCP libraray becomes incompatible.
UCP_VERSION=1

# Name for UCP. Will become part of the library name (See below).
UCP_NAME=pkg

PRJ=..$/..$/..

PRJNAME=ucb

TARGET=ucp$(UCP_NAME)

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

# --- General -----------------------------------------------------
.IF "$(L10N_framework)"==""

SLOFILES=\
    $(SLO)$/pkgservices.obj    	\
    $(SLO)$/pkguri.obj		\
    $(SLO)$/pkgprovider.obj    	\
    $(SLO)$/pkgcontent.obj     	\
    $(SLO)$/pkgcontentcaps.obj	\
    $(SLO)$/pkgresultset.obj	\
    $(SLO)$/pkgdatasupplier.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

# --- Shared-Library ---------------------------------------------------

SHL1TARGET=$(TARGET)$(UCP_VERSION)
SHL1IMPLIB=i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS=\
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(SALHELPERLIB)  \
    $(UCBHELPERLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------------------------------

.ENDIF # L10N_framework
.INCLUDE: target.mk


ALLTAR : $(MISC)/ucppkg1.component

$(MISC)/ucppkg1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucppkg1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucppkg1.component
