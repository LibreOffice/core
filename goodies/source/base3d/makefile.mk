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

PRJ=..$/..

PRJNAME=goodies
TARGET=base3d
AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------
EXCEPTIONSFILES=\
                $(SLO)$/b3dgeom.obj

SLOFILES= \
                $(SLO)$/base3d.obj      \
                $(SLO)$/b3dtrans.obj    \
                $(SLO)$/b3ddeflt.obj    \
                $(SLO)$/b3dopngl.obj    \
                $(SLO)$/b3dcommn.obj    \
                $(SLO)$/matril3d.obj	\
                $(SLO)$/b3dcolor.obj	\
                $(SLO)$/b3dlight.obj	\
                $(SLO)$/b3dentty.obj	\
                $(SLO)$/b3dcompo.obj	\
                $(SLO)$/b3dtex.obj		\
                $(SLO)$/b3dgeom.obj		


.INCLUDE :  target.mk

