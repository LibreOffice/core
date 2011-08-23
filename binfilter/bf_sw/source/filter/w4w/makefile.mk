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
TARGET=sw_w4w

#GEN_HID=TRUE
#GEN_HID_OTHER=TRUE
NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

#.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  $(PRJ)$/inc$/bf_sw$/swpre.mk
.INCLUDE :  settings.mk
#.INCLUDE :  $(PRJ)$/inc$/sw.mk
.INCLUDE :  $(PRJ)$/inc$/bf_sw$/sw.mk

INC+= -I$(PRJ)$/inc$/bf_sw
.IF "$(mydebug)" != ""
CDEFS=$(CDEFS) -Dmydebug
.ENDIF

# --- Files --------------------------------------------------------

CXXFILES = \
        sw_w4watr.cxx \
        sw_w4wgraf.cxx \
        sw_w4wpar1.cxx \
        sw_w4wpar2.cxx \
        sw_w4wpar3.cxx \
        sw_w4wstk.cxx \
        sw_wrtgraf.cxx \
        sw_wrtw4w.cxx



SLOFILES =  \
        $(SLO)$/sw_w4watr.obj \
        $(SLO)$/sw_w4wgraf.obj \
        $(SLO)$/sw_w4wpar1.obj \
        $(SLO)$/sw_w4wpar2.obj \
        $(SLO)$/sw_w4wpar3.obj \
        $(SLO)$/sw_w4wstk.obj \
        $(SLO)$/sw_wrtgraf.obj \
        $(SLO)$/sw_wrtw4w.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

