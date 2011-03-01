/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_svx.hxx"
#include <svx/SpellDialogChildWindow.hxx>

#include <svx/svxdlg.hxx>

namespace svx {

/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
SpellDialogChildWindow::SpellDialogChildWindow (
    Window* _pParent,
    USHORT nId,
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
    SetHideNotDelete (TRUE);
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
SpellDialogChildWindow::~SpellDialogChildWindow (void)
{
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
SfxBindings& SpellDialogChildWindow::GetBindings (void) const
{
    OSL_ASSERT (m_pAbstractSpellDialog != NULL);
    return m_pAbstractSpellDialog->GetBindings();
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
void SpellDialogChildWindow::InvalidateSpellDialog()
{
    OSL_ASSERT (m_pAbstractSpellDialog != NULL);
    if(m_pAbstractSpellDialog)
        m_pAbstractSpellDialog->Invalidate();
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
bool SpellDialogChildWindow::HasAutoCorrection()
{
    return false;
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
void SpellDialogChildWindow::AddAutoCorrection(
        const String& /*rOld*/,
        const String& /*rNew*/,
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
