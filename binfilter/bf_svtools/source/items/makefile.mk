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

PRJ=..$/..$/..
PRJNAME=binfilter
TARGET=items
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
INC+= -I$(PRJ)$/inc$/bf_svtools

# --- Files --------------------------------------------------------

SLOFILES=\
    $(SLO)$/svt_aeitem.obj	\
    $(SLO)$/svt_eitem.obj	\
    $(SLO)$/svt_flagitem.obj	\
    $(SLO)$/svt_globalnameitem.obj	\
    $(SLO)$/svt_intitem.obj	\
    $(SLO)$/svt_macitem.obj	\
    $(SLO)$/svt_poolcach.obj	\
    $(SLO)$/svt_ptitem.obj	\
    $(SLO)$/svt_rectitem.obj	\
    $(SLO)$/svt_rngitem.obj	\
    $(SLO)$/svt_stritem.obj	\
    $(SLO)$/svt_style.obj	\
    $(SLO)$/svt_szitem.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

