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

PRJ=..$/..$/..$/..
BFPRJ=..$/..$/..

PRJNAME=binfilter
TARGET=sw_bastyp

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/bf_sw$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/bf_sw$/sw.mk
INC+= -I$(PRJ)$/inc$/bf_sw
# --- Files --------------------------------------------------------

CXXFILES = \
        sw_bparr.cxx \
        sw_breakit.cxx \
        sw_calc.cxx \
                sw_checkit.cxx \
        sw_index.cxx \
        sw_init.cxx \
        sw_ring.cxx \
        sw_swcache.cxx \
        sw_swrect.cxx \
        sw_swregion.cxx \
        sw_swtypes.cxx

SLOFILES =  \
        $(SLO)$/sw_bparr.obj \
        $(SLO)$/sw_breakit.obj \
        $(SLO)$/sw_calc.obj \
                $(SLO)$/sw_checkit.obj \
        $(SLO)$/sw_index.obj \
        $(SLO)$/sw_init.obj \
        $(SLO)$/sw_ring.obj \
        $(SLO)$/sw_swcache.obj \
        $(SLO)$/sw_swrect.obj \
        $(SLO)$/sw_swregion.obj \
        $(SLO)$/sw_swtypes.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

