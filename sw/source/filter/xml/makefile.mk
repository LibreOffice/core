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
TARGET=xml

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(mydebug)" != ""
CDEFS=$(CDEFS) -Dmydebug
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES =  \
        $(EXCEPTIONSFILES) \
        $(SLO)$/xmlimpit.obj \
        $(SLO)$/xmlitemm.obj \
        $(SLO)$/xmlitmpr.obj

EXCEPTIONSFILES= \
        $(SLO)$/XMLRedlineImportHelper.obj \
        $(SLO)$/swxml.obj \
        $(SLO)$/wrtxml.obj \
        $(SLO)$/xmlbrsh.obj \
        $(SLO)$/xmlexp.obj \
        $(SLO)$/xmlexpit.obj \
        $(SLO)$/xmlfmt.obj \
        $(SLO)$/xmlfmte.obj \
        $(SLO)$/xmlfonte.obj \
        $(SLO)$/xmlimp.obj \
        $(SLO)$/xmlitem.obj \
        $(SLO)$/xmliteme.obj \
        $(SLO)$/xmlitemi.obj \
        $(SLO)$/xmlithlp.obj \
        $(SLO)$/xmlmeta.obj \
        $(SLO)$/xmlscript.obj \
        $(SLO)$/xmltble.obj \
        $(SLO)$/xmltbli.obj \
        $(SLO)$/xmltext.obj \
        $(SLO)$/xmltexte.obj \
        $(SLO)$/xmltexti.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
