#*************************************************************************
#*
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.16 $
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
#************************************************************************/

PRJ=..$/..


PRJNAME=i18npool
TARGET=indexentry

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk


# --- Files --------------------------------------------------------

SLOFILES=   \
            $(SLO)$/indexentrysupplier.obj \
            $(SLO)$/indexentrysupplier_asian.obj \
            $(SLO)$/indexentrysupplier_ja_phonetic.obj \
            $(SLO)$/indexentrysupplier_default.obj \
            $(SLO)$/indexentrysupplier_common.obj

OBJFILES   = $(OBJ)$/genindex_data.obj

APP1TARGET = genindex_data

DEPOBJFILES   = $(OBJ)$/genindex_data.obj
APP1OBJS   = $(DEPOBJFILES)

APP1STDLIBS = $(SALLIB) \
        $(ICUINLIB) \
        $(ICUUCLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

