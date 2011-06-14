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
PRJ=..$/..

PROJECTPCH4DLL=TRUE
PROJECTPCH=svxpch
PROJECTPCHSOURCE=$(PRJ)$/util$/svxpch

PRJNAME=svx
TARGET=stbctrls
AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES = \
        stbctrls.src

SLOFILES=	\
        $(EXCEPTIONSFILES)

EXCEPTIONSFILES= \
        $(SLO)$/insctrl.obj	\
        $(SLO)$/modctrl.obj	\
        $(SLO)$/pszctrl.obj	\
        $(SLO)$/selctrl.obj	\
        $(SLO)$/xmlsecctrl.obj	\
        $(SLO)$/zoomctrl.obj	\
        $(SLO)$/zoomsliderctrl.obj

HXX1TARGET=stbctrls
HXX1EXT=   hxx
HXX1EXCL=  -E:*include*
HXX1DEPN=\
        $(INC)$/insctrl.hxx	\
        $(INC)$/zoomctrl.hxx	\
        $(INC)$/pszctrl.hxx	\
        $(INC)$/selctrl.hxx	\
        $(INC)$/modctrl.hxx \
        $(INC)$/xmlsecctrl.hxx \
        $(INC)$/zoomsliderctrl.hxx \

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

