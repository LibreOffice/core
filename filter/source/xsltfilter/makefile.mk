#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.19 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:34:41 $
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
