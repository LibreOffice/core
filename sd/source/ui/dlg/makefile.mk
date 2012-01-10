#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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

