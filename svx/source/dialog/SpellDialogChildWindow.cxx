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

#include <svx/SpellDialogChildWindow.hxx>

#include <svx/svxdlg.hxx>

namespace svx {


SpellDialogChildWindow::SpellDialogChildWindow (
    Window* _pParent,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* /*pInfo*/)
    : SfxChildWindow (_pParent, nId)

{

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "SvxAbstractDialogFactory::Create() failed");
    m_pAbstractSpellDialog = pFact->CreateSvxSpellDialog(_pParent,
                                            pBindings,
                                            this );
    pWindow = m_pAbstractSpellDialog->GetWindow();
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    SetHideNotDelete (sal_True);
}

SpellDialogChildWindow::~SpellDialogChildWindow (void)
{
}

SfxBindings& SpellDialogChildWindow::GetBindings (void) const
{
    OSL_ASSERT (m_pAbstractSpellDialog != NULL);
    return m_pAbstractSpellDialog->GetBindings();
}

void SpellDialogChildWindow::InvalidateSpellDialog()
{
    OSL_ASSERT (m_pAbstractSpellDialog != NULL);
    if(m_pAbstractSpellDialog)
        m_pAbstractSpellDialog->Invalidate();
}

bool SpellDialogChildWindow::HasAutoCorrection()
{
    return false;
}

void SpellDialogChildWindow::AddAutoCorrection(
        const OUString& /*rOld*/,
        const OUString& /*rNew*/,
        LanguageType /*eLanguage*/)
{
    OSL_FAIL("AutoCorrection should have been overloaded - if available");
}

bool SpellDialogChildWindow::HasGrammarChecking()
{
    return false;
}

bool SpellDialogChildWindow::IsGrammarChecking()
{
    OSL_FAIL("Grammar checking should have been overloaded - if available");
    return false;
}

void SpellDialogChildWindow::SetGrammarChecking(bool )
{
    OSL_FAIL("Grammar checking should have been overloaded - if available");
}
} // end of namespace ::svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
