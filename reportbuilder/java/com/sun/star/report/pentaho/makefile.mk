#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJ	= ..$/..$/..$/..$/..$/..
PRJNAME = reportbuilder
TARGET=sun-report-builder
PACKAGE = com$/sun$/star$/report$/pentaho
# we have circular dependencies
nodep=true
# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE :  $(SOLARBINDIR)/jfreereport_version.mk

#----- compile .java files -----------------------------------------
JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar
.IF "$(SYSTEM_JFREEREPORT)" == "YES"
EXTRAJARFILES += $(FLUTE_JAR) $(LIBBASE_JAR) $(LIBXML_JAR) $(JFREEREPORT_JAR) $(LIBLOADER_JAR) $(SAC_JAR) $(LIBLAYOUT_JAR) $(LIBSERIALIZER_JAR) $(LIBFONTS_JAR) $(LIBFORMULA_JAR) $(LIBREPOSITORY_JAR)
.ELSE
JARFILES += \
    flute-$(FLUTE_VERSION).jar				            \
    libserializer-$(LIBSERIALIZER_VERSION).jar			\
    libbase-$(LIBBASE_VERSION).jar                      \
    libfonts-$(LIBFONTS_VERSION).jar					\
    libformula-$(LIBFORMULA_VERSION).jar				\
    liblayout-$(LIBLAYOUT_VERSION).jar					\
    libloader-$(LIBLOADER_VERSION).jar					\
    librepository-$(LIBREPOSITORY_VERSION).jar			\
    libxml-$(LIBXML_VERSION).jar						\
    flow-engine-$(FLOW_ENGINE_VERSION).jar 	            \
    sac.jar
.ENDIF

.IF "$(SYSTEM_APACHE_COMMONS)" == "YES"
EXTRAJARFILES += $(COMMONS_LOGGING_JAR)
.ELSE
JARFILES += commons-logging-1.1.1.jar
.ENDIF

.IF "$(SYSTEM_JFREEREPORT)" == "YES" || "$(SYSTEM_APACHE_COMMONS)" == "YES"
COMP=fix_system_libs
.ENDIF

JAVAFILES       := $(shell @$(FIND) . -name "*.java")
# JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:s/.java/.class/))

#----- copy .properties and .css files -----------------------------------------
# external find gives / for 4nt too...
# PROPERTYFILES := $(subst,/,$/ $(subst,./,$(CLASSDIR)$/$(PACKAGE)$/ $(shell @$(FIND) . -name "*.properties")))
PROPERTYFILES := $(CLASSDIR)$/$(PACKAGE)$/configuration.properties \
                 $(CLASSDIR)$/$(PACKAGE)$/module.properties \
                 $(CLASSDIR)$/$(PACKAGE)$/parser$/selectors.properties
                 
# CSSFILES := $(subst,./,$(CLASSDIR)$/$(PACKAGE)$/ $(shell @$(FIND) . -name "*.css"))
CSSFILES := $(CLASSDIR)$/$(PACKAGE)$/oasis-datastyle.css\
            $(CLASSDIR)$/$(PACKAGE)$/oasis-draw.css\
            $(CLASSDIR)$/$(PACKAGE)$/oasis-form.css\
            $(CLASSDIR)$/$(PACKAGE)$/oasis-style.css\
            $(CLASSDIR)$/$(PACKAGE)$/oasis-table.css\
            $(CLASSDIR)$/$(PACKAGE)$/oasis-text.css\
            $(CLASSDIR)$/$(PACKAGE)$/smil.css\
            $(CLASSDIR)$/$(PACKAGE)$/star-office.css\
            $(CLASSDIR)$/$(PACKAGE)$/star-report.css\
            $(CLASSDIR)$/$(PACKAGE)$/star-rpt.css\
            $(CLASSDIR)$/$(PACKAGE)$/svg.css\
            $(CLASSDIR)$/$(PACKAGE)$/xsl-fo.css

XSDFILES := $(CLASSDIR)$/$(PACKAGE)$/parser$/rpt-schema-v1.0-os.xsd \
            $(CLASSDIR)$/$(PACKAGE)$/styles$/stylemapper.xsd
            
XMLFILES := $(CLASSDIR)$/$(PACKAGE)$/styles$/stylemapper.xml
            
TXTFILES := $(CLASSDIR)$/$(PACKAGE)$/parser$/style-mapping.txt

            
#----- make a jar from compiled files ------------------------------

JARCLASSDIRS    = .
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE
CUSTOMMANIFESTFILE = Manifest.mf


# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

.IF "$(JARTARGETN)"!=""
$(JARTARGETN) : $(COMP) $(PROPERTYFILES) $(CSSFILES) $(XSDFILES) $(TXTFILES) $(XMLFILES)
.ENDIF          # "$(JARTARGETN)"!=""

fix_system_libs: $(JARMANIFEST)
    @echo "Fix Java Class-Path entry for libraries from system."
.IF ("$(SYSTEM_JFREEREPORT)" != "YES" && "$(SYSTEM_APACHE_COMMONS)" == "YES")
    @$(SED) -r -e "s#commons-logging-1.1.1.jar#file://$(COMMONS_LOGGING_JAR)#" \
        -i $<
.ENDIF
.IF ("$(SYSTEM_JFREEREPORT)" == "YES" && "$(SYSTEM_APACHE_COMMONS)" == "YES")
    @$(SED) '/flute/,/sac/d' -i ../../../../../../$(INPATH)/class/sun-report-builder/META-INF/MANIFEST.MF
    @$(SED) -r -e "s#^Class-Path.*#\0\n  file://$(LIBBASE_JAR)\n  file://$(SAC_JAR)\n  file://$(LIBXML_JAR)\n\
  file://$(FLUTE_JAR)\n  file://$(JFREEREPORT_JAR)\n  file://$(LIBLAYOUT_JAR)\n  file://$(LIBLOADER_JAR)\n  file://$(LIBFORMULA_JAR)\n\
  file://$(LIBREPOSITORY_JAR)\n  file://$(LIBFONTS_JAR)\n  file://$(LIBSERIALIZER_JAR)\n  file://$(COMMONS_LOGGING_JAR)#" \
    -i $<
.ENDIF

$(CLASSDIR)$/$(PACKAGE)$/%.properties : %.properties
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(CLASSDIR)$/$(PACKAGE)$/%.css : %.css
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(CLASSDIR)$/$(PACKAGE)$/%.xsd : %.xsd
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@
    
$(CLASSDIR)$/$(PACKAGE)$/%.xml : %.xml
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@    

$(CLASSDIR)$/$(PACKAGE)$/%.txt : %.txt
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@
    
