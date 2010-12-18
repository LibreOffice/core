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
TARGET=svcontnr

PROJECTPCH4DLL=TRUE
PROJECTPCH=cont_pch
PROJECTPCHSOURCE=cont_pch

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/svt.pmk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=\
            $(SLO)$/contentenumeration.obj	\
            $(SLO)$/fileview.obj	\
            $(SLO)$/imivctl1.obj	\
            $(SLO)$/imivctl2.obj	\
            $(SLO)$/ivctrl.obj		\
            $(SLO)$/svicnvw.obj		\
            $(SLO)$/svimpbox.obj	\
            $(SLO)$/svimpicn.obj	\
            $(SLO)$/svlbitm.obj		\
            $(SLO)$/svlbox.obj		\
            $(SLO)$/svtabbx.obj		\
            $(SLO)$/svtreebx.obj	\
            $(SLO)$/templwin.obj	\
            $(SLO)$/tooltiplbox.obj

SLOFILES=   $(EXCEPTIONSFILES) \
            $(SLO)$/treelist.obj

SRS1NAME=$(TARGET)
SRC1FILES =\
            fileview.src	\
            templwin.src	\
            svcontnr.src

HXX1TARGET=	svcontnr
HXX1EXT=	hxx
HXX1FILES=	$(PRJ)$/inc$/svlbox.hxx \
        $(PRJ)$/inc$/svlbitm.hxx \
        $(PRJ)$/inc$/svtreebx.hxx \
        $(PRJ)$/inc$/svicnvw.hxx \
        $(PRJ)$/inc$/svtabbx.hxx \
        $(PRJ)$/inc$/treelist.hxx
HXX1EXCL=	-E:*include*

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

