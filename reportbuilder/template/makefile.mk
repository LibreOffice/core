#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.2 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..
PRJNAME=reportbuilder
TARGET=rpt_templates

# ???
no_common_build_zip=true

EXTENSION_VERSION_BASE=1.0.0
.IF "$(PRODUCT)" != ""
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
