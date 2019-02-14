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

#include <SpellDialogChildWindow.hxx>
#include <svx/svxids.hrc>

#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <DrawViewShell.hxx>
#include <OutlineViewShell.hxx>
#include <Outliner.hxx>
#include <drawdoc.hxx>

namespace sd {

SFX_IMPL_CHILDWINDOW_WITHID(SpellDialogChildWindow, SID_SPELL_DIALOG)

SpellDialogChildWindow::SpellDialogChildWindow (
    vcl::Window* _pParent,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SAL_UNUSED_PARAMETER SfxChildWinInfo* /*pInfo*/)
    : svx::SpellDialogChildWindow (_pParent, nId, pBindings),
      mpSdOutliner (nullptr),
      mbOwnOutliner (false)
{
    ProvideOutliner();
}

SpellDialogChildWindow::~SpellDialogChildWindow()
{
    EndSpellingAndClearOutliner();
}

SfxChildWinInfo SpellDialogChildWindow::GetInfo() const
{
    return svx::SpellDialogChildWindow::GetInfo();
}

void SpellDialogChildWindow::InvalidateSpellDialog()
{
    svx::SpellDialogChildWindow::InvalidateSpellDialog();
}

svx::SpellPortions SpellDialogChildWindow::GetNextWrongSentence( bool /*bRecheck*/ )
{
    svx::SpellPortions aResult;

    if (mpSdOutliner != nullptr)
    {
        ProvideOutliner();
        aResult = mpSdOutliner->GetNextSpellSentence();
    }
    return aResult;
}

void SpellDialogChildWindow::ApplyChangedSentence (
    const svx::SpellPortions& rChanged, bool bRecheck )
{
    if (mpSdOutliner != nullptr)
    {
        OutlinerView* pOutlinerView = mpSdOutliner->GetView(0);
        if (pOutlinerView != nullptr)
            mpSdOutliner->ApplyChangedSentence (
                pOutlinerView->GetEditView(),
                rChanged, bRecheck);
    }
}

void SpellDialogChildWindow::GetFocus()
{
    // In order to detect a cursor movement we could compare the
    // currently selected text shape with the one that was selected
    // when LoseFocus() was called the last time.
    // For the time being we instead rely on the DetectChange() method
    // in the SdOutliner class.
}

void SpellDialogChildWindow::LoseFocus()
{
}

void SpellDialogChildWindow::EndSpellingAndClearOutliner()
{
    if (!mpSdOutliner)
        return;
    EndListening(*mpSdOutliner->GetDoc());
    mpSdOutliner->EndSpelling();
    if (mbOwnOutliner)
        delete mpSdOutliner;
    mpSdOutliner = nullptr;
    mbOwnOutliner = false;
}

void SpellDialogChildWindow::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint))
    {
        if (SdrHintKind::ModelCleared == pSdrHint->GetKind())
        {
            EndSpellingAndClearOutliner();
        }
    }
}

void SpellDialogChildWindow::ProvideOutliner()
{
    ViewShellBase* pViewShellBase = dynamic_cast<ViewShellBase*>( SfxViewShell::Current() );

    if (pViewShellBase == nullptr)
        return;

    ViewShell* pViewShell = pViewShellBase->GetMainViewShell().get();
    // If there already exists an outliner that has been created
    // for another view shell then destroy it first.
    if (mpSdOutliner != nullptr)
        if(( dynamic_cast< const DrawViewShell *>( pViewShell ) !=  nullptr && ! mbOwnOutliner)
            || (dynamic_cast< const OutlineViewShell *>( pViewShell ) !=  nullptr && mbOwnOutliner))
        {
            EndSpellingAndClearOutliner();
        }

    // Now create/get an outliner if none is present.
    if (mpSdOutliner != nullptr)
        return;

    if( dynamic_cast< const DrawViewShell *>( pViewShell ) !=  nullptr)
    {
        // We need an outliner for the spell check so we have
        // to create one.
        mbOwnOutliner = true;
        SdDrawDocument *pDoc = pViewShell->GetDoc();
        mpSdOutliner = new SdOutliner(pDoc, OutlinerMode::TextObject);
        StartListening(*pDoc);
    }
    else if( dynamic_cast< const OutlineViewShell *>( pViewShell ) !=  nullptr)
    {
        // An outline view is already visible. The SdOutliner
        // will use it instead of creating its own.
        mbOwnOutliner = false;
        SdDrawDocument *pDoc = pViewShell->GetDoc();
        mpSdOutliner = pDoc->GetOutliner();
        StartListening(*pDoc);
    }

    // Initialize spelling.
    if (mpSdOutliner != nullptr)
    {
        mpSdOutliner->PrepareSpelling();
        mpSdOutliner->StartSpelling();
    }
}

} // end of namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
