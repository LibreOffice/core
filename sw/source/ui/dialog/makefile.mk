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
TARGET=dialog
LIBTARGET=no

# future: DEMO\dialog.srs

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
        abstract.src \
        ascfldlg.src \
        dialog.src \
        docstdlg.src \
        regionsw.src \
        wordcountdialog.src

EXCEPTIONSFILES = \
        $(SLO)$/ascfldlg.obj \
        $(SLO)$/SwSpellDialogChildWindow.obj

SLOFILES =  \
        $(SLO)$/abstract.obj \
        $(SLO)$/addrdlg.obj \
        $(SLO)$/docstdlg.obj \
        $(SLO)$/macassgn.obj \
        $(SLO)$/SwSpellDialogChildWindow.obj \
        $(SLO)$/regionsw.obj	\
        $(SLO)$/uiregionsw.obj	\
        $(SLO)$/swabstdlg.obj	\
        $(SLO)$/swuiexp.obj		\
        $(SLO)$/swwrtshitem.obj \
        $(SLO)$/swdialmgr.obj  \
        $(SLO)$/wordcountdialog.obj \
        $(SLO)$/swdlgfact.obj

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =  \
        $(SLO)$/regionsw.obj \
        $(SLO)$/swabstdlg.obj \
        $(SLO)$/SwSpellDialogChildWindow.obj \
        $(SLO)$/swwrtshitem.obj

#		$(SLO)$/macassgn.obj \

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
$(INCCOM)$/swuilib.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
    echo \#define DLL_NAME \"libswui$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
    echo \#define DLL_NAME \"swui$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ENDIF

$(SLO)$/swabstdlg.obj : $(INCCOM)$/swuilib.hxx







