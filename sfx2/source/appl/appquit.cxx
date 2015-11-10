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

#include <config_features.h>

#include <basic/basmgr.hxx>
#include <basic/sbstar.hxx>

#include <svl/svdde.hxx>
#include <vcl/layout.hxx>
#include <svl/eitem.hxx>

#include <unotools/saveopt.hxx>
#include <unotools/misccfg.hxx>

#include "app.hrc"
#include <sfx2/app.hxx>
#include <sfx2/evntconf.hxx>
#include <sfx2/unoctitm.hxx>
#include "appdata.hxx"
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/printer.hxx>
#include "arrdecl.hxx"
#include <sfx2/sfxresid.hxx>
#include <sfx2/event.hxx>
#include <sfx2/mnumgr.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/docfile.hxx>
#include "sfxtypes.hxx"
#include "sfxlocal.hrc"
#include <sfx2/fcontnr.hxx>
#include "nochaos.hxx"
#include <sfx2/doctempl.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include "appbaslib.hxx"
#include "childwinimpl.hxx"
#include <ctrlfactoryimpl.hxx>
#include <shellimpl.hxx>
#include <basic/basicmanagerrepository.hxx>
#include <svtools/svtresid.hxx>

using ::basic::BasicManagerRepository;

void SfxApplication::Deinitialize()
{
    if ( pAppData_Impl->bDowning )
        return;

#if HAVE_FEATURE_SCRIPTING
    StarBASIC::Stop();

    SaveBasicAndDialogContainer();
#endif

    pAppData_Impl->bDowning = true; // due to Timer from DecAliveCount and QueryExit

    DELETEZ( pAppData_Impl->pTemplates );

    // By definition there shouldn't be any open view frames when we reach
    // this method. Therefore this call makes no sense and is the source of
    // some stack traces, which we don't understand.
    // For more information see:
    pAppData_Impl->bDowning = false;
    DBG_ASSERT( !SfxViewFrame::GetFirst(),
                "existing SfxViewFrame after Execute" );
    DBG_ASSERT( !SfxObjectShell::GetFirst(),
                "existing SfxObjectShell after Execute" );
    pAppData_Impl->pAppDispat->Pop( *this, SfxDispatcherPopFlags::POP_UNTIL );
    pAppData_Impl->pAppDispat->Flush();
    pAppData_Impl->bDowning = true;
    pAppData_Impl->pAppDispat->DoDeactivate_Impl( true, nullptr );

    // Release Controller and others
    // then the remaining components should also disappear ( Beamer! )

#if HAVE_FEATURE_SCRIPTING
    BasicManagerRepository::resetApplicationBasicManager();
    pAppData_Impl->pBasicManager->reset( nullptr );
        // this will also delete pBasMgr
#endif

    DBG_ASSERT( pAppData_Impl->pViewFrame == nullptr, "active foreign ViewFrame" );

    delete[] pAppData_Impl->pInterfaces, pAppData_Impl->pInterfaces = nullptr;

    // free administration managers
    DELETEZ(pAppData_Impl->pAppDispat);
    SfxResId::DeleteResMgr();
    SvtResId::DeleteResMgr();

    // from here no SvObjects have to exists
    DELETEZ(pAppData_Impl->pMatcher);

    DELETEX(SfxSlotPool, pAppData_Impl->pSlotPool);
    DELETEX(SfxChildWinFactArr_Impl, pAppData_Impl->pFactArr);

    DELETEX(SfxTbxCtrlFactArr_Impl, pAppData_Impl->pTbxCtrlFac);
    DELETEX(SfxStbCtrlFactArr_Impl, pAppData_Impl->pStbCtrlFac);
    DELETEX(SfxMenuCtrlFactArr_Impl, pAppData_Impl->pMenuCtrlFac);
    DELETEX(SfxViewFrameArr_Impl, pAppData_Impl->pViewFrames);
    DELETEX(SfxViewShellArr_Impl, pAppData_Impl->pViewShells);
    DELETEX(SfxObjectShellArr_Impl, pAppData_Impl->pObjShells);

    //TODO/CLEANUP
    //ReleaseArgs could be used instead!
    pAppData_Impl->pPool = nullptr;
    NoChaos::ReleaseItemPool();

#if HAVE_FEATURE_SCRIPTING
    DELETEZ(pAppData_Impl->pBasicResMgr);
#endif
    DELETEZ(pAppData_Impl->pSvtResMgr);

#if HAVE_FEATURE_SCRIPTING
    delete pAppData_Impl->m_pSbxErrorHdl;
#endif
    delete pAppData_Impl->m_pSoErrorHdl;
    delete pAppData_Impl->m_pToolsErrorHdl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
