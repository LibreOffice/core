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

PRJNAME=drawinglayer
TARGET=drawinglayer

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ---------------------------------------------------

LIB1TARGET=$(SLB)$/drawinglayer.lib
LIB1FILES=\
    $(SLB)$/primitive2d.lib	\
    $(SLB)$/primitive3d.lib	\
    $(SLB)$/geometry.lib	\
    $(SLB)$/processor2d.lib	\
    $(SLB)$/processor3d.lib	\
    $(SLB)$/attribute.lib	\
    $(SLB)$/animation.lib	\
    $(SLB)$/texture.lib

SHL1TARGET= drawinglayer$(DLLPOSTFIX)
SHL1IMPLIB= idrawinglayer

SHL1STDLIBS=\
        $(VCLLIB)			\
        $(BASEGFXLIB)		\
        $(TOOLSLIB)			\
        $(SVLLIB)			\
        $(TKLIB)			\
        $(CANVASTOOLSLIB)	\
        $(CPPCANVASLIB)		\
        $(SVTOOLLIB)		\
        $(SALLIB)			\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(AVMEDIALIB)		\
        $(I18NISOLANGLIB)	\
        $(COMPHELPERLIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=	$(SLB)$/drawinglayer.lib

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
        $(LIB1TARGET)

DEF1DES		=DrawingLayer
DEFLIB1NAME	=drawinglayer

# --- Targets -----------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt : makefile.mk
    @+$(TYPE) $(TARGET).flt > $@

