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
TARGET=adabas

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : $(PRJ)$/makefile.pmk
.INCLUDE : $(PRJ)$/version.mk

.IF "$(SYSTEM_ODBC_HEADERS)" == "YES"
CFLAGS+=-DSYSTEM_ODBC_HEADERS
.ENDIF

# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/BFunctions.obj					\
        $(SLO)$/BConnection.obj					\
        $(SLO)$/BDriver.obj						\
        $(SLO)$/BCatalog.obj					\
        $(SLO)$/BGroups.obj						\
        $(SLO)$/BGroup.obj						\
        $(SLO)$/BUser.obj						\
        $(SLO)$/BUsers.obj						\
        $(SLO)$/BKeys.obj						\
        $(SLO)$/BColumns.obj					\
        $(SLO)$/BIndex.obj						\
        $(SLO)$/BIndexColumns.obj				\
        $(SLO)$/BIndexes.obj					\
        $(SLO)$/BTable.obj						\
        $(SLO)$/BTables.obj						\
        $(SLO)$/BViews.obj						\
        $(SLO)$/Bservices.obj					\
        $(SLO)$/BDatabaseMetaData.obj			\
        $(SLO)$/BPreparedStatement.obj          \
        $(SLO)$/BStatement.obj                  \
        $(SLO)$/BResultSetMetaData.obj			\
        $(SLO)$/BResultSet.obj

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- Library -----------------------------------

SHL1TARGET=	$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(TOOLSLIB)					\
    $(ODBCBASELIB)				\
    $(UNOTOOLSLIB)				\
    $(COMPHELPERLIB)

.IF "$(ODBCBASELIB)" == ""
SHL1STDLIBS+=$(ODBCBASELIB)
.ENDIF

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ----------------------------------


.INCLUDE : $(PRJ)$/target.pmk

ALLTAR : $(MISC)/adabas.component

$(MISC)/adabas.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        adabas.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt adabas.component
