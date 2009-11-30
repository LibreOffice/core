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
# $Revision: 1.38 $
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
LIBBASENAME = helplinker
PACKAGE = com$/sun$/star$/help
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
.INCLUDE : helplinker.pmk
 
.IF "$(SYSTEM_LIBXSLT)" == "YES"
CFLAGS+= $(LIBXSLT_CFLAGS)
.ELSE
LIBXSLTINCDIR=external$/libxslt
CFLAGS+= -I$(SOLARINCDIR)$/$(LIBXSLTINCDIR)
.ENDIF

.IF "$(SYSTEM_DB)" == "YES"
CFLAGS+=-DSYSTEM_DB -I$(DB_INCLUDES)
.ENDIF

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ENDIF


JAVACLASSFILES = \
    $(SOLARBINDIR)$/help$/$(PACKAGE)$/HelpIndexerTool.class			\
    $(SOLARBINDIR)$/help$/$(PACKAGE)$/HelpFileDocument.class		\
    $(CLASSDIR)$/$(PACKAGE)$/HelpSearch.class			        \
    $(CLASSDIR)$/$(PACKAGE)$/HelpComponent.class			        \
    $(CLASSDIR)$/$(PACKAGE)$/HelpIndexer.class			        

JAVAFILES = \
    HelpSearch.java 							\
    HelpComponent.java							\
    HelpIndexer.java

TRANSEX3FILES = \
        $(SOLARBINDIR)$/help$/$(PACKAGE)$/HelpIndexerTool.class		\
        $(SOLARBINDIR)$/help$/$(PACKAGE)$/HelpFileDocument.class				

ADDFILES = $(subst,$(SOLARBINDIR)$/help,$(CLASSDIR) $(TRANSEX3FILES))

#JAVAFILES = $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES)))

JARFILES  = ridl.jar jurt.jar unoil.jar juh.jar HelpIndexerTool.jar
.IF "$(SYSTEM_LUCENE)" == "YES"
XCLASSPATH!:=$(XCLASSPATH)$(PATH_SEPERATOR)$(LUCENE_CORE_JAR)$(PATH_SEPERATOR)$(LUCENE_ANALYZERS_JAR)
COMP=fix_system_lucene
.ELSE
JARFILES += lucene-core-2.3.jar lucene-analyzers-2.3.jar
.ENDIF
  
JARTARGET	   	   = LuceneHelpWrapper.jar
JARCOMPRESS        = TRUE 
CUSTOMMANIFESTFILE = MANIFEST.MF 
 
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(ADDFILES)

.IF "$(JARTARGETN)"!=""
$(JARTARGETN) : $(ADDFILES)
$(JARTARGETN) : $(COMP)
.ENDIF

$(CLASSDIR)$/$(PACKAGE)$/%.class : $(SOLARBINDIR)$/help$/$(PACKAGE)$/%.class 
    $(MKDIRHIER) $(@:d)	
    $(COPY) $< $@


fix_system_lucene:
    @echo "Fix Java Class-Path entry for Lucene libraries from system."
    @$(SED) -r -e "s#^(Class-Path:).*#\1 file://$(LUCENE_CORE_JAR) file://$(LUCENE_ANALYZERS_JAR)#" \
    -i ../../../../../$(INPATH)/class/HelpLinker/META-INF/MANIFEST.MF
