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
TARGET=crsr

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/inc$/swpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/inc$/sw.mk

.IF "$(mydebug)" != ""
CDEFS+=-Dmydebug
.ENDIF

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=    \
    $(SLO)$/BlockCursor.obj \
    $(SLO)$/bookmrk.obj \
    $(SLO)$/callnk.obj \
    $(SLO)$/crbm.obj \
    $(SLO)$/crossrefbookmark.obj \
    $(SLO)$/crsrsh.obj \
    $(SLO)$/crstrvl.obj \
    $(SLO)$/crstrvl1.obj \
    $(SLO)$/findattr.obj \
    $(SLO)$/findcoll.obj \
    $(SLO)$/findfmt.obj \
    $(SLO)$/findtxt.obj \
    $(SLO)$/pam.obj \
    $(SLO)$/paminit.obj \
    $(SLO)$/swcrsr.obj \
    $(SLO)$/trvlcol.obj \
    $(SLO)$/trvlfnfl.obj \
    $(SLO)$/trvlreg.obj \
    $(SLO)$/trvltbl.obj \
    $(SLO)$/unocrsr.obj \
    $(SLO)$/viscrs.obj

SLOFILES =	\
    $(EXCEPTIONSFILES)

.IF "$(mydebug)" != ""
SLOFILES +=  \
        $(SLO)$/pamio.obj
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :	target.mk
