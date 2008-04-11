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
# $Revision: 1.20 $
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
PRJ	= ..$/..
PRJNAME = filter
#PACKAGE = com$/sun$/star$/documentconversion$/XSLTFilter
TARGET  =XSLTFilter
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------
CLASSDIR!:=$(CLASSDIR)$/$(TARGET)
.IF "$(XML_CLASSPATH)" != ""
XCLASSPATH+=":$(XML_CLASSPATH)"
.ENDIF
.INCLUDE: settings.mk

SLOFILES=$(SLO)$/XSLTFilter.obj $(SLO)$/fla.obj
LIBNAME=xsltfilter
SHL1TARGETDEPN=makefile.mk
SHL1OBJS=$(SLOFILES)
SHL1TARGET=$(LIBNAME)$(DLLPOSTFIX)
SHL1IMPLIB=i$(LIBNAME)
SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS= \
    $(VCLLIB)           \
    $(UNOTOOLSLIB)      \
    $(TOOLSLIB)         \
    $(VOSLIB)           \
    $(CPPUHELPERLIB)    \
    $(CPPULIB)          \
    $(XMLOFFLIB) \
    $(SALLIB)

.IF "$(SOLAR_JAVA)"!=""

#USE_UDK_EXTENDED_MANIFESTFILE=TRUE
#USE_EXTENDED_MANIFESTFILE=TRUE
JARFILES 		= ridl.jar unoil.jar jurt.jar juh.jar

.IF "$(SYSTEM_XALAN)" == "YES"
XCLASSPATH!:=$(XCLASSPATH)$(PATH_SEPERATOR)$(XALAN_JAR)
.IF "$(SERIALIZER_JAR)" != ""
XCLASSPATH!:=$(XCLASSPATH)$(PATH_SEPERATOR)$(SERIALIZER_JAR)
.ENDIF
.ELSE
JARFILES += xalan.jar
.ENDIF

.IF "$(SYSTEM_XERCES)" == "YES"
XCLASSPATH!:=$(XCLASSPATH)$(PATH_SEPERATOR)$(XERCES_JAR)
.ELSE
JARFILES += xercesImpl.jar
.ENDIF

.IF "$(SYSTEM_XML_APIS)" == "YES"
XCLASSPATH!:=$(XCLASSPATH)$(PATH_SEPERATOR)$(XML_APIS_JAR)
.ELSE
JARFILES += xml-apis.jar
.ENDIF

JAVAFILES		= $(subst,$(CLASSDIR)$/, $(subst,.class,.java $(JAVACLASSFILES)))
CUSTOMMANIFESTFILE = Manifest

JARCOMPRESS		= TRUE
JARCLASSDIRS	= XSLTransformer*.class XSLTFilterOLEExtracter*.class
JARTARGET		= $(TARGET).jar
JARCLASSDIRS 	+= XSLTXalanOLEExtracter*.class

# --- Files --------------------------------------------------------
JAVACLASSFILES=$(CLASSDIR)$/XSLTransformer.class  $(CLASSDIR)$/XSLTFilterOLEExtracter.class
#this class we need xalan.jar.
JAVACLASSFILES+=$(CLASSDIR)$/XSLTXalanOLEExtracter.class
.ENDIF

# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk
.IF "$(SOLAR_JAVA)"!=""
$(JAVACLASSFILES) : $(CLASSDIR)

$(CLASSDIR) :
    $(MKDIR) $(CLASSDIR)
.ENDIF
