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



PRJ=..
PRJNAME=reportbuilder
TARGET=rpt_templates

# ???
no_common_build_zip=true

EXTENSION_VERSION_BASE=1.0.0
.IF "$(DBG_LEVEL)" == "0"
    EXTENSION_VERSION=$(EXTENSION_VERSION_BASE)
.ELSE
    EXTENSION_VERSION=$(EXTENSION_VERSION_BASE).$(BUILD)
.ENDIF

# --- Settings ----------------------------------
.INCLUDE :  settings.mk
# ------------------------------------------------------------------

# create Extension -----------------------------

.IF "$(SOLAR_JAVA)"!=""

ZIP1TARGET=sun-report-builder-templates
ZIP1FLAGS=-r
ZIP1DIR=$(MISC)$/templates
ZIP1LIST=*
ZIP1EXT=.oxt

XMLFILES := $(ZIP1DIR)$/description.xml \
            $(ZIP1DIR)$/META-INF$/manifest.xml

XCU_FILES := $(ZIP1DIR)$/registry$/data$/org$/openoffice$/Office$/Paths.xcu

ODR_FILES := \
    $(ZIP1DIR)$/template$/wizard$/report$/finance.odr \
    $(ZIP1DIR)$/template$/wizard$/report$/finance_2.odr \
    $(ZIP1DIR)$/template$/wizard$/report$/yellow.odr

HTMLFILES := $(ZIP1DIR)$/THIRDPARTYREADMELICENSE.html \
            $(ZIP1DIR)$/readme_en-US.html \
            $(ZIP1DIR)$/readme_en-US.txt

.IF "$(GUI)"!="WNT"
TXTFILES:=$(foreach,i,$(alllangiso) $(ZIP1DIR)$/registration$/LICENSE_$i)
LICLINES:=$(foreach,i,$(TXTFILES)  <license-text xlink:href="registration/$(i:f)" lang="$(subst,LICENSE_, $(i:f))" license-id="$(subst,LICENSE_, $(i:f))" />)
.ELSE   # "$(GUI)"!="WNT"
TXTFILES:=$(foreach,i,$(alllangiso) $(ZIP1DIR)$/registration$/license_$i.txt)
LICLINES:=$(foreach,i,$(TXTFILES)  <license-text xlink:href="registration/$(i:f)" lang="$(subst,.txt, $(subst,license_, $(i:f)))" license-id="$(subst,.txt, $(subst,license_, $(i:f)))" />)
.ENDIF  # "$(GUI)"!="WNT"

# --- Targets ----------------------------------
.INCLUDE : target.mk

.IF "$(ZIP1TARGETN)"!=""
$(ZIP1TARGETN) :  $(TXTFILES) $(XMLFILES) $(HTMLFILES) $(ODR_FILES)
.ENDIF          # "$(ZIP1TARGETN)"!="

$(MISC)$/template_update_report.flag : $(XCU_FILES)
    $(TOUCH) $@

$(ZIP1DIR)$/description.xml : pre.xml post.xml
    @@-$(MKDIRHIER) $(@:d)
    @@-$(RM) $(ZIP1DIR)$/description.xml
    $(TYPE) pre.xml > $@
    $(TYPE) $(mktmp  $(LICLINES)) >> $@
    $(TYPE) post.xml | $(SED) s/#VERSION#/$(EXTENSION_VERSION)/ >> $@

$(ZIP1DIR)$/registration$/license_%.txt : $(SOLARBINDIR)$/osl$/license_%.txt
     @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(ZIP1DIR)$/registration$/LICENSE_% : $(SOLARBINDIR)$/osl$/LICENSE_%
     @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(ZIP1DIR)$/registry$/data$/org$/openoffice$/Office$/%.xcu : $(MISC)$/template$/registry$/data$/org$/openoffice$/Office$/%.xcu
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(ZIP1DIR)$/META-INF$/manifest.xml : manifest.xml $(MISC)$/template_update_report.flag
    @@-$(MKDIRHIER) $(@:d)
    $(PERL) $(SOLARENV)$/bin$/makemani.pl $(PRJ)$/template$/manifest.xml $(ZIP1DIR) registry $(@:d:d)

$(ZIP1DIR)$/readme_en-US.% : $(PRJ)$/license$/readme_en-US.%
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(ZIP1DIR)$/THIRDPARTYREADMELICENSE.html : $(PRJ)$/license$/THIRDPARTYREADMELICENSE.html
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

.ELSE			# "$(SOLAR_JAVA)"!=""
.INCLUDE : target.mk
.ENDIF			# "$(SOLAR_JAVA)"!=""
#
