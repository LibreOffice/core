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

PRJ=				..$/..
PRJNAME=			chart2
TARGET=				chartcontroller

USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- export library -------------------------------------------------

#You can use several library macros of this form to build libraries that
#do not consist of all object files in a directory or to merge different libraries.
LIB1TARGET=		$(SLB)$/$(TARGET).lib

LIB1FILES=		\
                $(SLB)$/chcontroller.lib \
                $(SLB)$/chcdrawinglayer.lib \
                $(SLB)$/chcitemsetwrapper.lib \
                $(SLB)$/chcdialogs.lib \
                $(SLB)$/chchartapiwrapper.lib \
                $(SLB)$/chcaccessibility.lib

#--------

#Indicates the filename of the shared library.
SHL1TARGET=		$(TARGET)$(DLLPOSTFIX)

#indicates dependencies:
.IF "$(COM)" == "MSC"
SHL1DEPN = \
        $(LB)$/icharttools.lib \
        $(LB)$/ichartview.lib
.ELSE
SHL1DEPN =
.ENDIF

#Specifies an import library to create. For Win32 only.
SHL1IMPLIB=		i$(TARGET)

#Specifies libraries from the same module to put into the shared library.
#was created above
SHL1LIBS= 		$(LIB1TARGET)

#Links import libraries.

SHL1STDLIBS=	$(CHARTTOOLS)		\
                $(CHARTVIEW)		\
                $(EDITENGLIB)			\
                $(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(BASEGFXLIB)		\
                $(DRAWINGLAYERLIB)	\
                $(BASEGFXLIB) 		\
                $(SALLIB)			\
                $(SVLLIB)			\
                $(SVTOOLLIB)		\
                $(SVXCORELIB)			\
                $(SVXLIB)			\
                $(TKLIB)			\
                $(TOOLSLIB) 		\
                $(VCLLIB)           \
                $(SFXLIB)			\
                $(UNOTOOLSLIB)		\
                $(SOTLIB)

#--------exports

#specifies the exported symbols for Windows only:
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP = $(SOLARENV)/src/component.map

#--------definition file

#name of the definition file:
DEF1NAME=		$(SHL1TARGET)

# --- Resources ---------------------------------------------------------------

RESLIB1LIST=\
    $(SRS)$/chcdialogs.srs \
    $(SRS)$/chcmenus.srs \


RESLIB1NAME=	$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RESLIB1LIST)
RESLIB1DEPN=$(RESLIB1LIST)

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk

ALLTAR : $(MISC)/chartcontroller.component

$(MISC)/chartcontroller.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt chartcontroller.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt chartcontroller.component
