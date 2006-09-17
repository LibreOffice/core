/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SpellDialogChildWindow.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:08:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef SVX_SPELL_DIALOG_CHILD_WINDOW_HXX
#include <SpellDialogChildWindow.hxx>
#endif

#include "svxdlg.hxx"

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
    DBG_ERROR("AutoCorrection should have been overloaded - if avalable")
}

} // end of namespace ::svx
