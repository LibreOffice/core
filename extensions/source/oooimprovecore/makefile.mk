#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.2 $
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

PRJ=..$/..
PRJNAME=extensions
TARGET=oooimprovecore

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Files -------------------------------------

EXCEPTIONSFILES= \
        $(SLO)$/core.obj \
        $(SLO)$/oooimprovecore_services.obj \

SLOFILES= \
        $(EXCEPTIONSFILES) \
        $(SLO)$/oooimprovecore_module.obj \

LIB1TARGET=$(SLB)$/$(TARGET)_t.lib
LIB1FILES= \
        $(SLB)$/$(TARGET).lib \

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
.IF "$(GUI)"=="OS2"
SHL1TARGET=oooimpc$(DLLPOSTFIX)
.ENDIF

SHL1STDLIBS= \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB) \
        $(VCLLIB) \
        $(SVLLIB) \
        $(SVTOOLLIB) \
        $(SFXLIB) \
        $(SVXCORELIB) \
        $(SVXLIB) \
        $(TOOLSLIB)


SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
SHL1VERSIONMAP=$(TARGET).map

# --- Targets ----------------------------------

.INCLUDE : target.mk

