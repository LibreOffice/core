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
EXTRAJARFILES += $(FLUTE_JAR) $(LIBBASE_JAR) $(LIBXML_JAR) $(JFREEREPORT_JAR) $(LIBLOADER_JAR) $(SAC_JAR) $(LIBLAYOUT_JAR) $(LIBSERIALIZER_JAR) $(LIBFONTS_JAR) $(LIBFORMULA_JAR) $(LIBREPOSITORY_JAR)
.ELSE
.INCLUDE :  $(SOLARBINDIR)/jfreereport_version.mk
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
JARFILES += commons-logging-1.1.3.jar
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
.ELIF ("$(SYSTEM_JFREEREPORT)" == "YES" && "$(SYSTEM_APACHE_COMMONS)" != "YES")
    @$(SED) '/flute/,/sac/d' -i ../../../../../../$(INPATH)/class/sun-report-builder/META-INF/MANIFEST.MF
    @$(SED) -r -e "s#^Class-Path.*#\0\n  file://$(LIBBASE_JAR)\n  file://$(SAC_JAR)\n  file://$(LIBXML_JAR)\n\
  file://$(FLUTE_JAR)\n  file://$(JFREEREPORT_JAR)\n  file://$(LIBLAYOUT_JAR)\n  file://$(LIBLOADER_JAR)\n  file://$(LIBFORMULA_JAR)\n\
  file://$(LIBREPOSITORY_JAR)\n  file://$(LIBFONTS_JAR)\n  file://$(LIBSERIALIZER_JAR)\n  commons-logging-1.1.1.jar#" \
    -i $<
.ELIF ("$(SYSTEM_JFREEREPORT)" == "YES" && "$(SYSTEM_APACHE_COMMONS)" == "YES")
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
    
