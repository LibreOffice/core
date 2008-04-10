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
# $Revision: 1.3 $
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

PRJ=..$/..$/..
PRJINC=..$/..
PRJNAME=connectivity
TARGET=macab
TARGET2=$(TARGET)drv

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

.IF "$(GUI)" == "UNX"
.IF "$(OS)" == "MACOSX"

# === MACAB base library ==========================

# --- Files -------------------------------------

SLOFILES= \
    $(SLO)$/MacabDriver.obj     \
    $(SLO)$/MacabServices.obj

DEPOBJFILES= \
    $(SLO2FILES)

# --- Library -----------------------------------

SHL1VERSIONMAP=$(TARGET).map

SHL1TARGET= $(TARGET)$(MACAB_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)                  \
    $(CPPUHELPERLIB)            \
    $(SALLIB)

SHL1DEPN=
SHL1IMPLIB= i$(TARGET)

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME=   $(SHL1TARGET)

# === MACAB impl library ==========================

# --- Files -------------------------------------

SLO2FILES=\
    $(SLO)$/MacabColumns.obj            \
    $(SLO)$/MacabTable.obj              \
    $(SLO)$/MacabTables.obj             \
    $(SLO)$/MacabCatalog.obj            \
    $(SLO)$/MacabResultSet.obj          \
    $(SLO)$/MacabStatement.obj          \
    $(SLO)$/MacabPreparedStatement.obj  \
    $(SLO)$/MacabDatabaseMetaData.obj   \
    $(SLO)$/MacabConnection.obj         \
    $(SLO)$/MacabResultSetMetaData.obj  \
    $(SLO)$/macabcondition.obj          \
    $(SLO)$/macaborder.obj              \
        $(SLO)$/MacabRecord.obj             \
        $(SLO)$/MacabRecords.obj            \
        $(SLO)$/MacabHeader.obj             \
        $(SLO)$/MacabGroup.obj              \
        $(SLO)$/MacabAddressBook.obj
 
MACAB_LIB=-framework Carbon -framework AddressBook

# --- Library -----------------------------------

SHL2VERSIONMAP=$(TARGET2).map

SHL2TARGET= $(TARGET2)$(MACAB_MAJOR)
SHL2OBJS=$(SLO2FILES)
SHL2STDLIBS=\
    $(CPPULIB)                  \
    $(CPPUHELPERLIB)            \
    $(VOSLIB)                   \
    $(SALLIB)                   \
    $(DBTOOLSLIB)               \
    $(COMPHELPERLIB)            \
    $(MACAB_LIB)

SHL2DEPN=
SHL2IMPLIB= i$(TARGET2)

SHL2DEF=    $(MISC)$/$(SHL2TARGET).def

DEF2NAME=   $(SHL2TARGET)

# --- Targets -----------------------------------
.ELSE		# "$(OS)" == "MACOSX"
dummy:
    @echo Not using Mac OS X - nothing to build
.ENDIF

.ELSE		# "$(GUI)" == "UNX"
dummy:
    @echo "Nothing to build for GUI $(GUI)"
.ENDIF

.INCLUDE : target.mk

