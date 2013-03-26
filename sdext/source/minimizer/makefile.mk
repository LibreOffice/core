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
TARGET=SunPresentationMinimizer
GEN_HID=FALSE
ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.INCLUDE : settings.mk
.IF "$(L10N_framework)"==""
.INCLUDE :  $(PRJ)$/source$/minimizer$/minimizer.pmk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# set in minimizer.pmk
#EXTENSIONNAME:=PresentationMinimizer
EXTENSION_ZIPNAME:=presentation-minimizer

.IF "$(ENABLE_MINIMIZER)" != "YES"
@all:
    @echo "Presentation Minimizer build disabled."
.ENDIF

DLLPRE=

# --- Files -------------------------------------

SLOFILES=	$(SLO)$/unodialog.obj				\
            $(SLO)$/optimizerdialog.obj			\
            $(SLO)$/optimizerdialogcontrols.obj	\
            $(SLO)$/configurationaccess.obj		\
            $(SLO)$/impoptimizer.obj			\
            $(SLO)$/pppoptimizer.obj			\
            $(SLO)$/pppoptimizeruno.obj			\
            $(SLO)$/pppoptimizertoken.obj		\
            $(SLO)$/pppoptimizerdialog.obj		\
            $(SLO)$/fileopendialog.obj			\
            $(SLO)$/optimizationstats.obj		\
            $(SLO)$/graphiccollector.obj		\
            $(SLO)$/pagecollector.obj			\
            $(SLO)$/informationdialog.obj

# --- Library -----------------------------------

SHL1TARGET=		$(TARGET).uno

SHL1STDLIBS=	$(CPPUHELPERLIB)	\
                $(CPPULIB)			\
                $(SALLIB)
SHL1DEPN=
SHL1IMPLIB=		i$(SHL1TARGET)
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP=$(SOLARENV)/src/reg-component.map
SHL1RPATH=      OXT
DEF1NAME=		$(SHL1TARGET)

COMPONENT_XCU_REPLACEMENT=s/@MINIMIZEREXTENSIONPRODUCTNAME@/Presentation Minimizer/g

COMPONENT_MERGED_XCU= \
    $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/Addons.xcu \
    $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/ProtocolHandler.xcu \
    $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/extension$/SunPresentationMinimizer.xcu

COMPONENT_FILES= \
    $(EXTENSIONDIR)$/registry$/schema$/org$/openoffice$/Office$/extension$/SunPresentationMinimizer.xcs

# native libraries
COMPONENT_LIBRARIES= \
    $(EXTENSIONDIR)$/SunPresentationMinimizer.uno$(DLLPOST)

# rather freestyle or common to all?
COMPONENT_BITMAPS= \
    $(EXTENSIONDIR)$/bitmaps$/opt_16.png \
    $(EXTENSIONDIR)$/bitmaps$/opt_26.png \
    $(EXTENSIONDIR)$/bitmaps$/opt_16_h.png \
    $(EXTENSIONDIR)$/bitmaps$/opt_26_h.png \
    $(EXTENSIONDIR)$/bitmaps$/minimizepresi_80.png \
    $(EXTENSIONDIR)$/bitmaps$/minimizepresi_80_h.png

COMPONENT_IMAGES=\
    $(EXTENSIONDIR)$/bitmaps$/extension_32.png \
    $(EXTENSIONDIR)$/bitmaps$/extension_32_h.png

# rather freestyle or common to all?
COMPONENT_HELP= \
    $(EXTENSIONDIR)$/help$/help_de.odt \
    $(EXTENSIONDIR)$/help$/help_en-US.odt \
    $(EXTENSIONDIR)$/help$/component.txt

# make sure to add your custom files here
EXTENSION_PACKDEPS=$(COMPONENT_BITMAPS) $(COMPONENT_IMAGES) $(COMPONENT_HELP)

ZIP2TARGET=     presentation_minimizer_develop
.IF "$(WITH_LANG)"!=""
ZIP2DIR=        $(MISC)/$(EXTENSIONNAME)_in/merge
.ELSE          # "$(WITH_LANG)"!=""
ZIP2DIR=        registry/data
.ENDIF          # "$(WITH_LANG)"!=""
ZIP2EXT=        .zip
ZIP2FLAGS=-r
ZIP2LIST= \
    org/openoffice/Office/Addons.xcu \
    org/openoffice/Office/extension/*.xcu

# --- Targets ----------------------------------

.INCLUDE : extension_pre.mk

.ENDIF # L10N_framework
.INCLUDE : target.mk

.IF "$(L10N_framework)"==""
.INCLUDE : extension_post.mk

$(COMPONENT_BITMAPS) : $(SOLARSRC)$/$(RSCDEFIMG)$/minimizer$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(COMPONENT_IMAGES) : $(SOLARSRC)$/$(RSCDEFIMG)$/desktop$/res$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(COMPONENT_HELP) : help$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

.ENDIF # L10N_framework
