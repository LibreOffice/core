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

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=dlg
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =\
    masterlayoutdlg.src\
    headerfooterdlg.src\
    ins_paste.src\
    inspagob.src\
    present.src\
    dlg_char.src\
    paragr.src\
    tabtempl.src\
    dlgpage.src\
    tpaction.src\
    animobjs.src\
    prltempl.src\
    prntopts.src\
    dlgsnap.src\
    copydlg.src\
    tpoption.src\
    dlgass.src\
    navigatr.src\
    sdpreslt.src\
    morphdlg.src\
    dlgfield.src\
    custsdlg.src\
    brkdlg.src\
    vectdlg.src\
    dlgolbul.src \
    LayerDialog.src			\
    PaneDockingWindow.src \
    layeroptionsdlg.src

SLOFILES =  \
        $(SLO)$/inspagob.obj \
        $(SLO)$/ins_paste.obj \
        $(SLO)$/present.obj  \
        $(SLO)$/dlgctrls.obj \
        $(SLO)$/dlgchar.obj \
        $(SLO)$/paragr.obj \
        $(SLO)$/tabtempl.obj \
        $(SLO)$/dlgpage.obj  \
        $(SLO)$/diactrl.obj  \
        $(SLO)$/gluectrl.obj  \
        $(SLO)$/sdtreelb.obj \
        $(SLO)$/animobjs.obj \
        $(SLO)$/prltempl.obj \
        $(SLO)$/prntopts.obj \
        $(SLO)$/dlgsnap.obj  \
        $(SLO)$/copydlg.obj  \
        $(SLO)$/dlgolbul.obj \
        $(SLO)$/tpaction.obj \
        $(SLO)$/tpoption.obj \
        $(SLO)$/filedlg.obj   \
        $(SLO)$/unchss.obj  \
        $(SLO)$/dlgassim.obj	\
        $(SLO)$/dlgass.obj  \
        $(SLO)$/assclass.obj \
        $(SLO)$/navigatr.obj \
        $(SLO)$/sdpreslt.obj \
        $(SLO)$/morphdlg.obj	\
        $(SLO)$/dlgfield.obj	\
        $(SLO)$/custsdlg.obj	\
        $(SLO)$/brkdlg.obj      \
        $(SLO)$/vectdlg.obj		\
        $(SLO)$/docprev.obj     \
        $(SLO)$/SpellDialogChildWindow.obj \
        $(SLO)$/AnimationChildWindow.obj	\
        $(SLO)$/LayerDialogChildWindow.obj	\
        $(SLO)$/LayerDialogContent.obj		\
        $(SLO)$/LayerTabBar.obj				\
        $(SLO)$/NavigatorChildWindow.obj	\
        $(SLO)$/TemplateScanner.obj \
        $(SLO)$/sduiexp.obj \
        $(SLO)$/sddlgfact.obj \
        $(SLO)$/sdabstdlg.obj \
        $(SLO)$/PaneChildWindows.obj			\
        $(SLO)$/PaneDockingWindow.obj			\
        $(SLO)$/PaneShells.obj

LIB1TARGET= $(SLB)$/$(TARGET).lib

LIB1OBJFILES= \
        $(SLO)$/dlgctrls.obj \
        $(SLO)$/diactrl.obj  \
        $(SLO)$/gluectrl.obj  \
        $(SLO)$/sdtreelb.obj \
        $(SLO)$/animobjs.obj \
        $(SLO)$/filedlg.obj   \
        $(SLO)$/unchss.obj  \
        $(SLO)$/assclass.obj \
        $(SLO)$/navigatr.obj \
        $(SLO)$/docprev.obj     \
        $(SLO)$/ins_paste.obj \
        $(SLO)$/AnimationChildWindow.obj	\
        $(SLO)$/LayerDialogChildWindow.obj	\
        $(SLO)$/LayerDialogContent.obj		\
        $(SLO)$/LayerTabBar.obj				\
        $(SLO)$/NavigatorChildWindow.obj	\
        $(SLO)$/SpellDialogChildWindow.obj \
        $(SLO)$/TemplateScanner.obj \
        $(SLO)$/sdabstdlg.obj \
        $(SLO)$/PaneChildWindows.obj		\
        $(SLO)$/PaneDockingWindow.obj		\
        $(SLO)$/PaneShells.obj


LIB2TARGET= $(SLB)$/sdui.lib

LIB2OBJFILES= \
        $(SLO)$/masterlayoutdlg.obj\
        $(SLO)$/headerfooterdlg.obj\
        $(SLO)$/sduiexp.obj \
        $(SLO)$/sddlgfact.obj \
        $(SLO)$/brkdlg.obj \
        $(SLO)$/copydlg.obj \
        $(SLO)$/custsdlg.obj \
        $(SLO)$/dlgchar.obj \
        $(SLO)$/dlgass.obj \
        $(SLO)$/dlgassim.obj \
        $(SLO)$/dlgfield.obj \
        $(SLO)$/dlgpage.obj \
        $(SLO)$/dlgsnap.obj \
        $(SLO)$/ins_paste.obj \
        $(SLO)$/inspagob.obj \
        $(SLO)$/morphdlg.obj \
        $(SLO)$/dlgolbul.obj \
        $(SLO)$/paragr.obj \
        $(SLO)$/present.obj \
        $(SLO)$/prltempl.obj \
        $(SLO)$/sdpreslt.obj \
        $(SLO)$/tabtempl.obj \
        $(SLO)$/tpaction.obj \
        $(SLO)$/tpoption.obj \
        $(SLO)$/vectdlg.obj \
        $(SLO)$/prntopts.obj \
        $(SLO)$/layeroptionsdlg.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

$(INCCOM)$/sduilib.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
    echo \#define DLL_NAME \"libsdui$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
    echo $(EMQ)#define DLL_NAME $(EMQ)"sdui$(DLLPOSTFIX)$(DLLPOST)$(EMQ)" >$@
.ENDIF

$(SLO)$/sdabstdlg.obj : $(INCCOM)$/sduilib.hxx

