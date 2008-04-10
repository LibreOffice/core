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
.IF "$(GUI)"=="WNT"

PRJ=..$/..$/..
PRJINC=..$/..
PRJNAME=connectivity
TARGET=ado

#	ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

# --- Files -------------------------------------
        
EXCEPTIONSFILES= \
        $(SLO)$/ADatabaseMetaDataImpl.obj				\
        $(SLO)$/Aolevariant.obj							\
        $(SLO)$/ADatabaseMetaData.obj					\
        $(SLO)$/AColumn.obj								\
        $(SLO)$/AColumns.obj							\
        $(SLO)$/AIndex.obj								\
        $(SLO)$/AIndexes.obj							\
        $(SLO)$/AKey.obj								\
        $(SLO)$/AKeys.obj								\
        $(SLO)$/AUser.obj								\
        $(SLO)$/AUsers.obj								\
        $(SLO)$/AGroup.obj								\
        $(SLO)$/AGroups.obj								\
        $(SLO)$/ACatalog.obj							\
        $(SLO)$/AView.obj								\
        $(SLO)$/AViews.obj								\
        $(SLO)$/ATable.obj								\
        $(SLO)$/ATables.obj								\
        $(SLO)$/ACallableStatement.obj					\
        $(SLO)$/ADatabaseMetaDataResultSetMetaData.obj	\
        $(SLO)$/ADatabaseMetaDataResultSet.obj			\
        $(SLO)$/AResultSet.obj							\
        $(SLO)$/AConnection.obj							\
        $(SLO)$/AStatement.obj							\
        $(SLO)$/APreparedStatement.obj					\
        $(SLO)$/AResultSetMetaData.obj					\
        $(SLO)$/ADriver.obj								\
        $(SLO)$/Aservices.obj

SLOFILES=\
        $(EXCEPTIONSFILES)								\
        $(SLO)$/Awrapado.obj							\
        $(SLO)$/adoimp.obj
# --- Library -----------------------------------

SHL1TARGET=	$(ADO_TARGET)$(ADO_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SALLIB)					\
    $(OLE32LIB)					\
    $(OLEAUT32LIB)				\
    $(UUIDLIB)					\
    $(DBTOOLSLIB)				\
    $(COMPHELPERLIB)

.IF "$(COMPHELPERLIB)" == ""
SHL1STDLIBS+= icomphelp2.lib
.ENDIF

#SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ----------------------------------

.INCLUDE : target.mk

.ENDIF # "$(GUI)"=="WNT"

