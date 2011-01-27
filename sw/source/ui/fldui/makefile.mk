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
LIBTARGET=no
PRJNAME=sw
TARGET=fldui

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
        changedb.src \
        DropDownFieldDialog.src\
        flddb.src \
        flddinf.src \
        flddok.src \
        fldfunc.src \
        fldref.src \
        fldtdlg.src \
        fldui.src \
        fldvar.src \
        javaedit.src \
        inpdlg.src

SLOFILES =  \
        $(EXCEPTIONSFILES)

EXCEPTIONSFILES =  \
        $(SLO)$/DropDownFieldDialog.obj \
        $(SLO)$/FldRefTreeListBox.obj \
        $(SLO)$/changedb.obj \
        $(SLO)$/flddb.obj \
        $(SLO)$/flddinf.obj \
        $(SLO)$/flddok.obj \
        $(SLO)$/fldedt.obj \
        $(SLO)$/fldfunc.obj \
        $(SLO)$/fldmgr.obj \
        $(SLO)$/fldpage.obj \
        $(SLO)$/fldref.obj \
        $(SLO)$/fldtdlg.obj \
        $(SLO)$/fldvar.obj \
        $(SLO)$/fldwrap.obj \
        $(SLO)$/inpdlg.obj \
        $(SLO)$/javaedit.obj \
        $(SLO)$/xfldui.obj

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =	\
        $(SLO)$/fldmgr.obj \
        $(SLO)$/fldwrap.obj \
        $(SLO)$/xfldui.obj

#		$(SLO)$/DropDownFieldDialog.obj \

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

