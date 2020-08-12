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

#include <basic/sbstar.hxx>
#include <tools/solar.h>
#include <tools/debug.hxx>

#include <sfx2/app.hxx>
#include <appdata.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/fcontnr.hxx>
#include <nochaos.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <appbaslib.hxx>
#include <childwinimpl.hxx>
#include <ctrlfactoryimpl.hxx>
#include <shellimpl.hxx>
#include <basic/basicmanagerrepository.hxx>

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

    pImpl->pTemplates.reset();

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
    pImpl->pBasicManager->reset(nullptr);
        // this will also delete pBasMgr
#endif

    DBG_ASSERT( pImpl->pViewFrame == nullptr, "active foreign ViewFrame" );

    // free administration managers
    pImpl->pAppDispat.reset();

    // from here no SvObjects have to exists
    pImpl->pMatcher.reset();

    pImpl->pSlotPool.reset();
    pImpl->pFactArr.reset();

    pImpl->pTbxCtrlFac.reset();
    pImpl->pStbCtrlFac.reset();
    pImpl->pViewFrames.reset();
    pImpl->pViewShells.reset();
    pImpl->pObjShells.reset();

    //TODO/CLEANUP
    //ReleaseArgs could be used instead!
    pImpl->pPool = nullptr;
    NoChaos::ReleaseItemPool();

#if HAVE_FEATURE_SCRIPTING
    pImpl->m_pSbxErrorHdl.reset();
#endif
    pImpl->m_pSoErrorHdl.reset();
    pImpl->m_pToolsErrorHdl.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
