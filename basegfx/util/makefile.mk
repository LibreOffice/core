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

PRJNAME=basegfx
TARGET=basegfx

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ---------------------------------------------------

LIB1TARGET=$(SLB)$/basegfx.lib
LIB1FILES=\
    $(SLB)$/curve.lib	\
    $(SLB)$/matrix.lib	\
    $(SLB)$/numeric.lib	\
    $(SLB)$/point.lib	\
    $(SLB)$/polygon.lib	\
    $(SLB)$/range.lib	\
    $(SLB)$/tuple.lib	\
    $(SLB)$/tools.lib	\
    $(SLB)$/vector.lib	\
    $(SLB)$/color.lib	\
    $(SLB)$/pixel.lib	\
    $(SLB)$/raster.lib

SHL1TARGET= basegfx$(DLLPOSTFIX)
.IF "$(COM)" == "MSC"
SHL1IMPLIB= ibasegfx
.ELSE
SHL1IMPLIB= $(SHL1TARGET)
.ENDIF

SHL1STDLIBS=\
        $(SALLIB)		 \
        $(CPPUHELPERLIB) \
        $(CPPULIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=	$(SLB)$/basegfx.lib

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt \
        $(LIB1TARGET)

DEF1DES		=BaseGFX
DEFLIB1NAME	=basegfx

LIB2TARGET=$(LB)$/$(TARGET)_s.lib
LIB2ARCHIV=$(LB)$/lib$(TARGET)_s.a
LIB2FILES=$(LIB1FILES)

# --- Targets -----------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt : makefile.mk
    @$(TYPE) $(TARGET).flt > $@

