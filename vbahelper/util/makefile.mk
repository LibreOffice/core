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

PRJNAME=vbahelper
TARGET=vbahelper

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(ENABLE_VBA)" == "NO"
dummy:
    @echo "Nothing to build"
.ENDIF

TARGET_HELPER=vbahelper

LIB1TARGET=     $(SLB)$/$(TARGET).lib
LIB1FILES=$(SLB)$/$(TARGET)bits.lib

SHL1TARGET= $(TARGET_HELPER)$(DLLPOSTFIX)


# dynamic libraries
SHL1STDLIBS=       \
        $(CPPULIB) \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(BASICLIB) \
        $(TOOLSLIB) \
        $(SALLIB)\
        $(SFXLIB)	\
        $(SVTOOLLIB)    \
        $(SVLLIB) \
        $(VCLLIB) \
        $(SVTOOLLIB)    \
        $(MSFILTERLIB)    \
        $(TKLIB)

SHL1DEPN=
SHL1IMPLIB= i$(TARGET_HELPER)
SHL1USE_EXPORTS=name
SHL1LIBS=$(LIB1TARGET)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)

TARGET_MSFORMS=msforms
SHL2TARGET=$(TARGET_MSFORMS)$(DLLPOSTFIX).uno
SHL2IMPLIB=     i$(TARGET_MSFORMS)

SHL2VERSIONMAP=$(SOLARENV)/src/component.map
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=$(SHL2TARGET)
SHL2STDLIBS= \
                $(CPPUHELPERLIB) \
                $(CPPULIB) \
                $(COMPHELPERLIB) \
                $(SVLIB) \
                $(TOOLSLIB) \
                $(SALLIB)\
        $(VBAHELPERLIB) \
                $(SFXLIB)       \
                $(SVXLIB)       \
                $(SVTOOLLIB)    \
                $(SVLLIB) \
        $(VCLLIB) \
        $(TKLIB) \
                $(BASICLIB) \
        $(SVXMSFILTERLIB) \

SHL2DEPN=$(SHL1TARGETN)
SHL2LIBS=$(SLB)$/$(TARGET_MSFORMS).lib

# --- Targets -----------------------------------------------------------

.INCLUDE :  target.mk
