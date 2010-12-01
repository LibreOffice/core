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

PRJ=..$/..$/..

PRJNAME=sc
TARGET=dbgui
LIBTARGET=no

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
    $(SLO)$/sortdlg.obj		\
    $(SLO)$/tpsort.obj		\
    $(SLO)$/filtdlg.obj		\
    $(SLO)$/sfiltdlg.obj	\
    $(SLO)$/foptmgr.obj		\
    $(SLO)$/pfiltdlg.obj	\
    $(SLO)$/dbnamdlg.obj	\
    $(SLO)$/expftext.obj	\
    $(SLO)$/textimportoptions.obj \
    $(SLO)$/subtdlg.obj		\
    $(SLO)$/tpsubt.obj		\
    $(SLO)$/fieldwnd.obj	\
    $(SLO)$/pvlaydlg.obj	\
    $(SLO)$/pvfundlg.obj	\
    $(SLO)$/dpgroupdlg.obj	\
    $(SLO)$/dapitype.obj	\
    $(SLO)$/dapidata.obj	\
    $(SLO)$/consdlg.obj		\
    $(SLO)$/scendlg.obj		\
    $(SLO)$/imoptdlg.obj	\
    $(SLO)$/validate.obj	\
    $(SLO)$/csvsplits.obj	\
    $(SLO)$/csvcontrol.obj	\
    $(SLO)$/csvruler.obj	\
    $(SLO)$/csvgrid.obj		\
    $(SLO)$/csvtablebox.obj	\
    $(SLO)$/asciiopt.obj	\
    $(SLO)$/scuiasciiopt.obj	\
     $(SLO)$/scuiimoptdlg.obj

EXCEPTIONSFILES= \
    $(SLO)$/csvgrid.obj \
    $(SLO)$/csvruler.obj \
    $(SLO)$/csvsplits.obj \
    $(SLO)$/csvtablebox.obj \
    $(SLO)$/fieldwnd.obj \
    $(SLO)$/pvfundlg.obj	\
    $(SLO)$/pvlaydlg.obj	\
    $(SLO)$/dapidata.obj	\
    $(SLO)$/validate.obj

SRS1NAME=$(TARGET)
SRC1FILES =  \
    textimportoptions.src		\
        pivot.src		\
        pvfundlg.src	\
        dpgroupdlg.src	\
        dapitype.src	\
        consdlg.src		\
        scendlg.src     \
        imoptdlg.src    \
        validate.src    \
        asciiopt.src    \
        outline.src

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =  \
    $(SLO)$/filtdlg.obj		\
    $(SLO)$/sfiltdlg.obj	\
    $(SLO)$/foptmgr.obj		\
    $(SLO)$/dbnamdlg.obj	\
    $(SLO)$/expftext.obj	\
    $(SLO)$/fieldwnd.obj	\
    $(SLO)$/pvlaydlg.obj	\
    $(SLO)$/consdlg.obj		\
    $(SLO)$/imoptdlg.obj	\
    $(SLO)$/csvsplits.obj	\
    $(SLO)$/csvcontrol.obj	\
    $(SLO)$/csvruler.obj	\
    $(SLO)$/csvgrid.obj		\
    $(SLO)$/csvtablebox.obj	\
    $(SLO)$/asciiopt.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk


