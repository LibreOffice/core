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
TARGET=file

USE_DEFFILE=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

CDEFS += -DOOO_DLLIMPLEMENTATION_FILE

# --- Files -------------------------------------
EXCEPTIONSFILES=\
    $(SLO)$/FCatalog.obj			\
    $(SLO)$/FColumns.obj			\
    $(SLO)$/FConnection.obj			\
    $(SLO)$/FDatabaseMetaData.obj		\
    $(SLO)$/FDateFunctions.obj			\
    $(SLO)$/FDriver.obj				\
    $(SLO)$/FNoException.obj			\
    $(SLO)$/FNumericFunctions.obj		\
    $(SLO)$/FPreparedStatement.obj		\
    $(SLO)$/FResultSet.obj			\
    $(SLO)$/FResultSetMetaData.obj		\
    $(SLO)$/FStatement.obj			\
    $(SLO)$/FStringFunctions.obj		\
    $(SLO)$/FTable.obj				\
    $(SLO)$/FTables.obj				\
    $(SLO)$/fanalyzer.obj			\
    $(SLO)$/fcode.obj				\
    $(SLO)$/fcomp.obj

SLOFILES=\
    $(EXCEPTIONSFILES)				\
    $(SLO)$/quotedstring.obj


# --- Library -----------------------------------
SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(SVLLIB)					\
    $(TOOLSLIB)					\
    $(UCBHELPERLIB)				\
    $(SALLIB)					\
    $(SALHELPERLIB)					\
    $(DBTOOLSLIB)				\
    $(UNOTOOLSLIB)				\
    $(COMPHELPERLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
            $(SLB)$/$(TARGET).lib
DEFLIB1NAME=$(TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk


# --- filter file ------------------------------

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo _TI				>$@
    @echo _real				>>$@

