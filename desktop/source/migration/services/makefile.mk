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
.IF "$(OS)" == "OS2"
TARGET = migroo2
.ELSE
TARGET = migrationoo2.uno
.ENDIF
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST = migrationoo2
LIBTARGET=NO

# --- Settings -----------------------------------------------------
PRJINC=..$/..
.INCLUDE :  settings.mk
.INCLUDE : ..$/..$/deployment/inc/dp_misc.mk
.INCLUDE : ..$/..$/deployment/inc/dp_gui.mk
DLLPRE =

# ------------------------------------------------------------------

.INCLUDE :  cppumaker.mk

SLOFILES= \
        $(SLO)$/jvmfwk.obj \
        $(SLO)$/cexports.obj \
        $(SLO)$/basicmigration.obj \
        $(SLO)$/wordbookmigration.obj \
        $(SLO)$/autocorrmigration.obj \
        $(SLO)$/oo3extensionmigration.obj

SHL1OBJS= \
        $(SLO)$/jvmfwk.obj \
        $(SLO)$/cexports.obj \
        $(SLO)$/basicmigration.obj \
        $(SLO)$/wordbookmigration.obj \
        $(SLO)$/autocorrmigration.obj \
        $(SLO)$/oo3extensionmigration.obj

SHL1TARGET=$(TARGET)
SHL1VERSIONMAP = $(SOLARENV)/src/component.map

SHL1STDLIBS= \
    $(DEPLOYMENTMISCLIB) \
    $(DEPLOYMENTGUILIB) \
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

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

ALLTAR : $(MISC)/migrationoo2.component

$(MISC)/migrationoo2.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt migrationoo2.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt migrationoo2.component
