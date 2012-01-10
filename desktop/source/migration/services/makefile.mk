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

PRJNAME=desktop
TARGET = migrationoo2.uno
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST = migrationoo2
LIBTARGET=NO

# --- Settings -----------------------------------------------------
.INCLUDE : ..$/..$/deployment/inc/dp_misc.mk
.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

.INCLUDE :  cppumaker.mk

SLOFILES= \
        $(SLO)$/jvmfwk.obj \
        $(SLO)$/cexports.obj \
        $(SLO)$/basicmigration.obj \
        $(SLO)$/wordbookmigration.obj \
        $(SLO)$/autocorrmigration.obj \
        $(SLO)$/oo3extensionmigration.obj \
        $(SLO)$/cexportsoo3.obj

SHL1OBJS= \
        $(SLO)$/jvmfwk.obj \
        $(SLO)$/cexports.obj \
        $(SLO)$/basicmigration.obj \
        $(SLO)$/wordbookmigration.obj \
        $(SLO)$/autocorrmigration.obj

SHL1TARGET=$(TARGET)
SHL1VERSIONMAP = $(SOLARENV)/src/component.map

SHL1STDLIBS= \
    $(DEPLOYMENTMISCLIB) \
    $(CPPULIB)		\
    $(CPPUHELPERLIB)	\
    $(SALLIB) \
    $(UCBHELPERLIB)	\
    $(UNOTOOLSLIB) \
    $(TOOLSLIB)	\
    $(I18NISOLANGLIB) \
    $(JVMFWKLIB) \
    $(XMLSCRIPTLIB) \

SHL1DEPN=
SHL1IMPLIB=imigrationoo2
#SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

COMP2TYPELIST = migrationoo3
SHL2TARGET=migrationoo3.uno
SHL2VERSIONMAP = $(SOLARENV)/src/component.map

SHL2OBJS= \
        $(SLO)$/cexportsoo3.obj \
        $(SLO)$/oo3extensionmigration.obj

SHL2STDLIBS= \
    $(DEPLOYMENTMISCLIB) \
    $(CPPULIB)		\
    $(CPPUHELPERLIB)	\
    $(SALLIB) \
    $(UCBHELPERLIB)	\
    $(UNOTOOLSLIB) \
    $(TOOLSLIB)	\
    $(I18NISOLANGLIB) \
    $(JVMFWKLIB) \
    $(XMLSCRIPTLIB) \

SHL2DEPN=
SHL2IMPLIB=imigrationoo3
#SHL2LIBS=$(SLB)$/$(SHL2TARGET).lib
SHL2DEF=$(MISC)$/$(SHL2TARGET).def

DEF2NAME=$(SHL2TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

ALLTAR : $(MISC)/migrationoo3.component

$(MISC)/migrationoo3.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt migrationoo3.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL2TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt migrationoo3.component

ALLTAR : $(MISC)/migrationoo2.component

$(MISC)/migrationoo2.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt migrationoo2.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt migrationoo2.component
