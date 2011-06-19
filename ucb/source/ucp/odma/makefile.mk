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

UCP_VERSION=1
UCP_NAME=odma


PRJ=..$/..$/..
PRJNAME=ucb

TARGET=ucp$(UCP_NAME)
TARGET2=s$(UCP_NAME)

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

TARGET2TYPE=CUI
LIBTARGET=NO

# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

# --- General -----------------------------------------------------

SLO1FILES=\
    $(SLO)$/odma_lib.obj    		\
    $(SLO)$/odma_services.obj    	\
    $(SLO)$/odma_provider.obj    	\
    $(SLO)$/odma_content.obj     	\
    $(SLO)$/odma_resultset.obj		\
    $(SLO)$/odma_datasupplier.obj	\
    $(SLO)$/odma_inputstream.obj	\
    $(SLO)$/odma_contentcaps.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLO1FILES)

# --- Shared-Library ---------------------------------------------------

SHL1TARGET=$(TARGET)$(UCP_VERSION)
SHL1IMPLIB=i$(TARGET)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(SALHELPERLIB)	\
    $(UCBHELPERLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)

# --- SODMA executable -------------------------------------------------
OBJFILES= $(OBJ)$/odma_main.obj 

APP2TARGET= $(TARGET2)
APP2OBJS=   $(OBJFILES)
APP2STDLIBS=$(SALLIB)			\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)

DEF2DES=UCB ODMA URL converter

# --- odma_lib library -----------------------------------------------

SLO3FILES=$(SLO)$/odma_lib.obj

LIB3TARGET=$(SLB)$/odma_lib.lib
LIB3OBJFILES=$(SLO3FILES)

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

ALLTAR : $(MISC)/$(TARGET)$(UCP_VERSION).component

$(MISC)/$(TARGET)$(UCP_VERSION).component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        $(TARGET)$(UCP_VERSION).component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt $(TARGET)$(UCP_VERSION).component
