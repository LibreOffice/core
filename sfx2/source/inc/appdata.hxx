/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _SFX_APPDATA_HXX
#define _SFX_APPDATA_HXX

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <svl/lstner.hxx>
#include <svtools/ehdl.hxx>
#include <vcl/timer.hxx>

#include <com/sun/star/frame/XModel.hpp>

#include "bitset.hxx"
#include <vector>

class SfxApplication;
class SfxProgress;
class SfxChildWinFactArr_Impl;
class SfxDdeDocTopics_Impl;
class DdeService;
class SfxItemPool;
class SfxFilterMatcher;
class ISfxTemplateCommon;
class SfxFilterMatcher;
class SfxStatusDispatcher;
class SfxDdeTriggerTopic_Impl;
class SfxDocumentTemplates;
class SfxFrame;
typedef ::std::vector<SfxFrame*> SfxFrameArr_Impl;
class SvtSaveOptions;
class SvtHelpOptions;
class ResMgr;
class SfxTbxCtrlFactArr_Impl;
class SfxStbCtrlFactArr_Impl;
class SfxMenuCtrlFactArr_Impl;
class SfxViewFrameArr_Impl;
class SfxViewShellArr_Impl;
class SfxObjectShellArr_Impl;
class ResMgr;
class SfxViewFrame;
class SfxSlotPool;
class SfxDispatcher;
class SfxInterface;
class BasicManager;
class SfxBasicManagerHolder;
class SfxBasicManagerCreationListener;

namespace sfx2 { namespace appl { class ImeStatusWindow; } }

typedef Link* LinkPtr;

class SfxAppData_Impl
{
public:
    IndexBitSet                         aIndexBitSet;           // for counting noname documents
    OUString                            aLastDir;               // for IO dialog

    // DDE stuff
    DdeService*                         pDdeService;
    SfxDdeDocTopics_Impl*               pDocTopics;
    SfxDdeTriggerTopic_Impl*            pTriggerTopic;
    DdeService*                         pDdeService2;

    // single instance classes
    SfxChildWinFactArr_Impl*            pFactArr;
    SfxFrameArr_Impl*                   pTopFrames;

    // application members
    SfxFilterMatcher*                   pMatcher;
#ifndef DISABLE_SCRIPTING
    ResMgr*                             pBasicResMgr;
#endif
    ResMgr*                             pSvtResMgr;
#ifdef DBG_UTIL
    SimpleErrorHandler *m_pSimpleErrorHdl;
#endif
    SfxErrorHandler *m_pToolsErrorHdl;
    SfxErrorHandler *m_pSoErrorHdl;
#ifndef DISABLE_SCRIPTING
    SfxErrorHandler *m_pSbxErrorHdl;
#endif
    SfxStatusDispatcher*                pAppDispatch;
    SfxDocumentTemplates*               pTemplates;

    // global pointers
    SfxItemPool*                        pPool;
    SvtSaveOptions*                     pSaveOptions;
    SvtHelpOptions*                     pHelpOptions;

    // "current" functionality
    SfxProgress*                        pProgress;
    ISfxTemplateCommon*                 pTemplateCommon;

    sal_uInt16                              nDocModalMode;              // counts documents in modal mode
    sal_uInt16                              nAutoTabPageId;
    sal_uInt16                              nRescheduleLocks;
    sal_uInt16                              nInReschedule;

    rtl::Reference< sfx2::appl::ImeStatusWindow > m_xImeStatusWindow;

    SfxTbxCtrlFactArr_Impl*     pTbxCtrlFac;
    SfxStbCtrlFactArr_Impl*     pStbCtrlFac;
    SfxMenuCtrlFactArr_Impl*    pMenuCtrlFac;
    SfxViewFrameArr_Impl*       pViewFrames;
    SfxViewShellArr_Impl*       pViewShells;
    SfxObjectShellArr_Impl*     pObjShells;
    ResMgr*                     pSfxResManager;
    ResMgr*                     pOfaResMgr;
    SfxBasicManagerHolder*      pBasicManager;
    SfxBasicManagerCreationListener*
                                pBasMgrListener;
    SfxViewFrame*               pViewFrame;
    SfxSlotPool*                pSlotPool;
    SfxDispatcher*              pAppDispat;     // Dispatcher if no document
    SfxInterface**              pInterfaces;

    sal_uInt16                      nDocNo;             // current Doc-Number (AutoName)
    sal_uInt16                      nInterfaces;

    sal_Bool                        bDispatcherLocked:1;    // do nothing
    sal_Bool                        bDowning:1;   // sal_True on Exit and afterwards
    sal_Bool                        bInQuit : 1;
    sal_Bool                        bInvalidateOnUnlock : 1;
    sal_Bool                        bODFVersionWarningLater : 1;

                                SfxAppData_Impl( SfxApplication* );
                                ~SfxAppData_Impl();

    SfxDocumentTemplates*       GetDocumentTemplates();
    void                        DeInitDDE();

    /** called when the Application's BasicManager has been created. This can happen
        explicitly in SfxApplication::GetBasicManager, or implicitly if a document's
        BasicManager is created before the application's BasicManager exists.
    */
    void                        OnApplicationBasicManagerCreated( BasicManager& _rManager );
};

#endif // #ifndef _SFX_APPDATA_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
