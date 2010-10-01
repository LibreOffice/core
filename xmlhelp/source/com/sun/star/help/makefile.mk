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
