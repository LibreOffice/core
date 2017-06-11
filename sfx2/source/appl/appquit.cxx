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

#include "sfx2/strings.hrc"
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
#include <sfx2/templdlg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/docfile.hxx>
#include "sfxtypes.hxx"
#include "sfx2/strings.hrc"
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
    if ( pImpl->bDowning )
        return;

#if HAVE_FEATURE_SCRIPTING
    StarBASIC::Stop();

    SaveBasicAndDialogContainer();
#endif

    pImpl->bDowning = true; // due to Timer from DecAliveCount and QueryExit

    DELETEZ( pImpl->pTemplates );

    // By definition there shouldn't be any open view frames when we reach
    // this method. Therefore this call makes no sense and is the source of
    // some stack traces, which we don't understand.
    // For more information see:
    pImpl->bDowning = false;
    DBG_ASSERT( !SfxViewFrame::GetFirst(),
                "existing SfxViewFrame after Execute" );
    DBG_ASSERT( !SfxObjectShell::GetFirst(),
                "existing SfxObjectShell after Execute" );
    pImpl->pAppDispat->Pop( *this, SfxDispatcherPopFlags::POP_UNTIL );
    pImpl->pAppDispat->Flush();
    pImpl->bDowning = true;
    pImpl->pAppDispat->DoDeactivate_Impl( true, nullptr );

    // Release Controller and others
    // then the remaining components should also disappear ( Beamer! )

#if HAVE_FEATURE_SCRIPTING
    BasicManagerRepository::resetApplicationBasicManager();
    pImpl->pBasicManager->reset( nullptr );
        // this will also delete pBasMgr
#endif

    DBG_ASSERT( pImpl->pViewFrame == nullptr, "active foreign ViewFrame" );

    // free administration managers
    DELETEZ(pImpl->pAppDispat);
    SfxResLocale::DeleteResLocale();
    SvtResLocale::DeleteResLocale();

    // from here no SvObjects have to exists
    DELETEZ(pImpl->pMatcher);

    DELETEX(SfxSlotPool, pImpl->pSlotPool);
    DELETEX(SfxChildWinFactArr_Impl, pImpl->pFactArr);

    DELETEX(SfxTbxCtrlFactArr_Impl, pImpl->pTbxCtrlFac);
    DELETEX(SfxStbCtrlFactArr_Impl, pImpl->pStbCtrlFac);
    DELETEX(SfxViewFrameArr_Impl, pImpl->pViewFrames);
    DELETEX(SfxViewShellArr_Impl, pImpl->pViewShells);
    DELETEX(SfxObjectShellArr_Impl, pImpl->pObjShells);

    //TODO/CLEANUP
    //ReleaseArgs could be used instead!
    pImpl->pPool = nullptr;
    NoChaos::ReleaseItemPool();

#if HAVE_FEATURE_SCRIPTING
    delete pImpl->m_pSbxErrorHdl;
#endif
    delete pImpl->m_pSoErrorHdl;
    delete pImpl->m_pToolsErrorHdl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
