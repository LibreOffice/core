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
TARGET=mysql

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
        $(SLO)$/YDriver.obj				\
        $(SLO)$/YTables.obj				\
        $(SLO)$/YTable.obj				\
        $(SLO)$/YViews.obj				\
        $(SLO)$/YCatalog.obj			\
        $(SLO)$/YColumns.obj			\
        $(SLO)$/YUser.obj				\
        $(SLO)$/YUsers.obj				\
        $(SLO)$/Yservices.obj

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- Library -----------------------------------

REALNAME:=$(MYSQL_TARGET).uno
SHL1TARGET=$(REALNAME)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(COMPHELPERLIB)


SHL1DEPN=
SHL1IMPLIB=	i$(MYSQL_TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ----------------------------------

.INCLUDE : $(PRJ)$/target.pmk



ALLTAR : $(MISC)/mysql.component

$(MISC)/mysql.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        mysql.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt mysql.component
