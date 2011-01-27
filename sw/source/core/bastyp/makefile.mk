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
TARGET=bastyp

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

CXXFILES = \
        bparr.cxx \
        breakit.cxx \
        calc.cxx \
        checkit.cxx \
        index.cxx \
        init.cxx \
        ring.cxx \
        swcache.cxx \
        swrect.cxx \
        swregion.cxx \
        swtypes.cxx \
        tabcol.cxx \
        SwSmartTagMgr.cxx

SLOFILES =	\
        $(EXCEPTIONSFILES) \
        $(SLO)$/bparr.obj \
        $(SLO)$/checkit.obj \
        $(SLO)$/index.obj \
        $(SLO)$/ring.obj \
        $(SLO)$/swcache.obj \
        $(SLO)$/swrect.obj \
        $(SLO)$/swregion.obj

EXCEPTIONSFILES = \
        $(SLO)$/SwSmartTagMgr.obj \
        $(SLO)$/breakit.obj \
        $(SLO)$/calc.obj \
        $(SLO)$/init.obj \
        $(SLO)$/swtypes.obj \
        $(SLO)$/tabcol.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

