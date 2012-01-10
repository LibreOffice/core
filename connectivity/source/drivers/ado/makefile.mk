#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..$/..$/..
PRJINC=..$/..
PRJNAME=connectivity
TARGET=ado
VISIBILITY_HIDDEN=TRUE

#	ENABLE_EXCEPTIONS=TRUE

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
