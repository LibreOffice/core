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
TARGET=sc_unoobj

NO_HIDS=TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_sc
# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/sc_docuno.obj \
        $(SLO)$/sc_servuno.obj \
        $(SLO)$/sc_defltuno.obj \
        $(SLO)$/sc_drdefuno.obj \
        $(SLO)$/sc_cellsuno.obj \
        $(SLO)$/sc_textuno.obj \
        $(SLO)$/sc_notesuno.obj \
        $(SLO)$/sc_cursuno.obj \
        $(SLO)$/sc_fielduno.obj \
        $(SLO)$/sc_miscuno.obj \
        $(SLO)$/sc_optuno.obj \
        $(SLO)$/sc_appluno.obj \
        $(SLO)$/sc_funcuno.obj \
        $(SLO)$/sc_nameuno.obj \
        $(SLO)$/sc_datauno.obj \
        $(SLO)$/sc_dapiuno.obj \
        $(SLO)$/sc_chartuno.obj \
        $(SLO)$/sc_shapeuno.obj \
        $(SLO)$/sc_pageuno.obj \
        $(SLO)$/sc_forbiuno.obj \
        $(SLO)$/sc_styleuno.obj \
        $(SLO)$/sc_afmtuno.obj \
        $(SLO)$/sc_fmtuno.obj \
        $(SLO)$/sc_linkuno.obj \
        $(SLO)$/sc_targuno.obj \
        $(SLO)$/sc_convuno.obj \
        $(SLO)$/sc_editsrc.obj \
        $(SLO)$/sc_unoguard.obj \
        $(SLO)$/sc_confuno.obj \
        $(SLO)$/sc_filtuno.obj \
        $(SLO)$/sc_unodoc.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

