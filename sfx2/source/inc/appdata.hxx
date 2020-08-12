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
#ifndef INCLUDED_SFX2_SOURCE_INC_APPDATA_HXX
#define INCLUDED_SFX2_SOURCE_INC_APPDATA_HXX

#include <config_features.h>

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <svl/svdde.hxx>
#include <svtools/ehdl.hxx>
#include <sfx2/app.hxx>
#include <o3tl/enumarray.hxx>

#include <bitset.hxx>
#include <memory>
#include <vector>

class SfxApplication;
class SfxPickList;
class SfxProgress;
class SfxDdeDocTopic_Impl;
class DdeService;
class SfxItemPool;
class SfxFilterMatcher;
class ISfxTemplateCommon;
class SfxFilterMatcher;
class SfxStatusDispatcher;
class SfxDdeTriggerTopic_Impl;
class SfxDocumentTemplates;
class SfxFrame;
class SvtSaveOptions;
class SvtHelpOptions;
class SfxViewFrame;
class SfxSlotPool;
class SfxDispatcher;
class SfxInterface;
class BasicManager;
class SfxBasicManagerHolder;
class SfxBasicManagerCreationListener;
namespace sfx2::sidebar { class Theme; }



typedef std::vector<SfxDdeDocTopic_Impl*> SfxDdeDocTopics_Impl;

class SfxAppData_Impl
{
public:
    IndexBitSet                         aIndexBitSet;           // for counting noname documents
    OUString                            aLastDir;               // for IO dialog

    // DDE stuff
    std::unique_ptr<DdeService>              pDdeService;
    std::unique_ptr<SfxDdeDocTopics_Impl>    pDocTopics;
    std::unique_ptr<SfxDdeTriggerTopic_Impl> pTriggerTopic;
    std::unique_ptr<DdeService>              pDdeService2;

    // single instance classes
    std::unique_ptr<SfxChildWinFactArr_Impl>
                                        pFactArr;
    std::vector<SfxFrame*>              vTopFrames;

    // application members
    std::unique_ptr<SfxFilterMatcher>   pMatcher;
    std::unique_ptr<SfxErrorHandler>    m_pToolsErrorHdl;
    std::unique_ptr<SfxErrorHandler>    m_pSoErrorHdl;
#if HAVE_FEATURE_SCRIPTING
    std::unique_ptr<SfxErrorHandler>    m_pSbxErrorHdl;
#endif
    rtl::Reference<SfxStatusDispatcher> mxAppDispatch;
    std::unique_ptr<SfxPickList>        mxAppPickList;
    std::unique_ptr<SfxDocumentTemplates> pTemplates;

    // global pointers
    SfxItemPool*                        pPool;

    // "current" functionality
    SfxProgress*                        pProgress;

    sal_uInt16                              nDocModalMode;              // counts documents in modal mode
    sal_uInt16                              nRescheduleLocks;

    std::unique_ptr<SfxTbxCtrlFactArr_Impl>
                                pTbxCtrlFac;
    std::unique_ptr<SfxStbCtrlFactArr_Impl>
                                pStbCtrlFac;
    std::unique_ptr<SfxViewFrameArr_Impl>
                                pViewFrames;
    std::unique_ptr<SfxViewShellArr_Impl>
                                pViewShells;
    std::unique_ptr<SfxObjectShellArr_Impl>
                                pObjShells;
    std::unique_ptr<SfxBasicManagerHolder>
                                pBasicManager;
    std::unique_ptr<SfxBasicManagerCreationListener>
                                pBasMgrListener;
    SfxViewFrame*               pViewFrame;
    std::unique_ptr<SfxSlotPool>
                                pSlotPool;
    std::unique_ptr<SfxDispatcher>
                                pAppDispat;     // Dispatcher if no document
    ::rtl::Reference<sfx2::sidebar::Theme> m_pSidebarTheme;

    bool                        bDowning:1;   // sal_True on Exit and afterwards
    bool                        bInQuit : 1;

                                SfxAppData_Impl();
                                ~SfxAppData_Impl();

    SfxDocumentTemplates*       GetDocumentTemplates();
    void                        DeInitDDE();

    o3tl::enumarray<SfxToolsModule, std::unique_ptr<SfxModule>> aModules;

    /** called when the Application's BasicManager has been created. This can happen
        explicitly in SfxApplication::GetBasicManager, or implicitly if a document's
        BasicManager is created before the application's BasicManager exists.
    */
    void                        OnApplicationBasicManagerCreated( BasicManager& _rManager );
};

class SfxDdeTriggerTopic_Impl : public DdeTopic
{
#if defined(_WIN32)
public:
    SfxDdeTriggerTopic_Impl()
        : DdeTopic( "TRIGGER" )
        {}

    virtual bool Execute( const OUString* ) override { return true; }
#endif
};

#endif // INCLUDED_SFX2_SOURCE_INC_APPDATA_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
