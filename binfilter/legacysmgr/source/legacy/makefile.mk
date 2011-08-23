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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ = ..$/..$/..
BFPRJ = ..$/..
PRJNAME = binfilter
TARGET = legacy_binfilters
USE_DEFFILE = TRUE
NO_BSYMBOLIC = TRUE
ENABLE_EXCEPTIONS = TRUE
# COMP1TYPELIST = $(TARGET)

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
INC+= -I$(PRJ)$/inc$/legacysmgr
# ------------------------------------------------------------------

SLOFILES = \
    $(SLO)$/legacy_binfilters_smgr.obj

SHL1TARGET = $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS = \
    $(CPPUHELPERLIB)	\
    $(CPPULIB)		\
    $(SALLIB)

.IF "$(COMNAME)"=="msci"
SHL1VERSIONMAP=msci.map
.ELIF "$(COMNAME)"=="sunpro5"
SHL1VERSIONMAP=sols.map
.ELIF "$(GUI)$(COM)"=="WNTGCC"
SHL1VERSIONMAP=mingw_intel.map
.ELIF "$(COMNAME)"=="gcc3"
SHL1VERSIONMAP=gcc3.map
.ENDIF

SHL1DEPN =
SHL1IMPLIB = i$(TARGET)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1DEF = $(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)

.IF "$(debug)" != ""

# msvc++: no inlining for debugging
.IF "$(COM)" == "MSC"
CFLAGS += -Ob0
.ENDIF

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

