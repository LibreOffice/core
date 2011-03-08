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
PRJ=..$/..

PRJNAME=editeng
TARGET=editeng
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- General ----------------------------------------------------------

.IF "$(editdebug)" != "" || "$(EDITDEBUG)" != ""
CDEFS+=-DEDITDEBUG
.ENDIF

SLOFILES =	\
            $(SLO)$/textconv.obj \
            $(SLO)$/editattr.obj \
            $(SLO)$/editdbg.obj	\
            $(SLO)$/editdoc.obj	\
            $(SLO)$/editdoc2.obj \
            $(SLO)$/editeng.obj	\
            $(SLO)$/editobj.obj	\
            $(SLO)$/editsel.obj	\
            $(SLO)$/editundo.obj \
            $(SLO)$/editview.obj  \
            $(SLO)$/edtspell.obj \
            $(SLO)$/eehtml.obj \
            $(SLO)$/eerdll.obj	\
            $(SLO)$/eeobj.obj \
            $(SLO)$/eertfpar.obj \
            $(SLO)$/impedit.obj	\
            $(SLO)$/impedit2.obj \
            $(SLO)$/impedit3.obj \
            $(SLO)$/impedit4.obj \
            $(SLO)$/impedit5.obj

SRS1NAME=$(TARGET)
SRC1FILES=	editeng.src

EXCEPTIONSFILES= \
    $(SLO)$/unolingu.obj

.INCLUDE :  target.mk

