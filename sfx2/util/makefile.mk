#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: csaba $ $Date: 2000-10-16 15:05:18 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..

# ---- LDump2 laeuft auf tlx Rechner nicht
USE_LDUMP2=TRUE

ENABLE_EXCEPTIONS=TRUE
PRJNAME=sfx2
TARGET=sfx
.INCLUDE :  $(PRJ)$/util$/makefile.pmk
AUTOSEG=true
#sfx.hid generieren
.IF "$(header)" == ""
GEN_HID=TRUE
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Allgemein ----------------------------------------------------

.IF "$(COM)"=="ICC"
LINKFLAGS+=/SEGMENTS:512 /PACKD:32768
.ENDIF

.IF "$(header)" == ""
CXXFILES =         $(PROJECTPCHSOURCE).cxx
.ENDIF


.IF "$(header)" == ""

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=  $(SLB)$/appl.lib		\
            $(SLB)$/explorer.lib	\
            $(SLB)$/doc.lib			\
            $(SLB)$/view.lib		\
            $(SLB)$/control.lib		\
            $(SLB)$/notify.lib		\
            $(SLB)$/menu.lib		\
            $(SLB)$/inet.lib		\
            $(SLB)$/accel.lib		\
            $(SLB)$/toolbox.lib		\
            $(SLB)$/statbar.lib		\
            $(SLB)$/dialog.lib		\
            $(SLB)$/bastyp.lib		\
            $(SLB)$/config.lib

HELPIDFILES=\
            ..\inc\sfxsids.hrc	\
            ..\source\inc\helpid.hrc

.IF "$(GUI)"!="UNX"
LIB2TARGET= $(LB)$/$(TARGET).lib
LIB2FILES=  $(LB)$/appl.lib       \
            $(LB)$/isfx.lib
.ENDIF

SHL1TARGET= sfx$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB= isfx

#		$(ONELIB) \
#		$(UNOLIB) \


SHL1STDLIBS+=\
        $(AUTOMATIONLIB) \
        $(BASICLIB) \
        $(VOSLIB) \
        $(SALLIB) \
        $(CHANELLIB) \
        $(SO2LIB) \
        $(INETLIBSH) \
        $(SVTOOLLIB) \
        $(SVLLIB)	\
        $(SVLIB) \
        $(TOOLSLIB) \
        $(HELPLIB) \
        $(SVMEMLIB) \
        $(GOODIESLIB) \
        $(TKLIB) \
        $(SOTLIB) \
        $(SETUPLIB) \
        $(RTLLIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB) \
        $(UNOTOOLSLIB) \
        $(UCBHELPERLIB)

.IF "$(TF_UCB)" == ""
SHL1STDLIBS+=\
        $(CHAOSLIB)
.ELSE
SHL1STDLIBS+=\
        $(CPPULIB)	\
        $(CPPUHELPERLIB)
.ENDIF

.IF "$(SOLAR_JAVA)" != ""
SHL1STDLIBS+=\
        $(SJLIB)
.ENDIF

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=\
        advapi32.lib
.ENDIF


.IF "$(GUI)"!="MAC"
SHL1DEPN += $(shell $(FIND) $(SLO) -type f -name "*.OBJ" -print)
.ENDIF

.IF "$(SOLAR_JAVA)" != ""
SHL1DEPN+= \
            $(L)$/sj.lib
.ENDIF

SHL1LIBS=   $(LIB1TARGET)

SHL1OBJS=   $(SLO)$/sfxdll.obj

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(MISC)$/$(SHL1TARGET).flt	\
                $(HXX1FILES)	\
                $(HXX2FILES)	\
                $(HXX3FILES)	\
                $(HXX4FILES)	\
                $(HXX5FILES)	\
                $(HXX6FILES)	\
                $(HXX7FILES)	\
                $(HXX8FILES)	\
                $(HXX9FILES)	\
                $(HXX10FILES)
DEFLIB1NAME	=sfx
DEF1DES		=Sfx

.IF "$(GUI)"=="WNT"
DEF1EXPORT6 = component_writeInfo
DEF1EXPORT7 = component_getFactory
.ELSE
.IF "$(GUI)"=="OS2"
.IF "$(COM)"=="ICC"
DEF1EXPORT6 = component_writeInfo
DEF1EXPORT7 = component_getFactory
.ELSE
.IF "$(COM)"=="WTC"
DEF1EXPORT6 = component_writeInfo
DEF1EXPORT7 = component_getFactory
.ELSE
DEF1EXPORT6 = _component_writeInfo
DEF1EXPORT7 = _component_getFactory
.ENDIF
.ENDIF
.ENDIF
.ENDIF

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""

ALL:\
    $(LIB1TARGET) \
    $(MISC)$/linkinc.ls		\
    $(SRS)$/hidother.hid \
    ALLTAR

# --- SFX-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt:
    @echo ------------------------------
    @echo Making: $@
    @echo _Impl > $@
    @echo IMP >> $@
    @echo Dbg >> $@
    @echo DLL >> $@
    @echo AVL_Tree >> $@
    @echo BinaryGlobalLoader >> $@
    @echo BinaryResLoader >> $@
    @echo BitSet >> $@
    @echo CntBoxOpenJob >> $@
    @echo CntContentPropertiesTabPage >> $@
    @echo CntCoreDataComplex_Impl >> $@
    @echo CntCoreDataSimple_Impl >> $@
    @echo CntDirectory >> $@
    @echo CntDirEntry >> $@
    @echo CntDirtyItemSet >> $@
    @echo CntExplorerFileContent >> $@
    @echo CntFTPServerTabPage >> $@
    @echo CntGeneralTabPage >> $@
    @echo CntHash >> $@
    @echo CntIMAPFolderInfoItem >> $@
    @echo CntIMAPFolderNode >> $@
    @echo CntIMAPRootNode >> $@
    @echo CntInterface >> $@
    @echo CntItemPool >> $@
    @echo CntLazyDelete >> $@
    @echo CntLazyWriter >> $@
    @echo CntMessageBodyItemImp >> $@
    @echo CntMessageNode >> $@
    @echo CntNewMessagesControl >> $@
    @echo CntNewsBoxGetListJob >> $@
    @echo CntNewsBoxNode >> $@
    @echo CntNewsBoxNodeImp >> $@
    @echo CntNewsBoxOpenJob >> $@
    @echo CntNewsBoxPutListJob >> $@
    @echo CntNewsGroupNode >> $@
    @echo CntNewsGroupOpenJob >> $@
    @echo CntNewsGroupMarkJob >> $@
    @echo CntNewsMsgOpenJob >> $@
    @echo CntCrossReferenceItem >> $@
    @echo CntNode >> $@
    @echo CntNodeFactory >> $@
    @echo CntNodeHint >> $@
    @echo CntNodeJob >> $@
    @echo CntOutMsgExternalStateItem >> $@
    @echo CntOutMsgInternalStateItem >> $@
    @echo CntOutTrayNode >> $@
    @echo CntPopupMenu >> $@
    @echo CntProgress >> $@
    @echo CntPOP3Node >> $@
    @echo CntPOP3Box >> $@
    @echo CntPOP3Msg >> $@
    @echo CntPropertyDialog >> $@
    @echo CntPropertyTabPage >> $@
    @echo CntRangesItem >> $@
    @echo CntRecipientInfo >> $@
    @echo CntRecipientListItem >> $@
    @echo CntRecipientStateTabPage >> $@
    @echo CntRootNodeMgr >> $@
    @echo CntRootStorageNode >> $@
    @echo CntRulesTabPage >> $@
    @echo CntSeenStatusItem >> $@
    @echo CntServerSettingsTabPage >> $@
    @echo CntSortingInfoArr >> $@
    @echo CntSortingItem >> $@
    @echo CntSubscribeTabPage >> $@
    @echo CntThreadData >> $@
    @echo CntThreadingInfoArr >> $@
    @echo CntThreadingItem >> $@
    @echo CntUI >> $@
    @echo CntUpdateHint >> $@
    @echo CntUpdateInitTimer >> $@
    @echo CntUShortListItem >> $@
    @echo CntViewDescriptionItem >> $@
    @echo DocManager >> $@
    @echo FileHeader >> $@
    @echo IdPool >> $@
    @echo ImportParser_Impl >> $@
    @echo ItemListData >> $@
    @echo MapEntry >> $@
    @echo NotesLibrary >> $@
    @echo QueryFolderNameDialog >> $@
    @echo SbxArrayRef >> $@
    @echo SbxObjectRef >> $@
    @echo SfxAccelerator >> $@
    @echo SfxConfigDialog >> $@
    @echo SfxConfigManager >> $@
    @echo SfxContextMenuData >> $@
    @echo SfxCustomizeToolBox >> $@
    @echo SfxDirEntryHint >> $@
    @echo SfxDocumentsCollection >> $@
    @echo SfxDocumentTemplateDlg >> $@
    @echo SfxEnumMenu >> $@
    @echo SfxEventConfigPage >> $@
    @echo SfxExecuteItem >> $@
    @echo SfxExplorerBookmark >> $@
    @echo SfxExplorerBrowserConfig >> $@
    @echo SfxExplorerContentHint >> $@
    @echo SfxExplorerFolderSearchCriteriaTabPage >> $@
    @echo SfxExplorerFolderSearchDlg >> $@
    @echo SfxExplorerFolderSearchPlaceTabPage >> $@
    @echo SfxExplorerFolderSearchWrapper >> $@
    @echo SfxExplorerIterator >> $@
    @echo SfxExplorerMenuView >> $@
    @echo SfxExplorerObjectShell >> $@
    @echo SfxExplorerSearchFolder >> $@
    @echo SfxExtDirEntryHint >> $@
    @echo SfxFilterDialog >> $@
    @echo SfxFilterFunc >> $@
    @echo SfxFrameProperties >> $@
    @echo SfxGenericObjectShell >> $@
    @echo SfxGenericViewShell >> $@
    @echo SfxHelpTextCache >> $@
    @echo SfxHelpViewShell >> $@
    @echo SfxIPCServer >> $@
    @echo SfxIPCService >> $@
    @echo SfxJobQueue >> $@
    @echo SfxMailBroadcaster >> $@
    @echo SfxMenuBarManager >> $@
    @echo SfxMenuManager >> $@
    @echo SfxMenuConfig >> $@
    @echo SfxMessageServer >> $@
    @echo SfxNewHdl >> $@
    @echo SfxObjectBarConfigPage >> $@
    @echo SfxObjectBarListBox >> $@
    @echo SfxObjectList >> $@
    @echo SfxObjectVerbsControl >> $@
    @echo SfxOfflineURLManager >> $@
    @echo SfxPlugInFrame >> $@
    @echo SfxPlugInObjectShell >> $@
    @echo SfxPlugInViewShell >> $@
    @echo SfxPopupMenuManager >> $@
    @echo SfxResId >> $@
    @echo SfxSlotInfo >> $@
    @echo SfxSplitWindow >> $@
    @echo SfxSrchDlgBrowseBox >> $@
    @echo SfxStateCache >> $@
    @echo SfxStatusBarConfigListBox >> $@
    @echo SfxStatusBarConfigPage >> $@
    @echo SfxStringHint >> $@
    @echo SfxTaskBar >> $@
    @echo SfxTaskBarWrapper >> $@
    @echo SfxTaskButtonBar >> $@
    @echo SfxTaskToolBox >> $@
    @echo SfxTemplateControllerItem >> $@
    @echo SfxTemplateDir >> $@
    @echo SfxTemplateDirEntry >> $@
    @echo SfxTemplateOrganizeDlg >> $@
    @echo SfxToolbox >> $@
    @echo SfxToolBoxConfig >> $@
    @echo SfxToolboxCustomizer >> $@
    @echo SfxToolboxCustomWindow >> $@
    @echo SfxToolBoxOptions >> $@
    @echo SfxURLFrame >> $@
    @echo SfxVirtualMenu >> $@
    @echo SfxWindowsCollection >> $@
    @echo SfxWorkWindow >> $@
    @echo SortedULONGs_SAR >> $@
    @echo solver >> $@
    @echo _FileListEntry >> $@
    @echo InitHelp >> $@
    @echo InitTpl >> $@
.IF "$(OLD_CHAOS)" != ""
.IF "$(VCL)" != ""
    @echo CntExtendedHash >> $@
    @echo CntStore >> $@
    @echo CntFldr >> $@
.ENDIF
.ENDIF
.IF "$(GUI)"=="WNT" || "$(GUI)"=="WIN"
    @echo WEP>>$@
.ENDIF
    @echo CntSearchLocationsTabPage>>$@
    @echo CntSearchDialog>>$@
.IF "$(COM)"=="MSC"
    @echo ??_5>>$@
    @echo ??_7>>$@
    @echo ??_8>>$@
    @echo ??_9>>$@
    @echo ??_C>>$@
    @echo ??_E>>$@
    @echo ??_F>>$@
    @echo ??_G>>$@
    @echo ??_H>>$@
    @echo ??_I>>$@
    @echo ?CreateImpl@>>$@
    @echo ?GetGlobalClassName@>>$@
    @echo ?GetSvFactory@>>$@
    @echo ?LinkStub>>$@
    @echo ?nClassId@>>$@
    @echo ?RegisterInterface@>>$@
    @echo ::UsrException>>$@
    @echo ::IllegalArg>>$@
    @echo AVIllegalArg>>$@
    @echo ::Exception>>$@
    @echo NoSuchElementException>>$@
    @echo __CT>>$@
.ENDIF
.IF "$(GUI)"=="WNT"
    @echo ?CreateType@>>$@
.ENDIF
.IF "$(COM)"=="ICC"
    @echo __dt__14SfxApplicationFv>>$@
    @echo __dt__14MDIApplicationFv>>$@
    @echo __dt__11ApplicationFv>>$@
    @echo __dt__17PlugInApplicationFv>>$@
    @echo __ct__4ORefXT7OSpeech_FP7OSpeech>>$@
    @echo __ct__4ORefXT7OSpeech_FRC4ORefXT7OSpeech_>>$@
    @echo m_pLoader__>>$@
.ENDIF

SFXSRSLIST=\
        $(SRS)$/accel.srs \
        $(SRS)$/appl.srs \
        $(SRS)$/doc.srs \
        $(SRS)$/view.srs \
        $(SRS)$/config.srs \
        $(SRS)$/menu.srs \
        $(SRS)$/statbar.srs \
        $(SRS)$/toolbox.srs \
        $(SRS)$/dialog.srs \
        $(SRS)$/bastyp.srs \
        $(SRS)$/explorer.srs

.IF "$(GUI)"=="WIN"
RESLIBSPLIT1NAME=$(TARGET)
RESLIBSPLIT1SRSFILES=$(SFXSRSLIST)
.ELSE
RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES=$(SFXSRSLIST)
.ENDIF

$(SRS)$/hidother.hid: hidother.src
.IF "$(GUI)$(CPU)"=="WNTI"
.IF "$(BUILD_SOSL)"==""
    @+echo no hids
    @+-mhids hidother.src ..\$(INPATH)$/srs sfx2 hidother
.ENDIF
.ELSE
    @echo nix
.ENDIF

.ENDIF
.ENDIF

.INCLUDE :  target.mk

