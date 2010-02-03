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
# $Revision: 1.22 $
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
PRJNAME=ucb
TARGET=ucb
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

# Version
UCB_MAJOR=1

.INCLUDE: settings.mk
.IF "$(L10N_framework)"==""

SLOFILES=\
    $(SLO)$/identify.obj \
    $(SLO)$/ucb.obj \
    $(SLO)$/ucbserv.obj \
    $(SLO)$/ucbstore.obj \
    $(SLO)$/ucbprops.obj \
    $(SLO)$/provprox.obj \
    $(SLO)$/ucbcmds.obj \
    $(SLO)$/cmdenv.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

SHL1TARGET=$(TARGET)$(UCB_MAJOR)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB) \
    $(UCBHELPERLIB)
SHL1LIBS=\
    $(LIB1TARGET) \
    $(SLB)$/regexp.lib
SHL1IMPLIB=i$(TARGET)

.IF "$(GUI)" == "OS2"
DEF1EXPORTFILE=exports2.dxp
.ELSE
SHL1VERSIONMAP=exports.map
.ENDIF

DEF1NAME=$(SHL1TARGET)

.ENDIF # L10N_framework

.INCLUDE: target.mk

