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
TARGET=kab
TARGET2=$(TARGET)drv

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.INCLUDE : $(PRJ)$/makefile.pmk
.INCLUDE :  $(PRJ)$/version.mk

.IF "$(GUI)" == "UNX"
.IF "$(ENABLE_KAB)" == "TRUE"

CFLAGS+=$(KDE_CFLAGS)

.IF "$(KDE_ROOT)"!=""
EXTRALIBPATHS+=-L$(KDE_ROOT)$/lib
.IF "$(OS)$(CPU)" == "LINUXX"
EXTRALIBPATHS+=-L$(KDE_ROOT)$/lib64
.ENDIF
.ENDIF

# === KAB base library ==========================

# --- Files -------------------------------------

SLOFILES= \
    $(SLO)$/KDriver.obj     \
    $(SLO)$/KServices.obj

DEPOBJFILES= \
    $(SLO2FILES)

# --- Library -----------------------------------

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1TARGET= $(TARGET)$(KAB_MAJOR)
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

# === KAB impl library ==========================

# --- Files -------------------------------------

SLO2FILES=\
    $(SLO)$/KColumns.obj            \
    $(SLO)$/KTable.obj              \
    $(SLO)$/KTables.obj             \
    $(SLO)$/KCatalog.obj            \
    $(SLO)$/KResultSet.obj          \
    $(SLO)$/KStatement.obj          \
    $(SLO)$/KPreparedStatement.obj  \
    $(SLO)$/KDatabaseMetaData.obj   \
    $(SLO)$/KConnection.obj         \
    $(SLO)$/KResultSetMetaData.obj  \
    $(SLO)$/kcondition.obj          \
    $(SLO)$/korder.obj              \
    $(SLO)$/kfields.obj             \
    $(SLO)$/KDEInit.obj
 
KAB_LIB=$(KDE_LIBS) -lkabc

# --- Library -----------------------------------

SHL2VERSIONMAP=$(TARGET2).map

SHL2TARGET= $(TARGET2)$(KAB_MAJOR)
SHL2OBJS=$(SLO2FILES)
SHL2STDLIBS=\
    $(CPPULIB)                  \
    $(CPPUHELPERLIB)            \
    $(VOSLIB)                   \
    $(SALLIB)                   \
    $(DBTOOLSLIB)               \
    $(COMPHELPERLIB)            \
    $(KAB_LIB)

SHL2DEPN=
SHL2IMPLIB= i$(TARGET2)

SHL2DEF=    $(MISC)$/$(SHL2TARGET).def

DEF2NAME=   $(SHL2TARGET)

# --- Targets -----------------------------------
.ELSE		# "$(ENABLE_KAB)" == "TRUE"
dummy:
    @echo KDE Addressbook disabled - nothing to build
.ENDIF

.ELSE		# "$(GUI)" == "UNX"
dummy:
    @echo "Nothing to build for GUI $(GUI)"
.ENDIF

.INCLUDE : $(PRJ)$/target.pmk


ALLTAR : $(MISC)/kab1.component

$(MISC)/kab1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        kab1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt kab1.component
