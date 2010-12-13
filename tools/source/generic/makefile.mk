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

PRJNAME=tools
TARGET=gen

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
.INCLUDE :      $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES = $(SLO)$/poly.obj $(OBJ)$/poly.obj $(SLO)$/svlibrary.obj

SLOFILES=       $(SLO)$/toolsin.obj     \
            $(SLO)$/svlibrary.obj \
            $(SLO)$/b3dtrans.obj                \
            $(SLO)$/link.obj                \
            $(SLO)$/bigint.obj              \
            $(SLO)$/fract.obj               \
            $(SLO)$/color.obj               \
            $(SLO)$/gen.obj			\
            $(SLO)$/config.obj		\
            $(SLO)$/poly.obj		\
            $(SLO)$/poly2.obj		\
            $(SLO)$/svborder.obj \
            $(SLO)$/line.obj

OBJFILES=       $(OBJ)$/toolsin.obj     \
            $(OBJ)$/b3dtrans.obj                \
            $(OBJ)$/link.obj                \
            $(OBJ)$/bigint.obj              \
            $(OBJ)$/fract.obj               \
            $(OBJ)$/color.obj               \
            $(OBJ)$/gen.obj			\
            $(OBJ)$/config.obj		\
            $(OBJ)$/poly.obj		\
            $(OBJ)$/poly2.obj		\
            $(OBJ)$/svborder.obj \
            $(OBJ)$/line.obj

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
