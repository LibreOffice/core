#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.10 $
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

PRJNAME=sfx2
TARGET=control
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# w.g. compilerbugs
.IF "$(GUI)"=="WNT"
.IF "$(COM)"!="GCC"
CFLAGS+=-Od
.ENDIF
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/bindings.obj \
    $(SLO)$/ctrlitem.obj \
    $(SLO)$/dispatch.obj \
    $(SLO)$/macrconf.obj \
        $(SLO)$/macro.obj \
    $(SLO)$/minfitem.obj \
    $(SLO)$/msg.obj \
       $(SLO)$/msgpool.obj \
       $(SLO)$/objface.obj \
    $(SLO)$/querystatus.obj \
    $(SLO)$/request.obj \
        $(SLO)$/sfxstatuslistener.obj \
    $(SLO)$/shell.obj \
    $(SLO)$/sorgitm.obj \
    $(SLO)$/statcach.obj \
        $(SLO)$/unoctitm.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

