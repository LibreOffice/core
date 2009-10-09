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
# $Revision: 1.13 $
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
PRJNAME=svl
TARGET=items
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/svl.pmk

# --- Files --------------------------------------------------------

SLOFILES=\
    $(SLO)$/aeitem.obj	\
    $(SLO)$/eitem.obj	\
    $(SLO)$/flagitem.obj	\
    $(SLO)$/globalnameitem.obj	\
    $(SLO)$/imageitm.obj    \
    $(SLO)$/intitem.obj	\
    $(SLO)$/macitem.obj	\
    $(SLO)$/poolcach.obj	\
    $(SLO)$/ptitem.obj	\
    $(SLO)$/rectitem.obj	\
    $(SLO)$/rngitem.obj	\
    $(SLO)$/stritem.obj	\
    $(SLO)$/style.obj	\
    $(SLO)$/szitem.obj \
    $(SLO)$/bintitem.obj	\
    $(SLO)$/cenumitm.obj	\
    $(SLO)$/cintitem.obj	\
    $(SLO)$/cntwall.obj	\
    $(SLO)$/ctypeitm.obj	\
    $(SLO)$/custritm.obj	\
    $(SLO)$/dtritem.obj	\
    $(SLO)$/frqitem.obj	\
    $(SLO)$/ilstitem.obj    \
    $(SLO)$/itemiter.obj	\
    $(SLO)$/itempool.obj	\
    $(SLO)$/itemprop.obj	\
    $(SLO)$/itemset.obj	\
    $(SLO)$/lckbitem.obj	\
    $(SLO)$/poolio.obj	\
    $(SLO)$/stylepool.obj	\
    $(SLO)$/poolitem.obj	\
    $(SLO)$/sfontitm.obj	\
    $(SLO)$/sitem.obj	    \
    $(SLO)$/slstitm.obj	\
    $(SLO)$/tfrmitem.obj	\
    $(SLO)$/tresitem.obj	\
    $(SLO)$/whiter.obj \
    $(SLO)$/visitem.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

