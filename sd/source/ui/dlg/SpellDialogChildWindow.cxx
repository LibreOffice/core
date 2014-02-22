/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "SpellDialogChildWindow.hxx"
#include <svx/svxids.hrc>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

namespace sd{

SFX_IMPL_CHILDWINDOW_WITHID(SpellDialogChildWindow, SID_SPELL_DIALOG)
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

    
    
    if (aResult.empty())
    {
        SfxBoolItem aItem (SID_SPELL_DIALOG, false);
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
        
        
        if (mpSdOutliner != NULL)
            if ((pViewShell->ISA(DrawViewShell) && ! mbOwnOutliner)
                || (pViewShell->ISA(OutlineViewShell) && mbOwnOutliner))
            {
                mpSdOutliner->EndSpelling();
                if (mbOwnOutliner)
                    delete mpSdOutliner;
                mpSdOutliner = NULL;
            }

        
        if (mpSdOutliner == NULL)
        {
            if (pViewShell->ISA(DrawViewShell))
            {
                
                
                mbOwnOutliner = true;
                mpSdOutliner = new Outliner (
                    pViewShell->GetDoc(),
                    OUTLINERMODE_TEXTOBJECT);
            }
            else if (pViewShell->ISA(OutlineViewShell))
            {
                
                
                mbOwnOutliner = false;
                mpSdOutliner = pViewShell->GetDoc()->GetOutliner();
            }

            
            if (mpSdOutliner != NULL)
            {
                mpSdOutliner->PrepareSpelling();
                mpSdOutliner->StartSpelling();
            }
        }
    }
}



} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
