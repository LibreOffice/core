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
# $Revision: 1.15.18.1 $
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

PRJNAME=svx
TARGET=engine3d
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES= \
        $(SLO)$/e3dsceneupdater.obj \
        $(SLO)$/helperminimaldepth3d.obj	\
        $(SLO)$/helperhittest3d.obj			\
        $(SLO)$/obj3d.obj 		\
        $(SLO)$/scene3d.obj		\
        $(SLO)$/polysc3d.obj	\
        $(SLO)$/cube3d.obj		\
        $(SLO)$/sphere3d.obj	\
        $(SLO)$/extrud3d.obj    \
        $(SLO)$/lathe3d.obj		\
        $(SLO)$/polygn3d.obj 	\
        $(SLO)$/svx3ditems.obj	\
        $(SLO)$/deflt3d.obj		\
        $(SLO)$/e3dundo.obj     \
        $(SLO)$/volume3d.obj	\
        $(SLO)$/viewpt3d.obj	\
        $(SLO)$/viewpt3d2.obj	\
        $(SLO)$/camera3d.obj	\
        $(SLO)$/objfac3d.obj 	\
        $(SLO)$/dragmt3d.obj	\
        $(SLO)$/view3d.obj		\
        $(SLO)$/view3d1.obj     \
        $(SLO)$/float3d.obj

#disable optimizer for MSCompiler and ICC
.IF "$(COM)"=="ICC" || "$(COM)"=="MSC"
NOOPTFILES=\
        $(SLO)$/viewpt3d.obj
.ENDIF

SRS1NAME=engine3d
SRC1FILES=\
        string3d.src	\
        float3d.src

.INCLUDE :  target.mk
