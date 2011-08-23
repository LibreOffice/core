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

PRJ=..$/..$/..
BFPRJ=..$/..

PRJNAME=binfilter
TARGET=svx_editeng

NO_HIDS=TRUE

PROJECTPCH4DLL=TRUE
PROJECTPCH=eeng_pch
PROJECTPCHSOURCE=svx_eeng_pch

ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_svx

# --- Allgemein ----------------------------------------------------------

.IF "$(editdebug)" != "" || "$(EDITDEBUG)" != ""
CDEFS+=-DEDITDEBUG
.ENDIF

SLOFILES =	\
            $(SLO)$/svx_txtrange.obj \
            $(SLO)$/svx_forbiddencharacterstable.obj \
            $(SLO)$/svx_swafopt.obj \
            $(SLO)$/svx_editattr.obj \
            $(SLO)$/svx_editdbg.obj	\
            $(SLO)$/svx_editdoc.obj	\
            $(SLO)$/svx_editdoc2.obj \
            $(SLO)$/svx_editeng.obj	\
            $(SLO)$/svx_editobj.obj	\
            $(SLO)$/svx_editsel.obj	\
            $(SLO)$/svx_editundo.obj \
            $(SLO)$/svx_editview.obj  \
            $(SLO)$/svx_eerdll.obj	\
            $(SLO)$/svx_impedit.obj	\
            $(SLO)$/svx_impedit2.obj \
            $(SLO)$/svx_impedit3.obj \
            $(SLO)$/svx_impedit4.obj \
            $(SLO)$/svx_impedit5.obj \
            $(SLO)$/svx_unolingu.obj

SRS1NAME=$(TARGET)
SRC1FILES=	svx_editeng.src

EXCEPTIONSFILES= \
    $(SLO)$/svx_unolingu.obj

.INCLUDE :  target.mk

