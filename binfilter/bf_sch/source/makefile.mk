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

PRJ=..$/..
BFPRJ=..

PROJECTPCH=sch
PROJECTPCHSOURCE=$(BFPRJ)$/util\sch_sch

PRJNAME=binfilter
TARGET=sch

#GEN_HID=TRUE
#GEN_HID_OTHER=TRUE
NO_HIDS=TRUE

LIBTARGET=NO

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk
INC+= -I$(PRJ)$/inc$/bf_sch
.IF "$(GUI)$(COM)" == "WINMSC"
LIBFLAGS+=/PAGE:128
.ENDIF


# --- Files --------------------------------------------------------

# --- ui Lib

LIB1TARGET= $(SLB)$/ui.lib
LIB1FILES= \
    $(SLB)$/app.lib \
    $(SLB)$/dlg.lib \
    $(SLB)$/docshell.lib \
    $(SLB)$/func.lib \
    $(SLB)$/view.lib \
    $(SLB)$/chxchart.lib 


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
