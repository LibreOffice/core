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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..$/..
BFPRJ=..$/..

PRJNAME=binfilter
TARGET=svx_engine3d

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_svx

# --- Files --------------------------------------------------------

SRS1NAME=svx_engine3d
SRC1FILES=\
        svx_float3d.src

SLOFILES= \
        $(SLO)$/svx_svx3ditems.obj	\
        $(SLO)$/svx_deflt3d.obj		\
        $(SLO)$/svx_class3d.obj		\
        $(SLO)$/svx_volume3d.obj		\
        $(SLO)$/svx_viewpt3d.obj		\
        $(SLO)$/svx_camera3d.obj		\
        $(SLO)$/svx_poly3d.obj 		\
        $(SLO)$/svx_polygn3d.obj 		\
        $(SLO)$/svx_objfac3d.obj 	\
        $(SLO)$/svx_obj3d.obj 		\
        $(SLO)$/svx_light3d.obj 		\
        $(SLO)$/svx_dlight3d.obj 	\
        $(SLO)$/svx_plight3d.obj 	\
        $(SLO)$/svx_pntobj3d.obj     \
        $(SLO)$/svx_polyob3d.obj		\
        $(SLO)$/svx_scene3d.obj		\
        $(SLO)$/svx_polysc3d.obj		\
        $(SLO)$/svx_cube3d.obj		\
        $(SLO)$/svx_sphere3d.obj		\
        $(SLO)$/svx_extrud3d.obj     \
        $(SLO)$/svx_lathe3d.obj		\
        $(SLO)$/svx_label3d.obj		\
        $(SLO)$/svx_view3d.obj		\
        $(SLO)$/svx_e3dcmpt.obj       

.INCLUDE :  target.mk

