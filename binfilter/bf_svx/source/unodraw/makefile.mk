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
TARGET=svx_unodraw

NO_HIDS=TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_svx

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/svx_UnoNamespaceMap.obj \
        $(SLO)$/svx_unopool.obj \
        $(SLO)$/svx_XPropertyTable.obj \
        $(SLO)$/svx_UnoNameItemTable.obj \
        $(SLO)$/svx_unoshape.obj	\
        $(SLO)$/svx_unoipset.obj	\
        $(SLO)$/svx_unoctabl.obj	\
        $(SLO)$/svx_unoshap2.obj	\
        $(SLO)$/svx_unoshap3.obj	\
        $(SLO)$/svx_unoshap4.obj	\
        $(SLO)$/svx_unopage.obj		\
        $(SLO)$/svx_unoshtxt.obj	\
        $(SLO)$/svx_unoshcol.obj	\
        $(SLO)$/svx_unoprov.obj		\
        $(SLO)$/svx_unomod.obj      \
        $(SLO)$/svx_unonrule.obj	\
        $(SLO)$/svx_unofdesc.obj	\
        $(SLO)$/svx_unomlstr.obj	\
        $(SLO)$/svx_unogtabl.obj	\
        $(SLO)$/svx_unohtabl.obj	\
        $(SLO)$/svx_unobtabl.obj	\
        $(SLO)$/svx_unottabl.obj	\
        $(SLO)$/svx_unomtabl.obj	\
        $(SLO)$/svx_unodtabl.obj	\
        $(SLO)$/svx_gluepts.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

