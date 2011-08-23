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
TARGET=svx_unoedit

NO_HIDS=TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_svx

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/svx_UnoForbiddenCharsTable.obj \
        $(SLO)$/svx_unoedsrc.obj	\
        $(SLO)$/svx_unoedhlp.obj	\
        $(SLO)$/svx_unoedprx.obj	\
        $(SLO)$/svx_unoviwed.obj	\
        $(SLO)$/svx_unoviwou.obj	\
        $(SLO)$/svx_unofored.obj	\
        $(SLO)$/svx_unoforou.obj	\
        $(SLO)$/svx_unotext.obj		\
        $(SLO)$/svx_unotext2.obj	\
        $(SLO)$/svx_unofield.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

