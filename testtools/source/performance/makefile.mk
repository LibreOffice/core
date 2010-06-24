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

PRJ=..$/..

PRJNAME=testtools
TARGET=performancetest
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

LIB1TARGET=$(SLB)$/perftest.lib
LIB1OBJFILES= \
        $(SLO)$/ubtest.obj

SHL1TARGET = perftest.uno

SHL1VERSIONMAP = $(SOLARENV)$/src$/component.map

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)

# ---- test object ----

LIB2TARGET=$(SLB)$/perfobj.lib
LIB2OBJFILES= \
        $(SLO)$/ubobject.obj

SHL2TARGET = perfobj.uno

SHL2VERSIONMAP = $(SOLARENV)$/src$/component.map

SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)

SHL2LIBS=	$(LIB2TARGET)
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)

# ---- pseudo uno bridge ----

LIB3TARGET=$(SLB)$/pseudo.lib
LIB3OBJFILES= \
        $(SLO)$/pseudo.obj

SHL3TARGET=pseudo_uno_uno

SHL3VERSIONMAP = pseudo_uno_uno.map

SHL3STDLIBS= \
        $(CPPULIB)		\
        $(SALLIB)

SHL3LIBS=	$(LIB3TARGET)
SHL3DEF=	$(MISC)$/$(SHL3TARGET).def
DEF3NAME=	$(SHL3TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
