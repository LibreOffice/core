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

PRJ=..$/..$/..
PRJINC=..$/..
PRJNAME=connectivity
TARGET=macab
TARGET2=$(TARGET)drv
VISIBILITY_HIDDEN=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : $(PRJ)$/makefile.pmk
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

SHL1TARGET= $(TARGET)$(MACAB_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)                  \
    $(CPPUHELPERLIB)            \
    $(DBTOOLSLIB)               \
    $(SALHELPERLIB)             \
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

SHL2TARGET= $(TARGET2)$(MACAB_MAJOR)
SHL2OBJS=$(SLO2FILES)
SHL2STDLIBS=\
    $(CPPULIB)                  \
    $(CPPUHELPERLIB)            \
    $(SALHELPERLIB)             \
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

.INCLUDE : $(PRJ)$/target.pmk


ALLTAR : $(MISC)/macab1.component

$(MISC)/macab1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        macab1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt macab1.component
