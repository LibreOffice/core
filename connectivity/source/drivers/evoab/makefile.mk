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
#SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1TARGET=	$(EVOAB_TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(TOOLSLIB)					\
    $(SVTOOLLIB)				\
    $(UNOTOOLSLIB)				\
    $(UCBHELPERLIB)				\
    $(SALLIB)					\
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
