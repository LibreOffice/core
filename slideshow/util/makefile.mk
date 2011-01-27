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

PRJNAME=slideshow
TARGET=slideshow
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Common ----------------------------------------------------------

.IF "$(VERBOSE)"=="TRUE"
CDEFS+=-DVERBOSE
.ENDIF

SHL1DLLPRE=
LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=\
    $(SLB)$/transitions.lib	\
    $(SLB)$/activities.lib	\
    $(SLB)$/animationnodes.lib	\
    $(SLB)$/shapes.lib \
    $(SLB)$/slide.lib \
    $(SLB)$/engine.lib

SHL1TARGET=$(TARGET).uno

SHL1STDLIBS= $(TOOLSLIB) \
             $(CPPULIB) \
             $(SALLIB) \
             $(VCLLIB) \
             $(COMPHELPERLIB) \
             $(CPPUHELPERLIB) \
             $(BASEGFXLIB) \
             $(CANVASTOOLSLIB) \
             $(CPPCANVASLIB) \
             $(UNOTOOLSLIB) \
             $(SVTOOLLIB) \
             $(AVMEDIALIB)

SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# ==========================================================================

TARGET2=slideshowtest
LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2FILES=$(LIB1FILES)

SHL2TARGET=$(TARGET2)$(DLLPOSTFIX)
SHL2IMPLIB=i$(TARGET2)
SHL2STDLIBS= $(SHL1STDLIBS)

SHL2DEF=$(MISC)$/$(SHL2TARGET).def
SHL2LIBS=$(SLB)$/$(TARGET2).lib

DEF2NAME	=$(SHL2TARGET)
DEF2LIBNAME = $(TARGET2)
DEF2DEPN	=$(MISC)$/$(SHL2TARGET).flt

DEF2DES		=SlideShowTest

# ==========================================================================

.INCLUDE :	target.mk

$(MISC)$/$(SHL2TARGET).flt: makefile.mk \
                            $(TARGET2).flt
    +$(TYPE) $(TARGET2).flt > $@


ALLTAR : $(MISC)/slideshow.component

$(MISC)/slideshow.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        slideshow.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt slideshow.component
