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
TARGET=sw_txtnode

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/bf_sw$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/bf_sw$/sw.mk
INC+= -I$(PRJ)$/inc$/bf_sw
.IF "$(GUI)"!="OS2"
INCEXT=s:$/solar$/inc$/hm
.ENDIF

.IF "$(mydebug)" != ""
CDEFS+=-Dmydebug
.ENDIF

.IF "$(GUI)$(COM)" == "WINMSC"
LIBFLAGS=/NOI /NOE /PAGE:512
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/sw_atrfld.obj \
        $(SLO)$/sw_atrflyin.obj \
        $(SLO)$/sw_atrftn.obj \
        $(SLO)$/sw_atrref.obj \
        $(SLO)$/sw_atrtox.obj \
        $(SLO)$/sw_fmtatr1.obj \
        $(SLO)$/sw_fmtatr2.obj \
        $(SLO)$/sw_fntcap.obj \
        $(SLO)$/sw_fntcache.obj \
        $(SLO)$/sw_swfntcch.obj \
        $(SLO)$/sw_ndhints.obj \
        $(SLO)$/sw_ndtxt.obj \
        $(SLO)$/sw_swfont.obj \
        $(SLO)$/sw_thints.obj \
        $(SLO)$/sw_txatbase.obj \
        $(SLO)$/sw_txtatr2.obj \
        $(SLO)$/sw_txtedt.obj

.IF "$(dbutil)" != ""
OFILES+=$(SLO)$/sw_dbchratr.$(QBJX)
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

