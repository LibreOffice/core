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
TARGET=kab
TARGET2=$(TARGET)drv

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.INCLUDE : $(PRJ)$/makefile.pmk
.INCLUDE :  $(PRJ)$/version.mk

.IF "$(GUI)" == "UNX"
.IF "$(ENABLE_KAB)" == "TRUE"

CFLAGS+=$(KDE_CFLAGS)

.IF "$(KDE_ROOT)"!=""
EXTRALIBPATHS+=-L$(KDE_ROOT)$/lib
.IF "$(OS)$(CPU)" == "LINUXX"
EXTRALIBPATHS+=-L$(KDE_ROOT)$/lib64
.ENDIF
.ENDIF

# === KAB base library ==========================

# --- Files -------------------------------------

SLOFILES= \
    $(SLO)$/KDriver.obj     \
    $(SLO)$/KServices.obj

DEPOBJFILES= \
    $(SLO2FILES)

# --- Library -----------------------------------

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1TARGET= $(TARGET)$(KAB_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)                  \
    $(CPPUHELPERLIB)            \
    $(DBTOOLSLIB)               \
    $(SALLIB)                   \
    $(SALHELPERLIB)

SHL1DEPN=
SHL1IMPLIB= i$(TARGET)

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME=   $(SHL1TARGET)

# === KAB impl library ==========================

# --- Files -------------------------------------

SLO2FILES=\
    $(SLO)$/KColumns.obj            \
    $(SLO)$/KTable.obj              \
    $(SLO)$/KTables.obj             \
    $(SLO)$/KCatalog.obj            \
    $(SLO)$/KResultSet.obj          \
    $(SLO)$/KStatement.obj          \
    $(SLO)$/KPreparedStatement.obj  \
    $(SLO)$/KDatabaseMetaData.obj   \
    $(SLO)$/KConnection.obj         \
    $(SLO)$/KResultSetMetaData.obj  \
    $(SLO)$/kcondition.obj          \
    $(SLO)$/korder.obj              \
    $(SLO)$/kfields.obj             \
    $(SLO)$/KDEInit.obj
 
KAB_LIB=$(KDE_LIBS) -lkabc

# --- Library -----------------------------------

SHL2VERSIONMAP=$(TARGET2).map

SHL2TARGET= $(TARGET2)$(KAB_MAJOR)
SHL2OBJS=$(SLO2FILES)
SHL2LINKFLAGS+=$(KAB_LIB)
SHL2STDLIBS=\
    $(CPPULIB)                  \
    $(CPPUHELPERLIB)            \
    $(SALLIB)                   \
    $(SALHELPERLIB)                   \
    $(DBTOOLSLIB)               \
    $(COMPHELPERLIB)

SHL2DEPN=
SHL2IMPLIB= i$(TARGET2)

SHL2DEF=    $(MISC)$/$(SHL2TARGET).def

DEF2NAME=   $(SHL2TARGET)

# --- Targets -----------------------------------
.ELSE		# "$(ENABLE_KAB)" == "TRUE"
dummy:
    @echo KDE Addressbook disabled - nothing to build
.ENDIF

.ELSE		# "$(GUI)" == "UNX"
dummy:
    @echo "Nothing to build for GUI $(GUI)"
.ENDIF

.INCLUDE : $(PRJ)$/target.pmk


ALLTAR : $(MISC)/kab1.component

$(MISC)/kab1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        kab1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt kab1.component
