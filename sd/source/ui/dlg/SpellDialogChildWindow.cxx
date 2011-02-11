/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "SpellDialogChildWindow.hxx"
#include <svx/svxids.hrc>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

namespace sd{

SFX_IMPL_CHILDWINDOW(SpellDialogChildWindow, SID_SPELL_DIALOG)
}

#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include "OutlineViewShell.hxx"
#include <Outliner.hxx>
#include "drawdoc.hxx"


namespace sd {

SpellDialogChildWindow::SpellDialogChildWindow (
    ::Window* _pParent,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : ::svx::SpellDialogChildWindow (_pParent, nId, pBindings, pInfo),
      mpSdOutliner (NULL),
      mbOwnOutliner (false)
{
    ProvideOutliner();
}




SpellDialogChildWindow::~SpellDialogChildWindow (void)
{
    if (mpSdOutliner != NULL)
        mpSdOutliner->EndSpelling();

    if (mbOwnOutliner)
        delete mpSdOutliner;
}








SfxChildWinInfo SpellDialogChildWindow::GetInfo (void) const
{
    return ::svx::SpellDialogChildWindow::GetInfo();
}




void SpellDialogChildWindow::InvalidateSpellDialog (void)
{
    ::svx::SpellDialogChildWindow::InvalidateSpellDialog();
}




::svx::SpellPortions SpellDialogChildWindow::GetNextWrongSentence( bool /*bRecheck*/ )
{
    ::svx::SpellPortions aResult;

    if (mpSdOutliner != NULL)
    {
        ProvideOutliner();
        aResult = mpSdOutliner->GetNextSpellSentence();
    }

    // Close the spell check dialog when there are no more sentences to
    // check.
    if (aResult.size() == 0)
    {
        SfxBoolItem aItem (SID_SPELL_DIALOG, sal_False);
        GetBindings().GetDispatcher()->Execute(
            SID_SPELL_DIALOG,
            SFX_CALLMODE_ASYNCHRON,
            &aItem,
            0L);
    }

    return aResult;
}




void SpellDialogChildWindow::ApplyChangedSentence (
    const ::svx::SpellPortions& rChanged, bool bRecheck )
{
    if (mpSdOutliner != NULL)
    {
        OutlinerView* pOutlinerView = mpSdOutliner->GetView(0);
        if (pOutlinerView != NULL)
            mpSdOutliner->ApplyChangedSentence (
                pOutlinerView->GetEditView(),
                rChanged, bRecheck);
    }
}




void SpellDialogChildWindow::GetFocus (void)
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




void SpellDialogChildWindow::ProvideOutliner (void)
{
    ViewShellBase* pViewShellBase = PTR_CAST (ViewShellBase, SfxViewShell::Current());

    if (pViewShellBase != NULL)
    {
        ViewShell* pViewShell = pViewShellBase->GetMainViewShell().get();
        // If there already exists an outliner that has been created
        // for another view shell then destroy it first.
        if (mpSdOutliner != NULL)
            if ((pViewShell->ISA(DrawViewShell) && ! mbOwnOutliner)
                || (pViewShell->ISA(OutlineViewShell) && mbOwnOutliner))
            {
                mpSdOutliner->EndSpelling();
                if (mbOwnOutliner)
                    delete mpSdOutliner;
                mpSdOutliner = NULL;
            }

        // Now create/get an outliner if none is present.
        if (mpSdOutliner == NULL)
        {
            if (pViewShell->ISA(DrawViewShell))
            {
                // We need an outliner for the spell check so we have
                // to create one.
                mbOwnOutliner = true;
                mpSdOutliner = new Outliner (
                    pViewShell->GetDoc(),
                    OUTLINERMODE_TEXTOBJECT);
            }
            else if (pViewShell->ISA(OutlineViewShell))
            {
                // An outline view is already visible. The SdOutliner
                // will use it instead of creating its own.
                mbOwnOutliner = false;
                mpSdOutliner = pViewShell->GetDoc()->GetOutliner();
            }

            // Initialize spelling.
            if (mpSdOutliner != NULL)
            {
                mpSdOutliner->PrepareSpelling();
                mpSdOutliner->StartSpelling();
            }
        }
    }
}



} // end of namespace ::sd
