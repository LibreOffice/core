#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.24 $
#
#   last change: $Author: kz $ $Date: 2006-07-05 21:13:24 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ		= ..$/..$/..$/..$/..
PRJNAME = xmlhelp
PACKAGE = com$/sun$/star$/help
TARGET  = com_sun_star_help

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

JARFILES 	= xt.jar unoil.jar ridl.jar jurt.jar jut.jar xmlsearch.jar
EXTRAJARFILES 	= 

.IF "$(SYSTEM_XML_APIS)" == "YES"
.IF "$(XCLASSPATH)" != ""
XCLASSPATH!:=$(XCLASSPATH)$(PATH_SEPERATOR)$(XML_APIS_JAR)
.ELSE
XCLASSPATH!:=$(XML_APIS_JAR)
.ENDIF
.ELSE
JARFILES += xml-apis.jar
.ENDIF

.IF "$(SYSTEM_XERCES)" == "YES"
.IF "$(XCLASSPATH)" != ""
XCLASSPATH!:=$(XCLASSPATH)$(PATH_SEPERATOR)$(XERCES_JAR)
.ELSE
XCLASSPATH!:=$(XERCES_JAR)
.ENDIF
.ELSE
JARFILES += xercesImpl.jar
.ENDIF

.IF "$(SYSTEM_DB)" == "YES"
.IF "$(XCLASSPATH)" != ""
XCLASSPATH!:=$(XCLASSPATH)$(PATH_SEPERATOR)$(DB_JAR)
.ELSE
XCLASSPATH!:=$(DB_JAR)
.ENDIF
.ELSE
JARFILES += db.jar
.ENDIF

CLASSGENDIR		= $(OUT)$/classgen
RDB	 		= $(SOLARBINDIR)$/types.rdb
JAVAFILES		= $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES))) 

# --- Files --------------------------------------------------------

JAVACLASSFILES = \
    $(CLASSDIR)$/$(PACKAGE)$/HelpCompiler.class			        \
    $(CLASSDIR)$/$(PACKAGE)$/HelpLinker.class  			        \
    $(CLASSDIR)$/$(PACKAGE)$/HelpContentIdentifier.class        \
    $(CLASSDIR)$/$(PACKAGE)$/HelpProvider.class                 \
    $(CLASSDIR)$/$(PACKAGE)$/HelpContent.class                  \
    $(CLASSDIR)$/$(PACKAGE)$/HelpOutputStream.class             \
    $(CLASSDIR)$/$(PACKAGE)$/HelpDatabases.class                \
    $(CLASSDIR)$/$(PACKAGE)$/HelpURLStreamHandlerFactory.class  \
    $(CLASSDIR)$/$(PACKAGE)$/HelpURLStreamHandler.class         \
    $(CLASSDIR)$/$(PACKAGE)$/HelpURLStreamHandlerWithJars.class \
    $(CLASSDIR)$/$(PACKAGE)$/HelpURLConnection.class            \
    $(CLASSDIR)$/$(PACKAGE)$/HelpURLConnectionWithJars.class    \
    $(CLASSDIR)$/$(PACKAGE)$/HelpURLParameter.class             \
    $(CLASSDIR)$/$(PACKAGE)$/HelpResultSetFactory.class         \
    $(CLASSDIR)$/$(PACKAGE)$/HelpDynamicResultSet.class         \
    $(CLASSDIR)$/$(PACKAGE)$/HelpResultSetBase.class            \
    $(CLASSDIR)$/$(PACKAGE)$/HelpResultSet.class                \
    $(CLASSDIR)$/$(PACKAGE)$/HelpResultSetForRoot.class         \
    $(CLASSDIR)$/$(PACKAGE)$/HelpIndexer.class                  \
    $(CLASSDIR)$/$(PACKAGE)$/HelpKeyword.class                  \
    $(CLASSDIR)$/$(PACKAGE)$/HelpPackager.class                 \
    $(CLASSDIR)$/$(PACKAGE)$/XSLData.class                      \
    $(CLASSDIR)$/$(PACKAGE)$/MemoryURLConnection.class          \
    $(CLASSDIR)$/$(PACKAGE)$/StringDbt.class 

.IF "$(JDK)"=="gcj"
JAVACLASSFILES += \
    $(CLASSDIR)$/$(PACKAGE)$/GCJFileURLStreamHandler.class
.ELSE
JAVACLASSFILES += \
    $(CLASSDIR)$/$(PACKAGE)$/FileURLStreamHandler.class
.ENDIF

JARCLASSDIRS	= com
JARTARGET		= $(PRJNAME).jar
JARCOMPRESS 	= TRUE
CUSTOMMANIFESTFILE = manifest

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(JAVAAOTCOMPILER)" != ""
AOTTARGET = com.sun.star.help.HelpLinker
.INCLUDE :  aottarget.mk
ALLTAR : $(AOTTARGETN)
.ENDIF
