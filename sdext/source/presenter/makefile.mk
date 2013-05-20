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
TARGET=PresenterScreen
GEN_HID=FALSE
EXTNAME=PresenterScreen

ENABLE_EXCEPTIONS=TRUE

PACKAGE=org.openoffice.Office
ABSXCSROOT=$(SOLARXMLDIR)
XCSROOT=..
DTDDIR=$(ABSXCSROOT)
XSLDIR=$(ABSXCSROOT)$/processing
PROCESSOUT=$(MISC)$/$(TARGET)
PROCESSORDIR=$(SOLARBINDIR)

# --- Settings ----------------------------------

.INCLUDE : settings.mk

.IF "$(L10N_framework)" != ""

@all:
    @echo "L10N framework disabled => Presenter Screen can not be built."

.ELSE

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

DLLPRE=

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/components.obj \
    $(SLO)$/PresenterAccessibility.obj \
    $(SLO)$/PresenterAnimation.obj \
    $(SLO)$/PresenterAnimator.obj \
    $(SLO)$/PresenterBitmapContainer.obj \
    $(SLO)$/PresenterButton.obj \
    $(SLO)$/PresenterCanvasHelper.obj \
    $(SLO)$/PresenterConfigurationAccess.obj \
    $(SLO)$/PresenterController.obj \
    $(SLO)$/PresenterCurrentSlideObserver.obj \
    $(SLO)$/PresenterFrameworkObserver.obj \
    $(SLO)$/PresenterGeometryHelper.obj \
    $(SLO)$/PresenterHelper.obj \
    $(SLO)$/PresenterHelpView.obj \
    $(SLO)$/PresenterNotesView.obj \
    $(SLO)$/PresenterPaintManager.obj \
    $(SLO)$/PresenterPane.obj \
    $(SLO)$/PresenterPaneAnimator.obj \
    $(SLO)$/PresenterPaneBase.obj \
    $(SLO)$/PresenterPaneBorderManager.obj \
    $(SLO)$/PresenterPaneBorderPainter.obj \
    $(SLO)$/PresenterPaneContainer.obj \
    $(SLO)$/PresenterPaneFactory.obj \
    $(SLO)$/PresenterProtocolHandler.obj \
    $(SLO)$/PresenterScreen.obj \
    $(SLO)$/PresenterScrollBar.obj \
    $(SLO)$/PresenterSlidePreview.obj \
    $(SLO)$/PresenterSlideShowView.obj \
    $(SLO)$/PresenterSlideSorter.obj \
    $(SLO)$/PresenterSprite.obj \
    $(SLO)$/PresenterSpritePane.obj \
    $(SLO)$/PresenterTextView.obj \
    $(SLO)$/PresenterTheme.obj \
    $(SLO)$/PresenterTimer.obj \
    $(SLO)$/PresenterToolBar.obj \
    $(SLO)$/PresenterUIPainter.obj \
    $(SLO)$/PresenterViewFactory.obj \
    $(SLO)$/PresenterWindowManager.obj \


# --- Library -----------------------------------

.IF "$(GUI)"=="OS2"
SHL1TARGET=PreScr
.ELSE
SHL1TARGET=$(TARGET).uno
.ENDIF

SHL1STDLIBS= \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)

SHL1DEPN=
SHL1IMPLIB=    i$(SHL1TARGET)
SHL1LIBS=    $(SLB)$/$(TARGET).lib
SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1RPATH=      OXT
DEF1NAME=    $(SHL1TARGET)

# ------------------------------------------------------------------------------

# Resource files

SRS1NAME=$(TARGET)
SRC1FILES =  \
        $(TARGET).src \

SRSFILELIST=\
        $(SRS)$/$(TARGET).srs \

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES= $(SRSFILELIST)


PACKAGEDIR=$(subst,.,$/ $(PACKAGE))
SPOOLDIR=$(MISC)$/registry$/spool

XCUFILES= \
    Jobs.xcu \
    ProtocolHandler.xcu

MYXCUFILES= \
    $(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-presenterscreen.xcu \
    $(SPOOLDIR)$/$(PACKAGEDIR)$/ProtocolHandler/ProtocolHandler-presenterscreen.xcu


# --- Targets ----------------------------------

.INCLUDE : target.mk

ALLTAR : $(MYXCUFILES)

XCU_SOURCEDIR:=.

$(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-presenterscreen.xcu : $(XCU_SOURCEDIR)$/Jobs.xcu
    @-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

$(SPOOLDIR)$/$(PACKAGEDIR)$/ProtocolHandler$/ProtocolHandler-presenterscreen.xcu : $(XCU_SOURCEDIR)$/ProtocolHandler.xcu
    @-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

ALLTAR : $(MISC)/PresenterScreen.component

$(MISC)/PresenterScreen.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt PresenterScreen.component
    $(XSLTPROC) --nonet --stringparam uri \
    '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
    $(SOLARENV)/bin/createcomponent.xslt PresenterScreen.component

.ENDIF # L10N_framework
