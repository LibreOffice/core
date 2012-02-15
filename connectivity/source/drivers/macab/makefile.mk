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
TARGET=macab
TARGET2=$(TARGET)drv
VISIBILITY_HIDDEN=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : $(PRJ)$/makefile.pmk
.INCLUDE :  $(PRJ)$/version.mk

.IF "$(GUI)" == "UNX"
.IF "$(OS)" == "MACOSX"

# === MACAB base library ==========================

# --- Files -------------------------------------

SLOFILES= \
    $(SLO)$/MacabDriver.obj     \
    $(SLO)$/MacabServices.obj

DEPOBJFILES= \
    $(SLO2FILES)

# --- Library -----------------------------------

SHL1TARGET= $(TARGET)$(MACAB_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)                  \
    $(CPPUHELPERLIB)            \
    $(DBTOOLSLIB)               \
    $(SALLIB)

SHL1DEPN=
SHL1IMPLIB= i$(TARGET)

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME=   $(SHL1TARGET)

# === MACAB impl library ==========================

# --- Files -------------------------------------

SLO2FILES=\
    $(SLO)$/MacabColumns.obj            \
    $(SLO)$/MacabTable.obj              \
    $(SLO)$/MacabTables.obj             \
    $(SLO)$/MacabCatalog.obj            \
    $(SLO)$/MacabResultSet.obj          \
    $(SLO)$/MacabStatement.obj          \
    $(SLO)$/MacabPreparedStatement.obj  \
    $(SLO)$/MacabDatabaseMetaData.obj   \
    $(SLO)$/MacabConnection.obj         \
    $(SLO)$/MacabResultSetMetaData.obj  \
    $(SLO)$/macabcondition.obj          \
    $(SLO)$/macaborder.obj              \
        $(SLO)$/MacabRecord.obj             \
        $(SLO)$/MacabRecords.obj            \
        $(SLO)$/MacabHeader.obj             \
        $(SLO)$/MacabGroup.obj              \
        $(SLO)$/MacabAddressBook.obj
 
MACAB_LIB=-framework Carbon -framework AddressBook

# --- Library -----------------------------------

SHL2TARGET= $(TARGET2)$(MACAB_MAJOR)
SHL2OBJS=$(SLO2FILES)
SHL2STDLIBS=\
    $(CPPULIB)                  \
    $(CPPUHELPERLIB)            \
    $(VOSLIB)                   \
    $(SALLIB)                   \
    $(DBTOOLSLIB)               \
    $(COMPHELPERLIB)            \
    $(MACAB_LIB)

SHL2DEPN=
SHL2IMPLIB= i$(TARGET2)

SHL2DEF=    $(MISC)$/$(SHL2TARGET).def

DEF2NAME=   $(SHL2TARGET)

# --- Targets -----------------------------------
.ELSE		# "$(OS)" == "MACOSX"
dummy:
    @echo Not using Mac OS X - nothing to build
.ENDIF

.ELSE		# "$(GUI)" == "UNX"
dummy:
    @echo "Nothing to build for GUI $(GUI)"
.ENDIF

.INCLUDE : $(PRJ)$/target.pmk


ALLTAR : $(MISC)/macab1.component

$(MISC)/macab1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        macab1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt macab1.component
