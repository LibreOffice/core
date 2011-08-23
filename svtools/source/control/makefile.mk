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

PRJNAME=svtools
TARGET=ctrl

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/svt.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES=\
        ctrltool.src \
        ctrlbox.src \
        calendar.src \
        filectrl.src

EXCEPTIONSFILES=\
        $(SLO)$/asynclink.obj \
        $(SLO)$/calendar.obj \
        $(SLO)$/ctrlbox.obj \
        $(SLO)$/ctrltool.obj \
        $(SLO)$/filectrl.obj \
        $(SLO)$/filectrl2.obj \
        $(SLO)$/fileurlbox.obj \
        $(SLO)$/fixedhyper.obj \
        $(SLO)$/fmtfield.obj \
        $(SLO)$/headbar.obj \
        $(SLO)$/hyperlabel.obj \
        $(SLO)$/inettbc.obj \
        $(SLO)$/prgsbar.obj \
        $(SLO)$/roadmap.obj \
        $(SLO)$/ruler.obj \
        $(SLO)$/scriptedtext.obj \
        $(SLO)$/scrwin.obj \
        $(SLO)$/stdctrl.obj \
        $(SLO)$/stdmenu.obj \
        $(SLO)$/svxbox.obj \
        $(SLO)$/tabbar.obj \
        $(SLO)$/taskbar.obj \
        $(SLO)$/taskbox.obj \
        $(SLO)$/taskmisc.obj \
        $(SLO)$/taskstat.obj \
        $(SLO)$/toolbarmenu.obj \
        $(SLO)$/toolbarmenuacc.obj \
        $(SLO)$/urlcontrol.obj \
        $(SLO)$/valueacc.obj \
        $(SLO)$/valueset.obj

SLOFILES=\
      $(EXCEPTIONSFILES) \
        $(SLO)$/collatorres.obj \
        $(SLO)$/indexentryres.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

