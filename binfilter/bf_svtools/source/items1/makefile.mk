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
TARGET=items1
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
INC+= -I$(PRJ)$/inc$/bf_svtools

# --- Files --------------------------------------------------------

SLOFILES=\
    $(SLO)$/svt_bintitem.obj	\
    $(SLO)$/svt_cenumitm.obj	\
    $(SLO)$/svt_cintitem.obj	\
    $(SLO)$/svt_cntwall.obj	\
    $(SLO)$/svt_cstitem.obj	\
    $(SLO)$/svt_ctypeitm.obj	\
    $(SLO)$/svt_custritm.obj	\
    $(SLO)$/svt_dateitem.obj	\
    $(SLO)$/svt_dtritem.obj	\
    $(SLO)$/svt_frqitem.obj	\
    $(SLO)$/svt_ilstitem.obj    \
    $(SLO)$/svt_itemiter.obj	\
    $(SLO)$/svt_itempool.obj	\
    $(SLO)$/svt_itemprop.obj	\
    $(SLO)$/svt_itemset.obj	\
    $(SLO)$/svt_lckbitem.obj	\
    $(SLO)$/svt_poolio.obj	\
    $(SLO)$/svt_stylepool.obj	\
    $(SLO)$/svt_poolitem.obj	\
    $(SLO)$/svt_sfontitm.obj	\
    $(SLO)$/svt_sitem.obj	    \
    $(SLO)$/svt_slstitm.obj	\
    $(SLO)$/svt_tfrmitem.obj	\
    $(SLO)$/svt_tresitem.obj	\
    $(SLO)$/svt_whiter.obj \
    $(SLO)$/svt_visitem.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

