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
TARGET=evoab2
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

.IF "$(ENABLE_EVOAB2)"!="TRUE"
dummy:
    @echo "Evolution 2.x Addressbook build disabled"
.ELSE

# --- Settings ----------------------------------

.INCLUDE : $(PRJ)$/makefile.pmk
.INCLUDE :  $(PRJ)$/version.mk

PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE : pkg_config.mk

CFLAGS+=$(GOBJECT_CFLAGS)

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/NDriver.obj \
    $(SLO)$/NTable.obj \
    $(SLO)$/NColumns.obj \
    $(SLO)$/NTables.obj \
    $(SLO)$/NCatalog.obj \
    $(SLO)$/NConnection.obj \
    $(SLO)$/NDatabaseMetaData.obj \
    $(SLO)$/NStatement.obj \
    $(SLO)$/NPreparedStatement.obj \
    $(SLO)$/NServices.obj \
    $(SLO)$/NResultSet.obj  \
    $(SLO)$/NResultSetMetaData.obj \
    $(SLO)$/EApi.obj \
    $(SLO)$/NDebug.obj

SHL1VERSIONMAP=$(SOLARENV)/src/component.map


# --- Library -----------------------------------
#SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1TARGET=	$(EVOAB_TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SVLLIB)					\
    $(TOOLSLIB)					\
    $(UNOTOOLSLIB)				\
    $(UCBHELPERLIB)				\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(DBFILELIB)				\
    $(COMPHELPERLIB)

.IF "$(DBFILELIB)" == ""
SHL1STDLIBS+= ifile.lib
.ENDIF
SHL1STDLIBS+=$(GOBJECT_LIBS)


SHL1STDLIBS+=$(PKGCONFIG_LIBS:s/ -lpangoxft-1.0//)
# hack for faked SO environment
.IF "$(PKGCONFIG_ROOT)"!=""
SHL1SONAME+=-z nodefs
SHL1NOCHECK=TRUE
.ENDIF          # "$(PKGCONFIG_ROOT)"!=""


SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)

.ENDIF
# --- Targets ----------------------------------

.INCLUDE : $(PRJ)$/target.pmk

ALLTAR : $(MISC)/evoab.component

$(MISC)/evoab.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        evoab.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt evoab.component
