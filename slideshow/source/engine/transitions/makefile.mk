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
# $Revision: 1.6 $
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

PRJNAME=slideshow
TARGET=transitions
ENABLE_EXCEPTIONS=TRUE
PRJINC=..$/..

# --- Settings -----------------------------------------------------------

.INCLUDE :      settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Common ----------------------------------------------------------

SLOFILES = \
        $(SLO)$/barwipepolypolygon.obj \
        $(SLO)$/boxwipe.obj \
        $(SLO)$/clippingfunctor.obj \
        $(SLO)$/combtransition.obj \
        $(SLO)$/fourboxwipe.obj \
        $(SLO)$/barndoorwipe.obj \
        $(SLO)$/iriswipe.obj \
        $(SLO)$/veewipe.obj \
        $(SLO)$/ellipsewipe.obj \
        $(SLO)$/checkerboardwipe.obj \
        $(SLO)$/randomwipe.obj \
        $(SLO)$/waterfallwipe.obj \
        $(SLO)$/clockwipe.obj \
        $(SLO)$/fanwipe.obj \
        $(SLO)$/pinwheelwipe.obj \
        $(SLO)$/snakewipe.obj \
        $(SLO)$/spiralwipe.obj \
        $(SLO)$/sweepwipe.obj \
        $(SLO)$/figurewipe.obj \
        $(SLO)$/doublediamondwipe.obj \
        $(SLO)$/zigzagwipe.obj \
        $(SLO)$/parametricpolypolygonfactory.obj \
        $(SLO)$/shapetransitionfactory.obj \
        $(SLO)$/slidetransitionfactory.obj \
        $(SLO)$/transitionfactorytab.obj \
        $(SLO)$/transitiontools.obj \
        $(SLO)$/slidechangebase.obj

# ==========================================================================

.INCLUDE :      target.mk
