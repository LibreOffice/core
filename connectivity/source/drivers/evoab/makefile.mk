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
TARGET=evoab

VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE :  $(PRJ)$/version.mk
.INCLUDE : $(PRJ)$/makefile.pmk

.IF "$(GUI)"=="UNX"

# --- Files -------------------------------------

EXCEPTIONSFILES=\
    $(SLO)$/LResultSet.obj			\
    $(SLO)$/LStatement.obj			\
    $(SLO)$/LPreparedStatement.obj	\
    $(SLO)$/LFolderList.obj			\
    $(SLO)$/LConfigAccess.obj		\
    $(SLO)$/LColumnAlias.obj		\
    $(SLO)$/LTable.obj				\
    $(SLO)$/LDatabaseMetaData.obj	\
    $(SLO)$/LCatalog.obj			\
    $(SLO)$/LColumns.obj			\
    $(SLO)$/LTables.obj				\
    $(SLO)$/LConnection.obj			\
    $(SLO)$/LServices.obj			\
    $(SLO)$/LDriver.obj

SLOFILES=\
    $(EXCEPTIONSFILES)				\
    $(SLO)$/LDebug.obj


SHL1VERSIONMAP=$(SOLARENV)/src/component.map


# --- Library -----------------------------------

SHL1TARGET=	$(EVOAB_TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(TOOLSLIB)					\
    $(SVTOOLLIB)				\
    $(UNOTOOLSLIB)				\
    $(UCBHELPERLIB)				\
    $(SALLIB)					\
    $(SALHELPERLIB)					\
    $(DBTOOLSLIB)				\
    $(DBFILELIB)				\
    $(COMPHELPERLIB)

.IF "$(DBFILELIB)" == ""
SHL1STDLIBS+= ifile.lib
.ENDIF

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)

.ELSE
dummy:
    @echo "Nothing to build for GUI $(GUI)"
.ENDIF

# --- Targets ----------------------------------

.INCLUDE : $(PRJ)$/target.pmk
