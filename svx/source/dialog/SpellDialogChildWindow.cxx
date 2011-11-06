/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/SpellDialogChildWindow.hxx>

#include <svx/svxdlg.hxx>

namespace svx {

/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
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
    DBG_ERROR("AutoCorrection should have been overloaded - if available");
}
/*-- 16.06.2008 10:11:57---------------------------------------------------

  -----------------------------------------------------------------------*/
bool SpellDialogChildWindow::HasGrammarChecking()
{
    return false;
}
/*-- 18.06.2008 12:26:35---------------------------------------------------

  -----------------------------------------------------------------------*/
bool SpellDialogChildWindow::IsGrammarChecking()
{
    DBG_ERROR("Grammar checking should have been overloaded - if available");
    return false;
}
/*-- 18.06.2008 12:26:35---------------------------------------------------

  -----------------------------------------------------------------------*/
void SpellDialogChildWindow::SetGrammarChecking(bool )
{
    DBG_ERROR("Grammar checking should have been overloaded - if available");
}
} // end of namespace ::svx
