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
# $Revision: 1.7.58.2 $
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

#----- compile .java files -----------------------------------------
JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar
.IF "$(SYSTEM_JFREEREPORT)" == "YES"
XCLASSPATH!:=$(XCLASSPATH)$(FLUTE_JAR)$(PATH_SEPERATOR)$(LIBBASE_JAR)$(PATH_SEPERATOR)$(LIBXML_JAR)$(PATH_SEPERATOR)$(JFREEREPORT_JAR)$(PATH_SEPERATOR)$(LIBLOADER_JAR)$(PATH_SEPERATOR)$(SAC_JAR)$(PATH_SEPERATOR)$(LIBLAYOUT_JAR)$(PATH_SEPERATOR)$(LIBSERIALIZER_JAR)$(PATH_SEPERATOR)$(LIBFONTS_JAR)$(PATH_SEPERATOR)$(LIBFORMULA_JAR)$(PATH_SEPERATOR)$(LIBREPOSITORY_JAR)$(PATH_SEPERATOR)$(COMMONS_LOGGING_JAR)
COMP=fix_system_jfreereport
.ELSE
JARFILES += flute-1.3.0.jar libbase-1.0.0.jar libfonts-1.0.0.jar libformula-0.2.0.jar liblayout-0.2.9.jar libloader-1.0.0.jar librepository-1.0.0.jar libxml-1.0.0.jar flow-engine-0.9.2.jar sac.jar commons-logging-1.1.1.jar
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

fix_system_jfreereport:
    @echo "Fix Java Class-Path entry for JFree JFreeReport libraries from system."
    @$(SED) '/flute/,/sac/d' -i ../../../../../../$(INPATH)/class/sun-report-builder/META-INF/MANIFEST.MF
    @$(SED) -r -e "s#^Class-Path.*#\0\n  file://$(LIBBASE_JAR)\n  file://$(SAC_JAR)\n  file://$(LIBXML_JAR)\n\
  file://$(FLUTE_JAR)\n  file://$(JFREEREPORT_JAR)\n  file://$(LIBLAYOUT_JAR)\n  file://$(LIBLOADER_JAR)\n  file://$(LIBFORMULA_JAR)\n\
  file://$(LIBREPOSITORY_JAR)\n  file://$(LIBFONTS_JAR)\n  file://$(LIBSERIALIZER_JAR)#" \
    -i ../../../../../../$(INPATH)/class/sun-report-builder/META-INF/MANIFEST.MF

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
    
