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
TARGET=rpt
EXTENSION_VERSION_BASE=1.2.1

.IF "$(CWS_WORK_STAMP)" == ""
    EXTENSION_VERSION=$(EXTENSION_VERSION_BASE)
.ELSE
    EXTENSION_VERSION=$(EXTENSION_VERSION_BASE).cws.$(CWS_WORK_STAMP)
.ENDIF

# --- Settings ----------------------------------
.INCLUDE :  makefile.pmk

.IF "$(L10N_framework)"==""
# ------------------------------------------------------------------
# calready set in util$/makefile.pmk
# EXTENSIONNAME:=report-builder
EXTENSION_ZIPNAME:=report-builder

# create Extension -----------------------------

.IF "$(SOLAR_JAVA)"!=""

XMLFILES =  $(EXTENSIONDIR)$/META-INF$/manifest.xml

# DESCRIPTION_SRC is the source file which is copied into the extension
# It is defaulted to "description.xml", but we want to pre-process it, so we use an intermediate
# file
DESCRIPTION_SRC = $(MISC)$/description.xml

COMPONENT_MERGED_XCU= \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Setup.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/DataAccess.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/ReportDesign.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/ExtendedColorScheme.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/Embedding.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/Paths.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/Accelerators.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/TypeDetection$/Filter.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/TypeDetection$/Types.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/UI$/ReportCommands.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/UI$/Controller.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/UI$/DbReportWindowState.xcu \
            $(EXTENSIONDIR)$/registry$/schema$/org$/openoffice$/Office$/UI$/DbReportWindowState.xcs \
            $(EXTENSIONDIR)$/registry$/schema$/org$/openoffice$/Office$/UI$/ReportCommands.xcs \
            $(EXTENSIONDIR)$/registry$/schema$/org$/openoffice$/Office$/ReportDesign.xcs

COMPONENT_OTR_FILES= \
    $(EXTENSIONDIR)$/template$/en-US$/wizard$/report$/default.otr

COMPONENT_IMAGES= \
    $(EXTENSIONDIR)$/images$/extension_32.png \
    $(EXTENSIONDIR)$/images$/extension_32_h.png

COMPONENT_HTMLFILES = $(EXTENSIONDIR)$/readme_en-US.html \
            $(EXTENSIONDIR)$/readme_en-US.txt

COMPONENT_JARFILES = \
    $(EXTENSIONDIR)$/sun-report-builder.jar

COMPONENT_HELP= \
    $(EXTENSIONDIR)$/help$/component.txt

# .jar files from solver
COMPONENT_EXTJARFILES = \
    $(EXTENSIONDIR)$/sun-report-builder.jar 					\
    $(EXTENSIONDIR)$/reportbuilderwizard.jar

.IF "$(SYSTEM_JFREEREPORT)" != "YES"
.INCLUDE :  $(SOLARBINDIR)/jfreereport_version.mk
COMPONENT_EXTJARFILES += \
    $(EXTENSIONDIR)$/flute-$(FLUTE_VERSION).jar				            \
    $(EXTENSIONDIR)$/libserializer-$(LIBSERIALIZER_VERSION).jar			\
    $(EXTENSIONDIR)$/libbase-$(LIBBASE_VERSION).jar                     \
    $(EXTENSIONDIR)$/libfonts-$(LIBFONTS_VERSION).jar					\
    $(EXTENSIONDIR)$/libformula-$(LIBFORMULA_VERSION).jar						\
    $(EXTENSIONDIR)$/liblayout-$(LIBLAYOUT_VERSION).jar					\
    $(EXTENSIONDIR)$/libloader-$(LIBLOADER_VERSION).jar					\
    $(EXTENSIONDIR)$/librepository-$(LIBREPOSITORY_VERSION).jar			\
    $(EXTENSIONDIR)$/libxml-$(LIBXML_VERSION).jar						\
    $(EXTENSIONDIR)$/flow-engine-$(FLOW_ENGINE_VERSION).jar 	        \
    $(EXTENSIONDIR)$/sac.jar
.ENDIF
.IF "$(SYSTEM_APACHE_COMMONS)" != "YES"
COMPONENT_EXTJARFILES += \
    $(EXTENSIONDIR)$/commons-logging-1.1.3.jar
.ENDIF

COMPONENT_MANIFEST_GENERIC:=TRUE
COMPONENT_MANIFEST_SEARCHDIR:=registry

# make sure to add your custom files here
EXTENSION_PACKDEPS=$(COMPONENT_EXTJARFILES) $(COMPONENT_HTMLFILES) $(COMPONENT_OTR_FILES) $(COMPONENT_HELP) $(COMPONENT_IMAGES)
.ENDIF
# --- Targets ----------------------------------

.INCLUDE : extension_pre.mk
.INCLUDE : target.mk
.INCLUDE : extension_post.mk
.IF "$(L10N_framework)"==""
$(EXTENSIONDIR)$/%.jar : $(SOLARBINDIR)$/%.jar
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(EXTENSIONDIR)$/readme_en-US.% : $(PRJ)$/license$/readme_en-US.%
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(COMPONENT_IMAGES) : $(SOLARSRC)$/$(RSCDEFIMG)$/desktop$/res$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@
    
$(COMPONENT_HELP) : $$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(DESCRIPTION_SRC): description.xml
    +-$(RM) $@
    $(TYPE) description.xml | $(SED) "s/#VERSION#/$(EXTENSION_VERSION)/" > $@
.ENDIF
.ELSE			# "$(SOLAR_JAVA)"!=""
.INCLUDE : target.mk
.ENDIF			# "$(SOLAR_JAVA)"!=""
#
