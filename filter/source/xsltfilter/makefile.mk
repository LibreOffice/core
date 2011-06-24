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
PRJ	= ..$/..
PRJNAME = filter
PACKAGE = com/sun/star/comp/xsltfilter
TARGET  =XSLTFilter
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------
CLASSDIR!:=$(CLASSDIR)$/$(TARGET)

.INCLUDE: settings.mk

.IF "$(SYSTEM_LIBXSLT)" == "YES"
CFLAGS+= $(LIBXSLT_CFLAGS)
.ELSE
LIBXSLTINCDIR=external$/libxslt
CFLAGS+= -I$(SOLARINCDIR)$/$(LIBXSLTINCDIR)
.ENDIF

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+= $(LIBXML_CFLAGS)
.ENDIF

SLOFILES=$(SLO)$/XSLTFilter.obj $(SLO)$/LibXSLTTransformer.obj $(SLO)/OleHandler.obj
LIBNAME=xsltfilter
SHL1TARGETDEPN=makefile.mk
SHL1OBJS=$(SLOFILES)
SHL1TARGET=$(LIBNAME)$(DLLPOSTFIX)
SHL1IMPLIB=i$(LIBNAME)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS= \
    $(TOOLSLIB) \
    $(CPPUHELPERLIB) \
    $(UCBHELPERLIB) \
    $(COMPHELPERLIB) \
    $(CPPULIB) \
    $(XMLOFFLIB) \
    $(SALLIB) \
    $(LIBXML2LIB) \
    $(XSLTLIB) \
    $(PACKAGE2LIB)

.IF "$(SOLAR_JAVA)"!=""

JARFILES 		= ridl.jar unoil.jar jurt.jar juh.jar

JAVAFILES		= $(subst,$(CLASSDIR)$/, $(subst,.class,.java $(JAVACLASSFILES)))
CUSTOMMANIFESTFILE = Manifest

JARCOMPRESS		= TRUE
JARCLASSDIRS	= com/sun/star/comp/xsltfilter
JARTARGET		= $(TARGET).jar

.IF "$(SYSTEM_SAXON)" == "YES"
XCLASSPATH:=$(XCLASSPATH)$(PATH_SEPERATOR)$(SAXON_JAR)
.ELSE
JARFILES += saxon9.jar
.ENDIF

# --- Files --------------------------------------------------------
JAVACLASSFILES= \
 $(CLASSDIR)/com/sun/star/comp/xsltfilter/XSLTransformer.class \
 $(CLASSDIR)/com/sun/star/comp/xsltfilter/XSLTFilterOLEExtracter.class \
 $(CLASSDIR)/com/sun/star/comp/xsltfilter/Base64.class \

.ENDIF

# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk
.IF "$(SOLAR_JAVA)"!=""
$(JAVACLASSFILES) : $(CLASSDIR)

$(CLASSDIR) :
    $(MKDIR) $(CLASSDIR)
.ENDIF

ALLTAR : $(MISC)/XSLTFilter.jar.component $(MISC)/xsltfilter.component

$(MISC)/XSLTFilter.jar.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt XSLTFilter.jar.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_JAVA)$(JARTARGET)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt XSLTFilter.jar.component

$(MISC)/xsltfilter.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        xsltfilter.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt xsltfilter.component
