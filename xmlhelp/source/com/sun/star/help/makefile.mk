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



PRJ		= ..$/..$/..$/..$/..
PRJNAME = xmlhelp
TARGET  = HelpLinker
PACKAGE = com$/sun$/star$/help

.IF "$(SOLAR_JAVA)"!=""
# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
 
JAVACLASSFILES = \
    $(CLASSDIR)$/$(PACKAGE)$/HelpSearch.class			        \
    $(CLASSDIR)$/$(PACKAGE)$/HelpComponent.class			        \
    $(CLASSDIR)$/$(PACKAGE)$/HelpIndexer.class			        

TRANSEX3FILES = \
        $(SOLARBINDIR)$/help$/$(PACKAGE)$/HelpIndexerTool.class		\
        $(SOLARBINDIR)$/help$/$(PACKAGE)$/HelpFileDocument.class				

ADDFILES = $(subst,$(SOLARBINDIR)$/help,$(CLASSDIR) $(TRANSEX3FILES))

JARFILES  = ridl.jar jurt.jar unoil.jar juh.jar 
.IF "$(SYSTEM_LUCENE)" == "YES"
EXTRAJARFILES = $(LUCENE_CORE_JAR) $(LUCENE_ANALYZERS_JAR)
JARCLASSPATH = $(EXTRAJARFILES)
.ELSE
JARFILES += lucene-core-2.3.jar lucene-analyzers-2.3.jar
JARCLASSPATH = lucene-core-2.3.jar lucene-analyzers-2.3.jar
.ENDIF
  
JARTARGET	   	   = LuceneHelpWrapper.jar
JARCOMPRESS        = TRUE 
CUSTOMMANIFESTFILE = MANIFEST.MF
 
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(JARTARGETN)"!=""
$(JAVATARGET) : $(ADDFILES)
$(JARTARGETN) : $(ADDFILES)
.ENDIF

$(ADDFILES) : $(SOLARBINDIR)$/help$/$(PACKAGE)$/$$(@:f)
    $(MKDIRHIER) $(@:d)	
    $(COPY) $< $@

fix_system_lucene:
    @echo "Fix Java Class-Path entry for Lucene libraries from system."
    @$(SED) -r -e "s#^(Class-Path:).*#\1 file://$(LUCENE_CORE_JAR) file://$(LUCENE_ANALYZERS_JAR)#" \
    -i ../../../../../$(INPATH)/class/HelpLinker/META-INF/MANIFEST.MF

ALLTAR : $(MISC)/LuceneHelpWrapper.component

$(MISC)/LuceneHelpWrapper.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt LuceneHelpWrapper.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_JAVA)$(JARTARGET)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt LuceneHelpWrapper.component
.ELSE
all:
        @echo java disabled
.ENDIF
