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
# $Revision: 1.15 $
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

PRJNAME=basic
TARGET=app

LIBTARGET = NO

# --- Settings ------------------------------------------------------------

.INCLUDE :  settings.mk

# --- Common ------------------------------------------------------------

OBJFILES = \
    $(OBJ)$/ttbasic.obj	\
    $(OBJ)$/basicrt.obj	\
    $(OBJ)$/processw.obj    \
    $(OBJ)$/process.obj     \
    $(OBJ)$/brkpnts.obj     \
    $(OBJ)$/mybasic.obj     \
    $(OBJ)$/status.obj      \
    $(OBJ)$/printer.obj     \
    $(OBJ)$/appwin.obj      \
    $(OBJ)$/appedit.obj     \
    $(OBJ)$/appbased.obj    \
    $(OBJ)$/apperror.obj    \
    $(OBJ)$/textedit.obj    \
    $(OBJ)$/msgedit.obj     \
    $(OBJ)$/dialogs.obj     \

EXCEPTIONSFILES = \
    $(OBJ)$/app.obj \
    $(OBJ)$/printer.obj     \
    $(OBJ)$/process.obj

.IF "$(GUI)" == "WNT"
EXCEPTIONSFILES += \
    $(OBJ)$/process.obj
.ENDIF

SRS1NAME=$(TARGET)
SRC1FILES = \
    basic.src				\
    ttmsg.src				\
    basmsg.src				\
    svtmsg.src				\
    testtool.src

LIB1TARGET=$(LB)$/app.lib
LIB1ARCHIV=$(LB)$/libapp.a
LIB1OBJFILES = \
        $(OBJ)$/basicrt.obj	\
        $(OBJ)$/processw.obj     \
        $(OBJ)$/process.obj      \
        $(OBJ)$/brkpnts.obj      \
        $(OBJ)$/app.obj          \
        $(OBJ)$/mybasic.obj	\
        $(OBJ)$/status.obj       \
        $(OBJ)$/printer.obj      \
        $(OBJ)$/appwin.obj       \
        $(OBJ)$/appedit.obj      \
        $(OBJ)$/appbased.obj     \
        $(OBJ)$/apperror.obj     \
        $(OBJ)$/textedit.obj     \
        $(OBJ)$/msgedit.obj      \
        $(OBJ)$/dialogs.obj      \
        $(OBJ)$/sbintern.obj

# --- Targets ------------------------------------------------------------

.INCLUDE :  target.mk

$(OBJ)$/dialogs.obj : $(INCCOM)$/_version.h

