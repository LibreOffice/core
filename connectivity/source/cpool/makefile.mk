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



PRJ=..$/..
PRJINC=..
PRJNAME=connectivity
TARGET=dbpool

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk


# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/ZConnectionWrapper.obj			\
        $(SLO)$/ZDriverWrapper.obj				\
        $(SLO)$/ZPooledConnection.obj			\
        $(SLO)$/ZConnectionPool.obj				\
        $(SLO)$/ZPoolCollection.obj				\
        $(SLO)$/Zregistration.obj

# --- Library -----------------------------------

SHL1TARGET=	$(DBPOOL_TARGET)$(DBPOOL_MAJOR)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(DBTOOLSLIB)				\
    $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ----------------------------------

.INCLUDE : target.mk

ALLTAR : $(MISC)/dbpool2.component

$(MISC)/dbpool2.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        dbpool2.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt dbpool2.component
