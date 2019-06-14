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
#include <osl/diagnose.h>

namespace svx {


SpellDialogChildWindow::SpellDialogChildWindow (
    vcl::Window* _pParent,
    sal_uInt16 nId,
    SfxBindings* pBindings)
    : SfxChildWindow (_pParent, nId)
{
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    m_xAbstractSpellDialog = pFact->CreateSvxSpellDialog(_pParent->GetFrameWeld(),
                                            pBindings,
                                            this );
    SetController(m_xAbstractSpellDialog->GetController());
    SetHideNotDelete(true);
}

SpellDialogChildWindow::~SpellDialogChildWindow()
{
    m_xAbstractSpellDialog.disposeAndClear();
}

SfxBindings& SpellDialogChildWindow::GetBindings() const
{
    assert(m_xAbstractSpellDialog);
    return m_xAbstractSpellDialog->GetBindings();
}

void SpellDialogChildWindow::InvalidateSpellDialog()
{
    OSL_ASSERT (m_xAbstractSpellDialog);
    if (m_xAbstractSpellDialog)
        m_xAbstractSpellDialog->InvalidateDialog();
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
    OSL_FAIL("AutoCorrection should have been overridden - if available");
}

bool SpellDialogChildWindow::HasGrammarChecking()
{
    return false;
}

bool SpellDialogChildWindow::IsGrammarChecking()
{
    OSL_FAIL("Grammar checking should have been overridden - if available");
    return false;
}

void SpellDialogChildWindow::SetGrammarChecking(bool )
{
    OSL_FAIL("Grammar checking should have been overridden - if available");
}
} // end of namespace ::svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
