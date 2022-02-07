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

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>
#include <osl/diagnose.h>

#include <com/sun/star/frame/XModel.hpp>

#define SC_PROGRESS_CXX
#include <progress.hxx>
#include <document.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

using namespace com::sun::star;

static ScProgress theDummyInterpretProgress;
SfxProgress*    ScProgress::pGlobalProgress = nullptr;
sal_uInt64      ScProgress::nGlobalRange = 0;
sal_uInt64      ScProgress::nGlobalPercent = 0;
ScProgress*     ScProgress::pInterpretProgress = &theDummyInterpretProgress;
sal_uInt64      ScProgress::nInterpretProgress = 0;
ScDocument*     ScProgress::pInterpretDoc;
bool            ScProgress::bIdleWasEnabled = false;

static bool lcl_IsHiddenDocument( const SfxObjectShell* pObjSh )
{
    if (pObjSh)
    {
        SfxMedium* pMed = pObjSh->GetMedium();
        if (pMed)
        {
            SfxItemSet* pSet = pMed->GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SfxItemState::SET == pSet->GetItemState( SID_HIDDEN, true, &pItem ) &&
                        static_cast<const SfxBoolItem*>(pItem)->GetValue() )
                return true;
        }
    }
    return false;
}

static bool lcl_HasControllersLocked( const SfxObjectShell& rObjSh )
{
    uno::Reference<frame::XModel> xModel( rObjSh.GetBaseModel() );
    if (xModel.is())
        return xModel->hasControllersLocked();
    return false;
}

ScProgress::ScProgress(SfxObjectShell* pObjSh, const OUString& rText,
                       sal_uInt64 nRange, bool bWait)
    : bEnabled(true)
{

    if ( pGlobalProgress || SfxProgress::GetActiveProgress() )
    {
        if ( lcl_IsHiddenDocument(pObjSh) )
        {
            // loading a hidden document while a progress is active is possible - no error
            pProgress = nullptr;
        }
        else
        {
            OSL_FAIL( "ScProgress: there can be only one!" );
            pProgress = nullptr;
        }
    }
    else if ( SfxGetpApp()->IsDowning() )
    {
        //  This happens. E.g. when saving the clipboard-content as OLE when closing the app.
        //  In this case a SfxProgress would produce dirt in memory.
        //TODO: Should that be this way ???

        pProgress = nullptr;
    }
    else if ( pObjSh && ( pObjSh->GetCreateMode() == SfxObjectCreateMode::EMBEDDED ||
                          pObjSh->GetProgress() ||
                          lcl_HasControllersLocked(*pObjSh) ) )
    {
        //  no own progress for embedded objects,
        //  no second progress if the document already has one

        pProgress = nullptr;
    }
    else
    {
        pProgress.reset(new SfxProgress( pObjSh, rText, nRange, bWait ));
        pGlobalProgress = pProgress.get();
        nGlobalRange = nRange;
        nGlobalPercent = 0;
    }
}

ScProgress::ScProgress()
    : bEnabled(true)
{
    // DummyInterpret
}

ScProgress::~ScProgress()
{
    if ( pProgress )
    {
        pProgress.reset();
        pGlobalProgress = nullptr;
        nGlobalRange = 0;
        nGlobalPercent = 0;
    }
}

void ScProgress::CreateInterpretProgress( ScDocument* pDoc, bool bWait )
{
    if ( nInterpretProgress )
        nInterpretProgress++;
    else if ( pDoc->GetAutoCalc() )
    {
        nInterpretProgress = 1;
        bIdleWasEnabled = pDoc->IsIdleEnabled();
        pDoc->EnableIdle(false);
        // Interpreter may be called in many circumstances, also if another
        // progress bar is active, for example while adapting row heights.
        // Keep the dummy interpret progress.
        if ( !pGlobalProgress )
            pInterpretProgress = new ScProgress( pDoc->GetDocumentShell(),
                ScResId( STR_PROGRESS_CALCULATING ),
                pDoc->GetFormulaCodeInTree()/MIN_NO_CODES_PER_PROGRESS_UPDATE, bWait );
        pInterpretDoc = pDoc;
    }
}

void ScProgress::DeleteInterpretProgress()
{
    if ( !nInterpretProgress )
        return;

    /*  Do not decrement 'nInterpretProgress', before 'pInterpretProgress'
        is deleted. In rare cases, deletion of 'pInterpretProgress' causes
        a refresh of the sheet window which may call CreateInterpretProgress
        and DeleteInterpretProgress again (from Output::DrawStrings),
        resulting in double deletion of 'pInterpretProgress'. */
    if ( nInterpretProgress == 1 )
    {
        if ( pInterpretProgress != &theDummyInterpretProgress )
        {
            // move pointer to local temporary to avoid double deletion
            ScProgress* pTmpProgress = pInterpretProgress;
            pInterpretProgress = &theDummyInterpretProgress;
            delete pTmpProgress;
        }
        if ( pInterpretDoc )
            pInterpretDoc->EnableIdle(bIdleWasEnabled);
    }
    --nInterpretProgress;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
