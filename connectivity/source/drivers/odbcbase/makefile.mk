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
TARGET=odbcbase

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

CDEFS += -DOOO_DLLIMPLEMENTATION_ODBCBASE

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

SHL1TARGET=	$(ODBC2_TARGET)$(DLLPOSTFIX)
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
SHL1USE_EXPORTS=name

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
            $(SLB)$/$(TARGET).lib
DEFLIB1NAME=$(TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- filter file ------------------------------

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo CLEAR_THE_FILE	> $@
