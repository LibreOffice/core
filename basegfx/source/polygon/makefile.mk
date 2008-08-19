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
# $Revision: 1.12 $
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
PRJNAME=basegfx
TARGET=polygon

#UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb
ENABLE_EXCEPTIONS=TRUE
#USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Files -------------------------------------

SLOFILES= \
        $(SLO)$/b2dpolygon.obj						\
        $(SLO)$/b2dpolygontools.obj					\
        $(SLO)$/b2dpolypolygon.obj					\
        $(SLO)$/b2dpolypolygontools.obj				\
        $(SLO)$/b2dsvgpolypolygon.obj				\
        $(SLO)$/b2dlinegeometry.obj					\
        $(SLO)$/b2dpolypolygoncutter.obj			\
        $(SLO)$/b2dpolypolygonrasterconverter.obj	\
        $(SLO)$/b2dpolygonclipper.obj				\
        $(SLO)$/b2dpolygontriangulator.obj			\
        $(SLO)$/b2dpolygoncutandtouch.obj			\
        $(SLO)$/b3dpolygon.obj						\
        $(SLO)$/b3dpolygontools.obj					\
        $(SLO)$/b3dpolypolygon.obj					\
        $(SLO)$/b3dpolypolygontools.obj				\
        $(SLO)$/b3dpolygonclipper.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk
