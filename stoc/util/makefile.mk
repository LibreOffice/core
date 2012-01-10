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


PRJ=..

PRJNAME= bootstrap
TARGET = bootstrap.uno
ENABLE_EXCEPTIONS=TRUE

# --- openoffice.org.orig/Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

SHL1TARGET=	$(TARGET)

LIB1TARGET=	$(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/bootstrap.lib \
        $(SLB)$/security.lib \
        $(SLB)$/servicemgr.lib \
        $(SLB)$/simplereg.lib \
        $(SLB)$/nestedreg.lib \
        $(SLB)$/implreg.lib \
        $(SLB)$/shlibloader.lib \
        $(SLB)$/regtypeprov.lib \
        $(SLB)$/typemgr.lib

SHL1VERSIONMAP = $(SOLARENV)/src/unloadablecomponent.map

SHL1STDLIBS= \
        $(CPPULIB)		\
                $(SALHELPERLIB)         \
        $(CPPUHELPERLIB)	\
                $(SALLIB)		\
        $(REGLIB) \
        $(XMLREADERLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=  URELIB

DEF1NAME=	$(SHL1TARGET)

SHL2TARGET=	stocservices.uno

LIB2TARGET=	$(SLB)$/stocservices.uno.lib
LIB2FILES=	\
        $(SLB)$/stocservices.lib \
        $(SLB)$/typeconverter.lib \
        $(SLB)$/uriproc.lib 

SHL2VERSIONMAP = $(SOLARENV)/src/unloadablecomponent.map

SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
                $(SALLIB)		

SHL2DEPN=
SHL2IMPLIB=	istocservices.uno
SHL2LIBS=	$(SLB)$/stocservices.uno.lib
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
SHL2RPATH=  URELIB

DEF2NAME=	$(SHL2TARGET)

# --- openoffice.org.orig/Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/bootstrap.component $(MISC)/stocservices.component

$(MISC)/bootstrap.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        bootstrap.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_URE_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt bootstrap.component

$(MISC)/stocservices.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt stocservices.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_URE_NATIVE)$(SHL2TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt stocservices.component
