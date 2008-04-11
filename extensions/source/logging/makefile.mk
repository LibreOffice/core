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
# $Revision: 1.4 $
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
TARGET=log

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Files -------------------------------------

# ... object files ............................
EXCEPTIONSFILES= \
        $(SLO)$/log_services.obj        \
        $(SLO)$/logger.obj              \
        $(SLO)$/filehandler.obj         \
        $(SLO)$/plaintextformatter.obj  \
        $(SLO)$/consolehandler.obj      \
        $(SLO)$/loghandler.obj          \
        $(SLO)$/loggerconfig.obj        \

SLOFILES= \
        $(EXCEPTIONSFILES) \
        $(SLO)$/log_module.obj          \
        $(SLO)$/logrecord.obj           \

# --- library -----------------------------------

LIB1TARGET=$(SLB)$/$(TARGET)_t.lib
LIB1FILES=\
        $(SLB)$/$(TARGET).lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB)

SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
SHL1VERSIONMAP=$(TARGET).map

# --- Targets ----------------------------------

.INCLUDE : target.mk

