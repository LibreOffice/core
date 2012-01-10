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
    $(SLO)$/FNoException.obj		\
    $(SLO)$/FConnection.obj			\
    $(SLO)$/FResultSetMetaData.obj	\
    $(SLO)$/FColumns.obj			\
    $(SLO)$/FTables.obj				\
    $(SLO)$/FTable.obj				\
    $(SLO)$/FCatalog.obj			\
    $(SLO)$/FResultSet.obj			\
    $(SLO)$/FStatement.obj			\
    $(SLO)$/FPreparedStatement.obj	\
    $(SLO)$/FDatabaseMetaData.obj	\
    $(SLO)$/FDriver.obj				\
    $(SLO)$/fanalyzer.obj			\
    $(SLO)$/fcomp.obj               \
    $(SLO)$/fcode.obj

SLOFILES=\
    $(EXCEPTIONSFILES)				\
    $(SLO)$/FStringFunctions.obj	\
    $(SLO)$/FDateFunctions.obj	\
    $(SLO)$/FNumericFunctions.obj \
    $(SLO)$/quotedstring.obj


# --- Library -----------------------------------
SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SVLLIB)					\
    $(TOOLSLIB)					\
    $(UCBHELPERLIB)				\
    $(SALLIB)					\
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

