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

PRJ=..$/..

PRJNAME=xml2cmp
TARGET=x2c_xcd
TARGETTYPE=CUI

LIBTARGET=NO

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  settings.mk

.IF "$(CROSS_COMPILING)"=="YES"
all:
    @echo Nothing done when cross-compiling
.ENDIF

# --- Files --------------------------------------------------------

LIBONLYFILES=\
    $(OBJ)$/cr_html.obj		\
    $(OBJ)$/cr_index.obj	\
    $(OBJ)$/cr_metho.obj	\
    $(OBJ)$/filebuff.obj	\
    $(OBJ)$/parse.obj		\
    $(OBJ)$/xmlelem.obj		\
    $(OBJ)$/xmltree.obj

OBJFILES=\
    $(OBJ)$/main.obj		\
    $(LIBONLYFILES)

LIB1TARGET=$(LB)$/$(TARGET).lib
LIB1OBJFILES=\
    $(OBJFILES)

LIB2TARGET=$(LB)$/$(TARGET)l.lib
LIB2OBJFILES=\
    $(LIBONLYFILES)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
