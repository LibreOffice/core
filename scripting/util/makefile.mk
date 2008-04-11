#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.9 $
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

ENABLE_EXCEPTIONS=TRUE
PRJNAME=scripting
TARGET=scriptframe
USE_DEFFILE=    TRUE
NO_BSYMBOLIC=   TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ----------------------------------------------------
LIB1TARGET=	 $(SLB)$/$(TARGET).lib
LIB1FILES=   $(SLB)/provider.lib


SHL1TARGET= $(TARGET)
SHL1IMPLIB= $(TARGET)

SHL1STDLIBS+=\
    $(TOOLSLIB) \
    $(CPPULIB) \
    $(CPPUHELPERLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(SALLIB)


SHL1LIBS=	$(SLB)$/$(TARGET).lib

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME	=$(SHL1TARGET)
DEF1EXPORTFILE= exports.dxp
# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk
