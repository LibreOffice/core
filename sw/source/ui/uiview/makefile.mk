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

PRJNAME=sw
TARGET=uiview

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
        view.src \
        pview.src

EXCEPTIONSFILES= \
        $(SLO)$/formatclipboard.obj \
        $(SLO)$/srcview.obj \
        $(SLO)$/swcli.obj  \
        $(SLO)$/uivwimp.obj \
        $(SLO)$/view.obj \
        $(SLO)$/view0.obj \
        $(SLO)$/view1.obj \
        $(SLO)$/view2.obj \
        $(SLO)$/viewdraw.obj \
        $(SLO)$/viewport.obj \
        $(SLO)$/viewprt.obj \
        $(SLO)$/viewsrch.obj \
        $(SLO)$/viewling.obj \
        $(SLO)$/viewmdi.obj \

SLOFILES =  \
        $(SLO)$/view0.obj \
        $(SLO)$/view.obj \
        $(SLO)$/view1.obj \
        $(SLO)$/view2.obj \
        $(SLO)$/viewcoll.obj \
        $(SLO)$/viewdlg2.obj \
        $(SLO)$/viewdlg.obj \
        $(SLO)$/viewdraw.obj \
        $(SLO)$/viewling.obj \
        $(SLO)$/viewmdi.obj \
        $(SLO)$/pview.obj \
        $(SLO)$/viewport.obj \
        $(SLO)$/viewstat.obj \
        $(SLO)$/viewtab.obj \
        $(SLO)$/viewprt.obj \
        $(SLO)$/viewsrch.obj \
        $(SLO)$/scroll.obj \
        $(SLO)$/swcli.obj  \
        $(SLO)$/srcview.obj \
        $(SLO)$/uivwimp.obj \
        $(SLO)$/formatclipboard.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

$(SRS)$/uiview.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc

