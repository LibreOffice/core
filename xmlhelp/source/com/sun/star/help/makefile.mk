#*************************************************************************
#*
#*    $Workfile:$
#*
#*    Creation date     KR 28.06.99
#*    last change       $Author: abi $ $Date: 2000-10-09 12:18:53 $
#*
#*    $Revision: 1.1 $
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

JARFILES 		= sandbox.jar unoil.jar jurt.jar bdb.jar sax.jar xp.jar xt.jar
EXTRAJARFILES 	=  

CLASSGENDIR		= $(OUT)$/classgen
RDB		 		= $(SOLARBINDIR)$/applicat.rdb
JAVAFILES		= $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES))) 


%.java:
    javamaker -BUCR -O$(OUT)$/misc$/java $(subst,$/,. $(subst,$(OUT)$/misc$/java$/,-T  $*)) $(RDB)

# --- Files --------------------------------------------------------

JAVACLASSFILES = \
    $(CLASSDIR)$/$(PACKAGE)$/HelpServer.class                  \
    $(CLASSDIR)$/$(PACKAGE)$/HelpContentIdentifier.class       \
    $(CLASSDIR)$/$(PACKAGE)$/HelpProvider.class                \
    $(CLASSDIR)$/$(PACKAGE)$/HelpContent.class                 \
    $(CLASSDIR)$/$(PACKAGE)$/HelpURLStreamHandlerFactory.class \
    $(CLASSDIR)$/$(PACKAGE)$/HelpURLStreamHandler.class        \
    $(CLASSDIR)$/$(PACKAGE)$/HelpURLConnection.class           \
    $(CLASSDIR)$/$(PACKAGE)$/CreateDb.class                    \
    $(CLASSDIR)$/$(PACKAGE)$/OutputStreamImpl.class            \
    $(CLASSDIR)$/$(PACKAGE)$/ParameterImpl.class               \
    $(CLASSDIR)$/$(PACKAGE)$/XSLData.class                     \
    $(CLASSDIR)$/$(PACKAGE)$/StringDbt.class


JARCLASSDIRS	= com
JARTARGET		= $(PRJNAME).jar
JARCOMPRESS 	= TRUE
CUSTOMMANIFESTFILE = manifest


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk




