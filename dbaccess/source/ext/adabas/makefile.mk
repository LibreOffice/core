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

PRJNAME=dbaccess
TARGET=adabasui
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------
.INCLUDE :	settings.mk

# --- resource -----------------------------------------------------
SRS1NAME=$(TARGET)
SRC1FILES =	\
        AdabasNewDb.src

# --- defines ------------------------------------------------------
CDEFS+=-DCOMPMOD_NAMESPACE=adabasui

# --- Files -------------------------------------
SLOFILES=\
        $(SLO)$/ASqlNameEdit.obj		\
        $(SLO)$/AdabasNewDb.obj			\
        $(SLO)$/ANewDb.obj				\
        $(SLO)$/adabasuistrings.obj		\
        $(SLO)$/Aservices.obj			\
        $(SLO)$/Acomponentmodule.obj

# --- Library -----------------------------------
SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(UNOTOOLSLIB)				\
    $(SVTOOLLIB)				\
    $(TOOLSLIB)					\
    $(SVLLIB)					\
    $(VCLLIB)					\
    $(UCBHELPERLIB) 			\
    $(SFXLIB)					\
    $(TKLIB)					\
    $(COMPHELPERLIB)

SHL1IMPLIB=i$(SHL1TARGET)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(SLB)$/$(TARGET).lib
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(TARGET).dxp
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- .res file ----------------------------------------------------------
RES1FILELIST=\
    $(SRS)$/$(SRS1NAME).srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RES1FILELIST)


# --- Targets ----------------------------------

.INCLUDE : target.mk

ALLTAR : $(MISC)/adabasui.component

$(MISC)/adabasui.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        adabasui.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt adabasui.component
