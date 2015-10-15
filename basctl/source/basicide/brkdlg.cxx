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


#define _SVX_NOIDERESIDS
#include "breakpoint.hxx"
#include "brkdlg.hxx"
#include "basidesh.hxx"
#include "basidesh.hrc"

#include <sfx2/dispatch.hxx>

namespace basctl
{

// FIXME  Why does BreakPointDialog allow only sal_uInt16 for break-point line
// numbers, whereas BreakPoint supports sal_uLong?

namespace
{

bool lcl_ParseText(OUString const &rText, size_t& rLineNr )
{
    // aText should look like "# n" where
    // n > 0 && n < std::numeric_limits< sal_uInt16 >::max().
    // All spaces are ignored, so there can even be spaces within the
    // number n.  (Maybe it would be better to ignore all whitespace instead
    // of just spaces.)
    OUString aText(
        rText.replaceAll(" ", ""));
    if (aText.isEmpty())
        return false;
    sal_Unicode cFirst = aText[0];
    if (cFirst != '#' && !(cFirst >= '0' && cFirst <= '9'))
        return false;
    if (cFirst == '#')
        aText = aText.copy(1);
    // XXX Assumes that sal_uInt16 is contained within sal_Int32:
    sal_Int32 n = aText.toInt32();
    if ( n <= 0 )
        return false;
    rLineNr = static_cast< size_t >(n);
    return true;
}

} // namespace

BreakPointDialog::BreakPointDialog( vcl::Window* pParent, BreakPointList& rBrkPntList )
    : ModalDialog(pParent, "ManageBreakpointsDialog",
        "modules/BasicIDE/ui/managebreakpoints.ui")
    , m_rOriginalBreakPointList(rBrkPntList)
    , m_aModifiedBreakPointList(rBrkPntList)
{
    get(m_pComboBox, "entries");
    m_pComboBox->set_height_request(m_pComboBox->GetTextHeight() * 12);
    m_pComboBox->set_width_request(m_pComboBox->approximate_char_width() * 32);
    get(m_pOKButton, "ok");
    get(m_pNewButton, "new");
    get(m_pDelButton, "delete");
    get(m_pCheckBox, "active");
    get(m_pNumericField, "pass-nospin");

    m_pComboBox->SetUpdateMode(false);
    for ( size_t i = 0, n = m_aModifiedBreakPointList.size(); i < n; ++i )
    {
        BreakPoint* pBrk = m_aModifiedBreakPointList.at( i );
        OUString aEntryStr( "# " + OUString::number(pBrk->nLine) );
        m_pComboBox->InsertEntry( aEntryStr );
    }
    m_pComboBox->SetUpdateMode(true);

    m_pOKButton->SetClickHdl( LINK( this, BreakPointDialog, ButtonHdl ) );
    m_pNewButton->SetClickHdl( LINK( this, BreakPointDialog, ButtonHdl ) );
    m_pDelButton->SetClickHdl( LINK( this, BreakPointDialog, ButtonHdl ) );

    m_pCheckBox->SetClickHdl( LINK( this, BreakPointDialog, CheckBoxHdl ) );
    m_pComboBox->SetSelectHdl( LINK( this, BreakPointDialog, ComboBoxHighlightHdl ) );
    m_pComboBox->SetModifyHdl( LINK( this, BreakPointDialog, EditModifyHdl ) );
    m_pComboBox->GrabFocus();

    m_pNumericField->SetMin( 0 );
    m_pNumericField->SetMax( 0x7FFFFFFF );
    m_pNumericField->SetSpinSize( 1 );
    m_pNumericField->SetStrictFormat(true);
    m_pNumericField->SetModifyHdl( LINK( this, BreakPointDialog, EditModifyHdl ) );

    m_pComboBox->SetText( m_pComboBox->GetEntry( 0 ) );
    UpdateFields( m_aModifiedBreakPointList.at( 0 ) );

    CheckButtons();
}

BreakPointDialog::~BreakPointDialog()
{
    disposeOnce();
}

void BreakPointDialog::dispose()
{
    m_pComboBox.clear();
    m_pOKButton.clear();
    m_pNewButton.clear();
    m_pDelButton.clear();
    m_pNumericField.clear();
    m_pCheckBox.clear();
    ModalDialog::dispose();
}

void BreakPointDialog::SetCurrentBreakPoint( BreakPoint* pBrk )
{
    OUString aStr( "# " + OUString::number(pBrk->nLine) );
    m_pComboBox->SetText( aStr );
    UpdateFields( pBrk );
}

void BreakPointDialog::CheckButtons()
{
    // "New" button is enabled if the combo box edit contains a valid line
    // number that is not already present in the combo box list; otherwise
    // "OK" and "Delete" buttons are enabled:
    size_t nLine;
    if (lcl_ParseText(m_pComboBox->GetText(), nLine)
        && m_aModifiedBreakPointList.FindBreakPoint(nLine) == 0)
    {
        m_pNewButton->Enable();
        m_pOKButton->Disable();
        m_pDelButton->Disable();
    }
    else
    {
        m_pNewButton->Disable();
        m_pOKButton->Enable();
        m_pDelButton->Enable();
    }
}

IMPL_LINK_TYPED( BreakPointDialog, CheckBoxHdl, Button *, pButton, void )
{
    ::CheckBox * pChkBx = static_cast<::CheckBox*>(pButton);
    BreakPoint* pBrk = GetSelectedBreakPoint();
    if ( pBrk )
        pBrk->bEnabled = pChkBx->IsChecked();
}

IMPL_LINK_TYPED( BreakPointDialog, ComboBoxHighlightHdl, ComboBox&, rBox, void )
{
    m_pNewButton->Disable();
    m_pOKButton->Enable();
    m_pDelButton->Enable();

    sal_Int32 nEntry = rBox.GetEntryPos( rBox.GetText() );
    BreakPoint* pBrk = m_aModifiedBreakPointList.at( nEntry );
    DBG_ASSERT( pBrk, "Kein passender Breakpoint zur Liste ?" );
    UpdateFields( pBrk );
}



IMPL_LINK_TYPED( BreakPointDialog, EditModifyHdl, Edit&, rEdit, void )
{
    if (&rEdit == m_pComboBox)
        CheckButtons();
    else if (&rEdit == m_pNumericField)
    {
        BreakPoint* pBrk = GetSelectedBreakPoint();
        if ( pBrk )
            pBrk->nStopAfter = rEdit.GetText().toInt32();
    }
}



IMPL_LINK_TYPED( BreakPointDialog, ButtonHdl, Button *, pButton, void )
{
    if (pButton == m_pOKButton)
    {
        m_rOriginalBreakPointList.transfer(m_aModifiedBreakPointList);
        EndDialog( 1 );
    }
    else if (pButton == m_pNewButton)
    {
        // keep checkbox in mind!
        OUString aText( m_pComboBox->GetText() );
        size_t nLine;
        bool bValid = lcl_ParseText( aText, nLine );
        if ( bValid )
        {
            BreakPoint* pBrk = new BreakPoint( nLine );
            pBrk->bEnabled = m_pCheckBox->IsChecked();
            pBrk->nStopAfter = (size_t) m_pNumericField->GetValue();
            m_aModifiedBreakPointList.InsertSorted( pBrk );
            OUString aEntryStr( "# " + OUString::number(pBrk->nLine) );
            m_pComboBox->InsertEntry( aEntryStr );
            if (SfxDispatcher* pDispatcher = GetDispatcher())
                pDispatcher->Execute( SID_BASICIDE_BRKPNTSCHANGED );
        }
        else
        {
            m_pComboBox->SetText( aText );
            m_pComboBox->GrabFocus();
        }
        CheckButtons();
    }
    else if (pButton == m_pDelButton)
    {
        sal_Int32 nEntry = m_pComboBox->GetEntryPos( m_pComboBox->GetText() );
        BreakPoint* pBrk = m_aModifiedBreakPointList.at( nEntry );
        if ( pBrk )
        {
            delete m_aModifiedBreakPointList.remove( pBrk );
            m_pComboBox->RemoveEntryAt(nEntry);
            if ( nEntry && !( nEntry < m_pComboBox->GetEntryCount() ) )
                nEntry--;
            m_pComboBox->SetText( m_pComboBox->GetEntry( nEntry ) );
            if (SfxDispatcher* pDispatcher = GetDispatcher())
                pDispatcher->Execute( SID_BASICIDE_BRKPNTSCHANGED );
        }
        CheckButtons();
    }
}



void BreakPointDialog::UpdateFields( BreakPoint* pBrk )
{
    if ( pBrk )
    {
        m_pCheckBox->Check( pBrk->bEnabled );
        m_pNumericField->SetValue( pBrk->nStopAfter );
    }
}



BreakPoint* BreakPointDialog::GetSelectedBreakPoint()
{
    size_t nEntry = m_pComboBox->GetEntryPos( m_pComboBox->GetText() );
    BreakPoint* pBrk = m_aModifiedBreakPointList.at( nEntry );
    return pBrk;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
