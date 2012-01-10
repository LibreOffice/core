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

  
PRJ	= ..$/.. 
PRJNAME = filter 
#PACKAGE = com$/sun$/star$/documentconversion$/XSLTValidate
TARGET  =XSLTValidate 
# --- Settings ----------------------------------------------------- 
.IF "$(XML_CLASSPATH)" != ""
XCLASSPATH+=":$(XML_CLASSPATH)"
.ENDIF
.INCLUDE: settings.mk  
CLASSDIR!:=$(CLASSDIR)$/$(TARGET)
#USE_UDK_EXTENDED_MANIFESTFILE=TRUE
#USE_EXTENDED_MANIFESTFILE=TRUE
JARFILES 		= ridl.jar unoil.jar jurt.jar juh.jar crimson.jar

.IF "$(SYSTEM_XALAN)" == "YES"
EXTRAJARFILES += $(XALAN_JAR)
.ELSE
JARFILES += xalan.jar
.ENDIF

.IF "$(SYSTEM_XML_APIS)" == "YES"
EXTRAJARFILES += $(XML_APIS_JAR)
.ELSE
JARFILES += xml-apis.jar
.ENDIF

JAVAFILES		= $(subst,$(CLASSDIR)$/, $(subst,.class,.java $(JAVACLASSFILES))) 
CUSTOMMANIFESTFILE = Manifest  
#JARMANIFEST = Manifest
JARCOMPRESS		= TRUE  
JARCLASSDIRS	= XSLTValidate*.class
JARTARGET		= $(TARGET).jar


# --- Files --------------------------------------------------------  
JAVACLASSFILES=$(CLASSDIR)$/XSLTValidate.class  
#---Manifest -------------------------------------------------------
#$(OUT)$/class$/$(TARGET)$/META-INF: META-INF
#        + $(COPY) $(COPYRECURSE) META-INF $(OUT)$/class$/META-INF
# --- Targets ------------------------------------------------------  
.IF "$(SOLAR_JAVA)"!=""
.INCLUDE :  target.mk 
$(JAVACLASSFILES) : $(CLASSDIR)
.IF "$(JARMANIFEST)"!=""
$(JARMANIFEST) : $(CLASSDIR)
.ENDIF			# "$(JARMANIFEST)"!=""
.ENDIF
  
$(CLASSDIR) :
    $(MKDIR) $(CLASSDIR)

ALLTAR : $(MISC)/XSLTValidate.component

$(MISC)/XSLTValidate.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt XSLTValidate.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_JAVA)$(JARTARGET)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt XSLTValidate.component
