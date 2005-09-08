/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SpellDialogChildWindow.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:31:07 $
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

#ifndef SVX_SPELL_DIALOG_CHILD_WINDOW_HXX
#include <SpellDialogChildWindow.hxx>
#endif

#include "svxdlg.hxx"

namespace svx {

/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
SpellDialogChildWindow::SpellDialogChildWindow (
    Window* pParent,
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : SfxChildWindow (pParent, nId)

{

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "SvxAbstractDialogFactory::Create() failed");
    m_pAbstractSpellDialog = pFact->CreateSvxSpellDialog(pParent,
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
void SpellDialogChildWindow::AddAutoCorrection(const String& rOld, const String& rNew, LanguageType eLanguage)
{
    DBG_ERROR("AutoCorrection should have been overloaded - if avalable")
}

} // end of namespace ::svx
