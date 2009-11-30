#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile$
#
# $Revision$
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

PRJ=../../..
PRJNAME=toolkit
TARGET=layout-core
ENABLE_EXCEPTIONS=true

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

# FIXME: This is bad, hmkay
CFLAGS+= -I$(PRJ)/source

SLOFILES= \
    $(SLO)$/bin.obj \
    $(SLO)$/box-base.obj \
    $(SLO)$/box.obj \
    $(SLO)$/byteseq.obj \
    $(SLO)$/container.obj \
    $(SLO)$/dialogbuttonhbox.obj \
    $(SLO)$/factory.obj \
    $(SLO)$/flow.obj \
    $(SLO)$/helper.obj \
    $(SLO)$/import.obj \
    $(SLO)$/localized-string.obj \
    $(SLO)$/proplist.obj \
    $(SLO)$/root.obj \
    $(SLO)$/table.obj \
    $(SLO)$/timer.obj \
    $(SLO)$/translate.obj\
    $(SLO)$/vcl.obj\
#

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
