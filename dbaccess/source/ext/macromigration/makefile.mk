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
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=dbmm
USE_DEFFILE=TRUE

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Files -------------------------------------

# ... resource files ............................

SRS1NAME=$(TARGET)
SRC1FILES =	\
        macromigration.src

# ... object files ............................
SLOFILES=   $(SLO)$/macromigrationwizard.obj \
            $(SLO)$/macromigrationdialog.obj \
            $(SLO)$/macromigrationpages.obj \
            $(SLO)$/dbmm_module.obj \
            $(SLO)$/dbmm_services.obj \
            $(SLO)$/migrationengine.obj \
            $(SLO)$/docinteraction.obj \
            $(SLO)$/progresscapture.obj \
            $(SLO)$/progressmixer.obj \
            $(SLO)$/migrationlog.obj \
            $(SLO)$/dbmm_types.obj \

# --- library -----------------------------------

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS= \
        $(CPPULIB)          \
        $(CPPUHELPERLIB)    \
        $(COMPHELPERLIB)    \
        $(UNOTOOLSLIB)      \
        $(TOOLSLIB)         \
        $(SALLIB)           \
        $(SVTOOLLIB)        \
        $(SVLLIB)           \
        $(VCLLIB)           \
        $(SVXCORELIB)           \
        $(SVXLIB)           \
        $(UCBHELPERLIB)     \
        $(XMLSCRIPTLIB)

SHL1LIBS=       $(SLB)$/$(TARGET).lib
SHL1IMPLIB=     i$(TARGET)
SHL1DEPN=       $(SHL1LIBS)
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def

DEF1NAME=       $(SHL1TARGET)

# --- .res files -------------------------------

RES1FILELIST=\
    $(SRS)$/$(TARGET).srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RES1FILELIST)

# --- Targets ----------------------------------

.INCLUDE : target.mk


ALLTAR : $(MISC)/dbmm.component

$(MISC)/dbmm.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        dbmm.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt dbmm.component
