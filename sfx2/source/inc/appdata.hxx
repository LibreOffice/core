/*************************************************************************
 *
 *  $RCSfile: appdata.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SFX_APPDATA_HXX
#define _SFX_APPDATA_HXX

#include <tools/link.hxx>
#include <tools/list.hxx>
#include <svtools/lstner.hxx>
#include <bitset.hxx>
#include <objsh.hxx>

class SfxTaskManager;

class SfxApplication;
class SvBindingTransportFactory;
class SvLockBytesFactory;
class SfxIniDefaultManager;
class DataLockByteFactory_Impl;
class SvStrings;
class Config;
class PopupMenu;
class SfxBmkMenu;
class SvVerbList;
class SfxProgress;
class SfxConfigManager;
class SfxChildWinFactArr_Impl;
class SfxToolBoxConfig;
class SfxMacro;
class SfxDdeDocTopics_Impl;
class DdeService;
class SfxEventConfiguration;
class SfxMacroConfig;
class SvFactory;
class SotFactory;
class SfxItemPool;
class SfxInitLinkList;
class SfxFilterMatcher;
class DemoData_Impl;
class SfxExplorer;
class SfxMenuBarManager;
class StopButtonTimer_Impl;
class SvUShorts;
class SfxNodeData_Impl;
class SfxSIDList_Impl;
struct SfxFrameObjectFactoryPtr;
struct SfxPluginObjectFactoryPtr;
struct SfxPluginObjectFactoryPtr;
class ISfxTemplateCommon;
class SfxAnchorJobList_Impl;
class SfxFilterMatcher;
class INetConnection;
class SfxINetPlugInService;
class SfxTrash;
class SfxSimpleLockBytesFactoryMemberList;
class CntUpdateResults_Impl;
class SfxIniDefaultManager;
class SfxFolderCfgList_Impl;
class SfxCancelManager;
class ISfxModule;
struct SfxDownload_Impl;
class SvBindStatusCallback;
class SfxStatusDispatcher;
class SfxDesktop;
class SfxDdeTriggerTopic_Impl;
class OfaMiscCfg;
class SvLibrary;
class SfxDocumentTemplates;

DECLARE_LIST( ConnectionList_Impl, INetConnection * );

class StopButtonTimer_Impl : public Timer
{
    BOOL bState;
protected:
    virtual void Timeout();
public:
    StopButtonTimer_Impl();
    void SetButtonState( BOOL bStateP );
    BOOL GetButtonState() const { return bState; }
};


//=========================================================================
// SfxFolderState
//=========================================================================

/*
  [Beschreibung]
  Elemente dieser Aufzaehlung geben den Zustand eines Ordners an:
  SFX_FOLDERSTATE_EMPTY   der Ordner hat keine Unterordner
  SFX_FOLDERSTATE_CONTENT der Ordner hat Unterordner
  SFX_FOLDERSTATE_UNKNOWN es ist nicht bekannt, ob Unterordner vorhanden sind
  */

enum SfxFolderState
{
    SFX_FOLDERSTATE_EMPTY   = 0,
    SFX_FOLDERSTATE_CONTENT = 1,
    SFX_FOLDERSTATE_UNKNOWN = 2
};


//=========================================================================
// SfxAppData_Impl
//=========================================================================

class SfxFrameArr_Impl;

class SfxAppData_Impl : public SfxListener
{
public:
    SfxConfigManager*                   pAppCfg;
    SfxProgress*                        pProgress;
    SfxItemPool*                        pPool;
    SfxChildWinFactArr_Impl*            pFactArr;
    SfxMacro*                           pMacro;
    SvLibrary*                          pODKLib;
    String                              aLastDir;
    String                              aLastFilter;
    char                                nConfigManagerAvailable;
    IndexBitSet                         aIndexBitSet;
    DdeService*                         pDdeService; // wollen wir DDE machen?
    SfxDdeDocTopics_Impl*               pDocTopics;
    SfxEventConfiguration*              pEventConfig;
    SotFactory*                         pSfxApplicationObjectFactory;
    USHORT                              nAsynchronCalls;
    SvVerbList*                         pVerbs;
    USHORT                              nBasicCallLevel;
    USHORT                              nRescheduleLocks;
    USHORT                              nInReschedule;
    DemoData_Impl*                      pDemoData;
    SfxInitLinkList*                    pInitLinkList;
    Timer                               aLateInitTimer;
    SfxFilterMatcher*                   pMatcher;
    SfxExplorer*                        pExplorer;
    String                              aSaveAsTarget;
    String                              aUserEMailAddr;
    ConnectionList_Impl                 aConnectionList;
    SfxObjectFactory*                   pSfxPlugInObjectShellFactory;
    Window*                             pDefFocusWin;
    SfxMenuBarManager*                  pAppManager;
    SfxFrameObjectFactoryPtr*           pSfxFrameObjectFactoryPtr;
    List                                aPendingInitFactories;
    SfxIniDefaultManager*               pIniDefMgr;
    String                              aLogicAppName;
    SfxINetPlugInService*               pPlugService;
    StopButtonTimer_Impl*               pStopButtonTimer;
    SfxCancelManager*                   pCancelMgr;
    USHORT                              nDocModalMode;
    SvUShorts*                          pDisabledSlotList;
    Config*                             pFilterIni;
    ISfxModule*                         pISfxModule;
    String                              aLastNewURL;
    SfxPluginObjectFactoryPtr*          pSfxPluginObjectFactoryPtr;
    SfxNodeData_Impl*                   pNodeData_Impl;
    SfxSIDList_Impl*                    pSIDList_Impl;
    ISfxTemplateCommon*                 pTemplateCommon;
    Window*                             pActiveInplaceWindow;
    SfxAnchorJobList_Impl*              pAnchorJobList;
    ResMgr*                             pLabelResMgr;
    USHORT                              nDemoKind;
    SfxFrameArr_Impl*                   pTopFrames;
    SfxTrash*                           pTrash;
    SvStrings*                          pSecureURLs;
    USHORT                              nAutoTabPageId;
    DataLockByteFactory_Impl*           pDataLockBytesFactory;
    USHORT                              nExecutingSID;
    CntUpdateResults_Impl*              pNewMessages;
    USHORT                              nNewMessages;
    SvLockBytesFactory*                 pImageLockBytesFactory;
    SvLockBytesFactory*                 pInfoLockBytesFactory;
    SfxBmkMenu*                         pNewMenu;
    SfxBmkMenu*                         pBookmarkMenu;
    SfxBmkMenu*                         pAutoPilotMenu;
    SfxBmkMenu*                         pStartMenu;
    SfxStatusDispatcher*                pAppDispatch;
    SfxDdeTriggerTopic_Impl*            pTriggerTopic;
    DdeService*                         pDdeService2;
    SvStrings*                          pExtBrwOnExceptionList;
    SvStrings*                          pExtBrwOffExceptionList;
    OfaMiscCfg*                         pMiscConfig;
    SfxObjectShell*                     pThisDocument;
    BYTE                                bMultiQuickSearch;
    BYTE                                bShowFsysExtension;
    BYTE                                bUseExternBrowser;
    BOOL                                bAccelEnabled : 1;
    BOOL                                bIBMTitle : 1;
    BOOL                                bOLEResize : 1;
    BOOL                                bDirectAliveCount : 1;
    BOOL                                bInQuit : 1;
    BOOL                                bStbWasVisible : 1;
    BOOL                                bSessionFailed : 1;
    BOOL                                bOLEAutomation : 1;
    BOOL                                bInvalidateOnUnlock : 1;
    BOOL                                bBean : 1;
    BOOL                                bMinimized : 1;
    BOOL                                bInvisible : 1;
    BOOL                                bSmartBeamer : 1;
    BOOL                                bInException : 1;
    BOOL                                bNewMessagesBlinker : 1;
    BOOL                                bNewTaskForNewMessages : 1;
    BOOL                                bBeamerSwitchedOn : 1;
    BOOL                                bLateInit_BrowseRegistrationPage : 1;
    USHORT                              nAppEvent;
    SfxDocumentTemplates*               pTemplates;
    String                              aOpenList;
    String                              aPrintList;
    String                              aUcbUrl;

    SfxAppData_Impl( SfxApplication* pApp );
    ~SfxAppData_Impl();

    PopupMenu*                          GetPopupMenu( USHORT nSid, BOOL bBig=FALSE, BOOL bNew=FALSE );
    virtual void                        Notify( SfxBroadcaster &rBC, const SfxHint &rHint );
    DECL_STATIC_LINK(                   SfxAppData_Impl, CreateDataLockBytesFactory, void* );
    void                                ResetNewMessages( const String &rFolderULR );
    void                                LoadNewMessages();
    void                                SaveNewMessages();
    void                                UpdateApplicationSettings( BOOL bDontHide );
    SfxDocumentTemplates*               GetDocumentTemplates();
};

extern void FATToVFat_Impl( String& );

#include <svtools/poolitem.hxx>

class SfxPtrItem : public SfxPoolItem
{
    void* pPtr;
public:
                             TYPEINFO();
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const
    {   return new SfxPtrItem( *this ); }
    virtual int              operator==( const SfxPoolItem& rL) const
    {   return ((SfxPtrItem&)rL).pPtr == pPtr; }
    SfxPtrItem( USHORT nWhich, void * pValue ) : SfxPoolItem( nWhich )
    {   pPtr = pValue; }
    void* GetValue() const { return pPtr; }
};

#endif // #ifndef _SFX_APPDATA_HXX


