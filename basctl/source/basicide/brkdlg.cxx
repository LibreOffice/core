/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: brkdlg.cxx,v $
 * $Revision: 1.11 $
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
#include "precompiled_basctl.hxx"

#include <limits>

#include <vcl/sound.hxx>

// #define ITEMID_SEARCH SID_SEARCH_ITEM
#define _SVX_NOIDERESIDS
#include <brkdlg.hxx>
#include <brkdlg.hrc>
#include <basidesh.hxx>
#include <basidesh.hrc>
#include <iderdll.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

// FIXME  Why does BreakPointDialog allow only USHORT for break-point line
// numbers, whereas BreakPoint supports ULONG?

bool lcl_ParseText( String aText, USHORT& rLineNr )
{
    // aText should look like "# n" where
    // n > 0 && n < std::numeric_limits< USHORT >::max().
    // All spaces are ignored, so there can even be spaces within the
    // number n.  (Maybe it would be better to ignore all whitespace instead
    // of just spaces.)
    aText.EraseAllChars(' ');
    sal_Unicode cFirst = aText.GetChar(0);
    if (cFirst != '#' && !(cFirst >= '0' && cFirst <= '9'))
        return false;
    if (cFirst == '#')
        aText.Erase(0, 1);
    // XXX Assumes that USHORT is contained within sal_Int32:
    sal_Int32 n = aText.ToInt32();
    if (n <= 0 || n > std::numeric_limits< USHORT >::max())
        return false;
    rLineNr = static_cast< USHORT >(n);
    return true;
}

BreakPointDialog::BreakPointDialog( Window* pParent, BreakPointList& rBrkPntList ) :
        ModalDialog( pParent, IDEResId( RID_BASICIDE_BREAKPOINTDLG ) ),
        aComboBox(      this, IDEResId( RID_CB_BRKPOINTS ) ),
        aOKButton(      this, IDEResId( RID_PB_OK ) ),
        aCancelButton(  this, IDEResId( RID_PB_CANCEL ) ),
        aNewButton(     this, IDEResId( RID_PB_NEW ) ),
        aDelButton(     this, IDEResId( RID_PB_DEL ) ),
        aCheckBox(      this, IDEResId( RID_CHKB_ACTIVE ) ),
        aBrkText(       this, IDEResId( RID_FT_BRKPOINTS ) ),
        aPassText(      this, IDEResId( RID_FT_PASS ) ),
        aNumericField(  this, IDEResId( RID_FLD_PASS ) ),
        m_rOriginalBreakPointList(rBrkPntList),
        m_aModifiedBreakPointList(rBrkPntList)
{
    FreeResource();

    aComboBox.SetUpdateMode( FALSE );
    BreakPoint* pBrk = m_aModifiedBreakPointList.First();
    BreakPoint* pFirstBrk = pBrk;
    while ( pBrk )
    {
        String aEntryStr( RTL_CONSTASCII_USTRINGPARAM( "# " ) );
        aEntryStr += String::CreateFromInt32( pBrk->nLine );
        aComboBox.InsertEntry( aEntryStr, COMBOBOX_APPEND );
        pBrk = m_aModifiedBreakPointList.Next();
    }
    aComboBox.SetUpdateMode( TRUE );

    aOKButton.SetClickHdl( LINK( this, BreakPointDialog, ButtonHdl ) );
    aNewButton.SetClickHdl( LINK( this, BreakPointDialog, ButtonHdl ) );
    aDelButton.SetClickHdl( LINK( this, BreakPointDialog, ButtonHdl ) );
//  aShowButton.SetClickHdl( LINK( this, BreakPointDialog, ButtonHdl ) );

    aCheckBox.SetClickHdl( LINK( this, BreakPointDialog, CheckBoxHdl ) );
    aComboBox.SetSelectHdl( LINK( this, BreakPointDialog, ComboBoxHighlightHdl ) );
    aComboBox.SetModifyHdl( LINK( this, BreakPointDialog, EditModifyHdl ) );
    aComboBox.GrabFocus();

    aNumericField.SetMin( 0 );
    aNumericField.SetMax( 0x7FFFFFFF );
    aNumericField.SetSpinSize( 1 );
    aNumericField.SetStrictFormat( TRUE );
    aNumericField.SetModifyHdl( LINK( this, BreakPointDialog, EditModifyHdl ) );

    aComboBox.SetText( aComboBox.GetEntry( 0 ) );
    UpdateFields( pFirstBrk );

    CheckButtons();
}

void BreakPointDialog::SetCurrentBreakPoint( BreakPoint* pBrk )
{
    String aStr( RTL_CONSTASCII_USTRINGPARAM( "# " ) );
    aStr += String::CreateFromInt32( pBrk->nLine );
    aComboBox.SetText( aStr );
    UpdateFields( pBrk );
}

void BreakPointDialog::CheckButtons()
{
    // "New" button is enabled if the combo box edit contains a valid line
    // number that is not already present in the combo box list; otherwise
    // "OK" and "Delete" buttons are enabled:
    USHORT nLine;
    if (lcl_ParseText(aComboBox.GetText(), nLine)
        && m_aModifiedBreakPointList.FindBreakPoint(nLine) == 0)
    {
        aNewButton.Enable();
        aOKButton.Disable();
        aDelButton.Disable();
    }
    else
    {
        aNewButton.Disable();
        aOKButton.Enable();
        aDelButton.Enable();
    }
}

IMPL_LINK_INLINE_START( BreakPointDialog, CheckBoxHdl, CheckBox *, pChkBx )
{
    BreakPoint* pBrk = GetSelectedBreakPoint();
    if ( pBrk )
        pBrk->bEnabled = pChkBx->IsChecked();

    return 0;
}
IMPL_LINK_INLINE_END( BreakPointDialog, CheckBoxHdl, CheckBox *, pChkBx )



IMPL_LINK( BreakPointDialog, ComboBoxHighlightHdl, ComboBox *, pBox )
{
    aNewButton.Disable();
    aOKButton.Enable();
    aDelButton.Enable();

    USHORT nEntry = pBox->GetEntryPos( pBox->GetText() );
    BreakPoint* pBrk = m_aModifiedBreakPointList.GetObject( nEntry );
    DBG_ASSERT( pBrk, "Kein passender Breakpoint zur Liste ?" );
    UpdateFields( pBrk );

    return 0;
}



IMPL_LINK( BreakPointDialog, EditModifyHdl, Edit *, pEdit )
{
    if ( pEdit == &aComboBox )
        CheckButtons();
    else if ( pEdit == &aNumericField )
    {
        BreakPoint* pBrk = GetSelectedBreakPoint();
        if ( pBrk )
            pBrk->nStopAfter = pEdit->GetText().ToInt32();
    }
    return 0;
}



IMPL_LINK( BreakPointDialog, ButtonHdl, Button *, pButton )
{
    if ( pButton == &aOKButton )
    {
        m_rOriginalBreakPointList.transfer(m_aModifiedBreakPointList);
        EndDialog( 1 );
    }
    else if ( pButton == &aNewButton )
    {
        // Checkbox beruecksichtigen!
        String aText( aComboBox.GetText() );
        USHORT nLine;
        BOOL bValid = lcl_ParseText( aText, nLine );
        if ( bValid )
        {
            BreakPoint* pBrk = new BreakPoint( nLine );
            pBrk->bEnabled = aCheckBox.IsChecked();
            pBrk->nStopAfter = (ULONG) aNumericField.GetValue();
            m_aModifiedBreakPointList.InsertSorted( pBrk );
            String aEntryStr( RTL_CONSTASCII_USTRINGPARAM( "# " ) );
            aEntryStr += String::CreateFromInt32( pBrk->nLine );
            aComboBox.InsertEntry( aEntryStr, COMBOBOX_APPEND );
            BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
            SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
            SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
            if( pDispatcher )
            {
                pDispatcher->Execute( SID_BASICIDE_BRKPNTSCHANGED );
            }
        }
        else
        {
            aComboBox.SetText( aText );
            aComboBox.GrabFocus();
            Sound::Beep();
        }
        CheckButtons();
    }
    else if ( pButton == &aDelButton )
    {
        USHORT nEntry = aComboBox.GetEntryPos( aComboBox.GetText() );
        BreakPoint* pBrk = m_aModifiedBreakPointList.GetObject( nEntry );
        if ( pBrk )
        {
            delete m_aModifiedBreakPointList.Remove( pBrk );
            aComboBox.RemoveEntry( nEntry );
            if ( nEntry && !( nEntry < aComboBox.GetEntryCount() ) )
                nEntry--;
            aComboBox.SetText( aComboBox.GetEntry( nEntry ) );
            BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
            SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
            SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;

            if( pDispatcher )
            {
                pDispatcher->Execute( SID_BASICIDE_BRKPNTSCHANGED );
            }
        }
        CheckButtons();
    }
//  else if ( pButton == &aShowButton )
//  {
//      ;
//  }

    return 0;
}



void BreakPointDialog::UpdateFields( BreakPoint* pBrk )
{
    if ( pBrk )
    {
        aCheckBox.Check( pBrk->bEnabled );
        aNumericField.SetValue( pBrk->nStopAfter );
    }
}



BreakPoint* BreakPointDialog::GetSelectedBreakPoint()
{
    USHORT nEntry = aComboBox.GetEntryPos( aComboBox.GetText() );
    BreakPoint* pBrk = m_aModifiedBreakPointList.GetObject( nEntry );
    return pBrk;
}



