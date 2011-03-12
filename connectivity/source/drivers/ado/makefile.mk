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
TARGET=ado
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE :  $(PRJ)$/version.mk
.INCLUDE : $(PRJ)$/makefile.pmk

.IF "$(GUI)"=="WNT"
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

SHL1TARGET=	$(ADO_TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(SALLIB)					\
    $(SALHELPERLIB)				\
    $(OLE32LIB)					\
    $(OLEAUT32LIB)				\
    $(UUIDLIB)					\
    $(DBTOOLSLIB)				\
    $(COMPHELPERLIB)

.IF "$(COMPHELPERLIB)" == ""
SHL1STDLIBS+= icomphelp2.lib
.ENDIF

SHL1IMPLIB=	i$(SHL1TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

.ELSE
dummy:
    @echo "Nothing to build for GUI $(GUI)"
.ENDIF # "$(GUI)"=="WNT"
# --- Targets ----------------------------------
.INCLUDE : $(PRJ)$/target.pmk


ALLTAR : $(MISC)/ado.component

$(MISC)/ado.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ado.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ado.component
