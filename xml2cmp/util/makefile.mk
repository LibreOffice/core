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

PRJNAME=xml2cmp
TARGET=xml2cmp
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------
ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  settings.mk

.IF "$(CROSS_COMPILING)"=="YES"
all:
    @echo Nothing done when cross-compiling
.ENDIF

# --- Files --------------------------------------------------------

# --- Targets ------------------------------------------------------

APP1TARGET=	$(TARGET)
APP1STACK=	1000000

UWINAPILIB=$(0)
LIBSALCPPRT=$(0)

APP1LIBS=	$(LB)$/x2c_xcd.lib $(LB)$/x2c_support.lib
APP1DEPN=   $(LB)$/x2c_xcd.lib $(LB)$/x2c_support.lib
APP1RPATH=  NONE

.INCLUDE :  target.mk
