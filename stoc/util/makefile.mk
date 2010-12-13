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
