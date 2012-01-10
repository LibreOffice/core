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
TARGET=calc

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : $(PRJ)$/makefile.pmk
.INCLUDE :  $(PRJ)$/version.mk


# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/CResultSet.obj			\
    $(SLO)$/CStatement.obj			\
    $(SLO)$/CPreparedStatement.obj	\
    $(SLO)$/CDatabaseMetaData.obj	\
    $(SLO)$/CCatalog.obj			\
    $(SLO)$/CColumns.obj			\
    $(SLO)$/CTable.obj				\
    $(SLO)$/CTables.obj				\
    $(SLO)$/CConnection.obj			\
    $(SLO)$/Cservices.obj			\
    $(SLO)$/CDriver.obj

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- Library -----------------------------------

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SVLLIB)					\
    $(TOOLSLIB)					\
        $(UNOTOOLSLIB) \
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(DBFILELIB)				\
    $(COMPHELPERLIB)


#.IF "$(DBFILELIB)" == ""
#SHL1STDLIBS+= ifile.lib
#.ENDIF

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ----------------------------------

.INCLUDE : $(PRJ)$/target.pmk



ALLTAR : $(MISC)/calc.component

$(MISC)/calc.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        calc.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt calc.component
