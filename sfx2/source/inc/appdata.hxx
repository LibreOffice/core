/*************************************************************************
 *
 *  $RCSfile: appdata.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2001-03-02 15:54:11 $
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
#include <vcl/timer.hxx>
#include <tools/string.hxx>

#include "bitset.hxx"

class SfxApplication;
class SvStrings;
class Config;
class PopupMenu;
class SfxBmkMenu;
class SvVerbList;
class SfxProgress;
class SfxConfigManager;
class SfxChildWinFactArr_Impl;
class SfxToolBoxConfig;
class SfxDdeDocTopics_Impl;
class DdeService;
class SfxEventConfiguration;
class SfxMacroConfig;
class SfxItemPool;
class SfxInitLinkList;
class SfxFilterMatcher;
class SvUShorts;
struct SfxFrameObjectFactoryPtr;
struct SfxPluginObjectFactoryPtr;
class ISfxTemplateCommon;
class SfxFilterMatcher;
class SfxCancelManager;
class SfxStatusDispatcher;
class SfxDdeTriggerTopic_Impl;
class SfxMiscCfg;
class SfxDocumentTemplates;
class SfxFrameArr_Impl;
class SvtSaveOptions;
class SvtUndoOptions;
class SvtHelpOptions;
class SfxObjectFactory;
class SfxObjectShell;
class ResMgr;
class Window;

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
// SfxAppData_Impl
//=========================================================================

class SfxAppData_Impl : public SfxListener
{
public:
    IndexBitSet                         aIndexBitSet;           // for counting noname documents
    List                                aPendingInitFactories;  // late filter init
    Timer                               aLateInitTimer;
    String                              aLastDir;               // for IO dialog
    String                              aLastFilter;            // for IO dialog
    String                              aLastNewURL;            // for AppControl
    String                              aOpenList;              // command line arguments
    String                              aPrintList;             // command line arguments
    String                              aPortalConnect;         // command line arguments
    bool                                bServer;                // command line arguments

    // DDE stuff
    DdeService*                         pDdeService;
    SfxDdeDocTopics_Impl*               pDocTopics;
    SfxDdeTriggerTopic_Impl*            pTriggerTopic;
    DdeService*                         pDdeService2;

    // single instance classes
    SfxChildWinFactArr_Impl*            pFactArr;
    SfxPluginObjectFactoryPtr*          pSfxPluginObjectFactoryPtr;
    SfxObjectFactory*                   pSfxPlugInObjectShellFactory;
    SfxFrameObjectFactoryPtr*           pSfxFrameObjectFactoryPtr;
    SfxFrameArr_Impl*                   pTopFrames;

    // special members
    SfxInitLinkList*                    pInitLinkList;
    StopButtonTimer_Impl*               pStopButtonTimer;

    // application members
    SfxFilterMatcher*                   pMatcher;
    SfxCancelManager*                   pCancelMgr;
    ResMgr*                             pLabelResMgr;
    SfxStatusDispatcher*                pAppDispatch;
    SfxConfigManager*                   pAppCfg;
    SfxDocumentTemplates*               pTemplates;

    // global pointers
    SvVerbList*                         pVerbs;
    Config*                             pFilterIni;
    SfxItemPool*                        pPool;
    SfxEventConfiguration*              pEventConfig;
    SvUShorts*                          pDisabledSlotList;
    SvStrings*                          pSecureURLs;
    SfxBmkMenu*                         pNewMenu;
    SfxBmkMenu*                         pAutoPilotMenu;
    SfxMiscCfg*                         pMiscConfig;
    SvtSaveOptions*                     pSaveOptions;
    SvtUndoOptions*                     pUndoOptions;
    SvtHelpOptions*                     pHelpOptions;

    // "current" functionality
    SfxObjectShell*                     pThisDocument;
    SfxProgress*                        pProgress;
    Window*                             pDefFocusWin;
    ISfxTemplateCommon*                 pTemplateCommon;

    USHORT                              nDocModalMode;              // counts documents in modal mode
    USHORT                              nAutoTabPageId;
    USHORT                              nExecutingSID;
    USHORT                              nBasicCallLevel;
    USHORT                              nRescheduleLocks;
    USHORT                              nInReschedule;
    USHORT                              nAsynchronCalls;
    USHORT                              nAppEvent;                  // command line interpretation

    BOOL                                bDirectAliveCount : 1;
    BOOL                                bInQuit : 1;
    BOOL                                bInvalidateOnUnlock : 1;
    BOOL                                bBean : 1;
    BOOL                                bMinimized : 1;
    BOOL                                bInvisible : 1;
    BOOL                                bInException : 1;
    BOOL                                bPlugged : 1;
    BOOL                                bOLEResize : 1;

                                        SfxAppData_Impl( SfxApplication* );
                                        ~SfxAppData_Impl();

    PopupMenu*                          GetPopupMenu( USHORT nSid, BOOL bBig=FALSE, BOOL bNew=FALSE );
    virtual void                        Notify( SfxBroadcaster &rBC, const SfxHint &rHint );
    void                                UpdateApplicationSettings( BOOL bDontHide );
    SfxDocumentTemplates*               GetDocumentTemplates();
    DECL_STATIC_LINK(                   SfxAppData_Impl, CreateDocumentTemplates, void* );
};

#endif // #ifndef _SFX_APPDATA_HXX


