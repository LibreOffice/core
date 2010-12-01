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

PRJNAME=xmloff
TARGET=xo
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Allgemein ----------------------------------------------------
.IF "$(L10N_framework)"==""
LIB1TARGET= $(SLB)$/xo.lib
LIB1FILES=	\
    $(SLB)$/core.lib \
    $(SLB)$/meta.lib \
    $(SLB)$/script.lib \
    $(SLB)$/style.lib \
    $(SLB)$/text.lib \
    $(SLB)$/draw.lib \
    $(SLB)$/chart.lib \
    $(SLB)$/forms.lib \
    $(SLB)$/form_handlers.lib \
    $(SLB)$/xforms.lib \
    $(SLB)$/table.lib

# --- Shared-Library -----------------------------------------------

SHL1TARGET= xo$(DLLPOSTFIX)
SHL1IMPLIB= i$(TARGET)
SHL1USE_EXPORTS=name

SHL1STDLIBS= \
        $(TOOLSLIB)		\
        $(I18NISOLANGLIB) \
        $(VOSLIB)		\
        $(CPPULIB)		\
        $(CPPUHELPERLIB)\
        $(COMPHELPERLIB)\
        $(SVLLIB)		\
        $(SALLIB)		\
        $(UNOTOOLSLIB)	\
        $(BASEGFXLIB)

# SCO: the linker does know about weak symbols, but we can't ignore multiple defined symbols
.IF "(OS)"=="SCO"
SHL1STDLIBS+=-licg617mxp
.ENDIF

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1LIBS=   $(LIB1TARGET)

# --- Def-File ---------------------------------------------------------

DEF1NAME    =$(SHL1TARGET)
DEFLIB1NAME =xo
DEF1DES     =XML Office Lib

# --- Targets ----------------------------------------------------------
.ENDIF
.INCLUDE :  target.mk

ALLTAR : $(MISC)/xo.component

$(MISC)/xo.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        xo.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt xo.component
