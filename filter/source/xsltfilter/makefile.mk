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
PACKAGE = com/sun/star/comp/xsltfilter
TARGET  =XSLTFilter
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------
CLASSDIR!:=$(CLASSDIR)$/$(TARGET)
.INCLUDE: settings.mk

.IF "$(DISABLE_SAXON)" == ""

SLOFILES= \
            $(SLO)$/XSLTFilter.obj \
            $(SLO)$/fla.obj \
            $(SLO)$/uof2storage.obj \
            $(SLO)$/uof2merge.obj \
            $(SLO)$/XMLBase64Codec.obj \
            $(SLO)$/uof2splithandler.obj \
            $(SLO)$/uof2splitter.obj
            
LIBNAME=xsltfilter
SHL1TARGETDEPN=makefile.mk
SHL1OBJS=$(SLOFILES)
SHL1TARGET=$(LIBNAME)$(DLLPOSTFIX)
SHL1IMPLIB=i$(LIBNAME)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS= \
    $(TOOLSLIB)         \
    $(CPPUHELPERLIB)    \
    $(CPPULIB)          \
    $(XMLOFFLIB) \
    $(SALLIB) \
    $(COMPHELPERLIB)

.IF "$(SOLAR_JAVA)"!=""

#USE_UDK_EXTENDED_MANIFESTFILE=TRUE
#USE_EXTENDED_MANIFESTFILE=TRUE
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

.ELSE
all:
    @echo "saxon disabled"
.ENDIF
