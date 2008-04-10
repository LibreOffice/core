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
# $Revision: 1.16 $
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
TARGET=odbcbase
TARGET2=odbc

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
LDUMP=ldump2.exe

# --- Settings ----------------------------------

.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

.IF "$(SYSTEM_ODBC_HEADERS)" == "YES"
CFLAGS+=-DSYSTEM_ODBC_HEADERS
.ENDIF

# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/OPreparedStatement.obj			\
        $(SLO)$/OStatement.obj					\
        $(SLO)$/OResultSetMetaData.obj			\
        $(SLO)$/OResultSet.obj					\
        $(SLO)$/OTools.obj						\
        $(SLO)$/ODatabaseMetaDataResultSet.obj	\
        $(SLO)$/ODatabaseMetaData.obj			\
        $(SLO)$/ODriver.obj						\
        $(SLO)$/OConnection.obj

# --- ODBC BASE Library -----------------------------------

SHL1TARGET=	$(ODBC2_TARGET)$(ODBC2_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(DBTOOLSLIB)				\
    $(COMPHELPERLIB)			\
    $(CPPUHELPERLIB)			\
    $(CPPULIB)					\
    $(VOSLIB)					\
    $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(ODBC2_TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
            $(SLB)$/$(TARGET).lib
DEFLIB1NAME=$(TARGET)

# --- ODBC Library -----------------------------------
# --- Files -------------------------------------

SLO2FILES=\
        $(SLO)$/oservices.obj	\
        $(SLO)$/ORealDriver.obj	\
        $(SLO)$/OFunctions.obj

# --- ODBC Library -----------------------------------

SHL2TARGET=	$(ODBC_TARGET)$(ODBC_MAJOR)
SHL2OBJS=$(SLO2FILES)
SHL2STDLIBS=\
    $(ODBCBASELIB)				\
    $(CPPUHELPERLIB)			\
    $(CPPULIB)					\
    $(SALLIB)

.IF "$(ODBCBASELIB)" == ""
SHL2STDLIBS+= $(ODBCBASELIB)
.ENDIF

SHL2DEPN=$(SHL1TARGETN)
SHL2IMPLIB=	i$(ODBC_TARGET)

SHL2DEF=	$(MISC)$/$(SHL2TARGET).def

DEF2NAME=	$(SHL2TARGET)
SHL2VERSIONMAP=odbc.map

# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- filter file ------------------------------

.IF "$(depend)"==""

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo CLEAR_THE_FILE	> $@
    @echo _TI				>>$@
    @echo _real				>>$@
.ENDIF

