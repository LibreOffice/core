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
TARGET=html


# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(mydebug)" != ""
CDEFS=$(CDEFS) -Dmydebug
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/css1atr.obj \
        $(SLO)$/css1kywd.obj \
        $(SLO)$/htmlatr.obj \
        $(SLO)$/htmlbas.obj \
        $(SLO)$/htmlcss1.obj \
        $(SLO)$/htmlctxt.obj \
        $(SLO)$/htmldraw.obj \
        $(SLO)$/htmlfld.obj \
        $(SLO)$/htmlfldw.obj \
        $(SLO)$/htmlfly.obj \
        $(SLO)$/htmlflyt.obj \
        $(SLO)$/htmlform.obj \
        $(SLO)$/htmlforw.obj \
        $(SLO)$/htmlftn.obj \
        $(SLO)$/htmlgrin.obj \
        $(SLO)$/htmlnum.obj \
        $(SLO)$/htmlplug.obj \
        $(SLO)$/htmlsect.obj \
        $(SLO)$/htmltab.obj \
        $(SLO)$/htmltabw.obj \
        $(SLO)$/parcss1.obj \
        $(SLO)$/svxcss1.obj \
        $(SLO)$/swhtml.obj \
        $(SLO)$/wrthtml.obj \
        $(SLO)$/SwAppletImpl.obj \

EXCEPTIONSFILES = \
        $(SLO)$/htmlatr.obj \
        $(SLO)$/htmlfld.obj \
        $(SLO)$/htmlgrin.obj \
        $(SLO)$/htmlplug.obj \
        $(SLO)$/htmlsect.obj \
        $(SLO)$/swhtml.obj \
        $(SLO)$/wrthtml.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

