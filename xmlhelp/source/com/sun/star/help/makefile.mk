#*************************************************************************
#*
#*    $Workfile:$
#*
#*    Creation date     KR 28.06.99
#*    last change       $Author: kz $ $Date: 2001-03-29 14:43:37 $
#*
#*    $Revision: 1.13 $
#*
#*    $Logfile:$
#*
#*    Copyright 2000 Sun Microsystems, Inc. All Rights Reserved.
#*
#*************************************************************************

PRJ		= ..$/..$/..$/..$/..
PRJNAME = xmlhelp
PACKAGE = com$/sun$/star$/help
TARGET  = com_sun_star_help

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

JARFILES 		= jaxp.jar parser.jar xt.jar unoil.jar ridl.jar jurt.jar jut.jar xmlsearch.jar db31.jar
EXTRAJARFILES 	=  ;e:$/src626a$/program$/classes$/db.jar

CLASSGENDIR		= $(OUT)$/classgen
RDB		 		= $(SOLARBINDIR)$/applicat.rdb
JAVAFILES		= $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES))) 

UNIXTEXT= \
    $(MISC)$/helpserver.sh

ALL:	            \
    $(UNIXTEXT)	\
    ALLTAR



$(BIN)$/helpserver.sh: helpserver.sh
    +$(COPY) helpserver.sh $(BIN)

%.java:
    javamaker -BUCR -O$(OUT)$/misc$/java $(subst,$/,. $(subst,$(OUT)$/misc$/java$/,-T  $*)) $(RDB)

# --- Files --------------------------------------------------------

JAVACLASSFILES = \
    $(CLASSDIR)$/$(PACKAGE)$/HelpServer.class                   \
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
    $(CLASSDIR)$/$(PACKAGE)$/HelpTransformer.class              \
    $(CLASSDIR)$/$(PACKAGE)$/HelpPackager.class                 \
    $(CLASSDIR)$/$(PACKAGE)$/CreateDb.class                     \
    $(CLASSDIR)$/$(PACKAGE)$/XSLData.class                      \
    $(CLASSDIR)$/$(PACKAGE)$/StringDbt.class 

JARCLASSDIRS	= com
JARTARGET		= $(PRJNAME).jar
JARCOMPRESS 	= TRUE
CUSTOMMANIFESTFILE = manifest


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk




