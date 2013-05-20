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

PRJ=..$/..
PRJNAME=sdext
TARGET=PresentationMinimizer
EXTENSIONNAME=$(TARGET)

GEN_HID=FALSE
ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.INCLUDE : settings.mk

.IF "$(L10N_framework)"==""


DLLPRE=

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/unodialog.obj \
    $(SLO)$/optimizerdialog.obj \
    $(SLO)$/informationdialog.obj \
    $(SLO)$/fileopendialog.obj \
    $(SLO)$/optimizerdialogcontrols.obj \
    $(SLO)$/configurationaccess.obj \
    $(SLO)$/impoptimizer.obj \
    $(SLO)$/pppoptimizer.obj \
    $(SLO)$/pppoptimizeruno.obj \
    $(SLO)$/pppoptimizertoken.obj \
    $(SLO)$/pppoptimizerdialog.obj \
    $(SLO)$/optimizationstats.obj \
    $(SLO)$/graphiccollector.obj \
    $(SLO)$/pagecollector.obj \

# --- Library -----------------------------------

.IF "$(GUI)"=="OS2"
SHL1TARGET=PreMin
.ELSE
SHL1TARGET= $(TARGET).uno
.ENDIF

SHL1STDLIBS=\
    $(CPPUHELPERLIB)\
    $(CPPULIB)\
    $(SALLIB)
SHL1DEPN=
SHL1IMPLIB=i$(SHL1TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1RPATH=OXT
DEF1NAME=$(SHL1TARGET)

# ------------------------------------------------------------------------------

# Resource files

SRS1NAME=minimizer
SRC1FILES =  \
        minimizer.src \

SRSFILELIST=\
        $(SRS)$/minimizer.srs \

RESLIB1NAME=minimizer
RESLIB1SRSFILES= $(SRSFILELIST)

# --- Targets ----------------------------------

.ENDIF # L10N_framework
.INCLUDE : target.mk


ALLTAR : $(MISC)/PresentationMinimizer.component

$(MISC)/PresentationMinimizer.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt PresentationMinimizer.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt PresentationMinimizer.component
