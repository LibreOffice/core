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

PRJNAME=svx
TARGET=tbxctrls
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET)-core.lib
LIB1OBJFILES= \
        $(SLO)$/fontworkgallery.obj\
        $(SLO)$/extrusioncontrols.obj \
        $(SLO)$/tbcontrl.obj	\
        $(SLO)$/tbxcolorupdate.obj

LIB2TARGET= $(SLB)$/$(TARGET).lib
LIB2OBJFILES= \
        $(SLO)$/formatpaintbrushctrl.obj\
        $(SLO)$/colrctrl.obj	\
        $(SLO)$/fillctrl.obj	\
        $(SLO)$/grafctrl.obj	\
        $(SLO)$/itemwin.obj		\
        $(SLO)$/layctrl.obj		\
        $(SLO)$/lboxctrl.obj	\
        $(SLO)$/linectrl.obj	\
        $(SLO)$/tbxalign.obj	\
        $(SLO)$/tbxdrctl.obj    \
        $(SLO)$/verttexttbxctrl.obj \
        $(SLO)$/subtoolboxcontrol.obj \
        $(SLO)$/tbxcolor.obj \
        $(SLO)$/tbunocontroller.obj \
        $(SLO)$/tbunosearchcontrollers.obj

SLOFILES = $(LIB1OBJFILES) $(LIB2OBJFILES)

SRS1NAME=$(TARGET)
SRC1FILES = \
        fontworkgallery.src		\
        extrusioncontrols.src		\
        colrctrl.src			\
        lboxctrl.src			\
        linectrl.src			\
        tbcontrl.src			\
        tbunosearchcontrollers.src			\
        grafctrl.src

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

