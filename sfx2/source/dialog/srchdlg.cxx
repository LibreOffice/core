/*************************************************************************
 *
 *  $RCSfile: srchdlg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 14:05:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "srchdlg.hxx"
#include "sfxresid.hxx"
#include "sfxuno.hxx"

#include "srchdlg.hrc"
#include "dialog.hrc"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif

using namespace ::com::sun::star::uno;

// ============================================================================

namespace sfx2 {

#define USERITEM_NAME       DEFINE_CONST_OUSTRING("UserItem")
#define MAX_SAVE_COUNT      (USHORT)10

// ============================================================================
// SearchDialog
// ============================================================================

SearchDialog::SearchDialog( Window* pWindow, const ::rtl::OUString& rConfigName ) :

    ModelessDialog( pWindow, SfxResId( RID_DLG_SEARCH ) ),

    m_aSearchLabel      ( this, ResId( FT_SEARCH ) ),
    m_aSearchEdit       ( this, ResId( ED_SEARCH ) ),
    m_aWholeWordsBox    ( this, ResId( CB_WHOLEWORDS ) ),
    m_aMatchCaseBox     ( this, ResId( CB_MATCHCASE ) ),
    m_aWrapAroundBox    ( this, ResId( CB_WRAPAROUND ) ),
    m_aBackwardsBox     ( this, ResId( CB_BACKWARDS ) ),
    m_aFindBtn          ( this, ResId( PB_FIND ) ),
    m_aCancelBtn        ( this, ResId( PB_CANCELFIND ) ),
    m_sToggleText       (       ResId( STR_TOGGLE ) ),
    m_sConfigName       ( rConfigName ),
    m_bIsConstructed    ( false )

{
    FreeResource();

    // set handler
    m_aFindBtn.SetClickHdl( LINK( this, SearchDialog, FindHdl ) );
    m_aBackwardsBox.SetClickHdl( LINK( this, SearchDialog, ToggleHdl ) );
    // load config: old search strings and the status of the check boxes
    LoadConfig();
    // we need to change the text of the WrapAround box, depends on the status of the Backwards box
    if ( m_aBackwardsBox.IsChecked() )
        ToggleHdl( &m_aBackwardsBox );
    // the search edit should have the focus
    m_aSearchEdit.GrabFocus();
}

SearchDialog::~SearchDialog()
{
    SaveConfig();
}

void SearchDialog::LoadConfig()
{
    SvtViewOptions aViewOpt( E_DIALOG, m_sConfigName );
    if ( aViewOpt.Exists() )
    {
        m_sWinState = ByteString( aViewOpt.GetWindowState().getStr(), RTL_TEXTENCODING_ASCII_US );
        Any aUserItem = aViewOpt.GetUserItem( USERITEM_NAME );
        ::rtl::OUString aTemp;
        if ( aUserItem >>= aTemp )
        {
            String sUserData( aTemp );
            DBG_ASSERT( sUserData.GetTokenCount() == 5, "invalid config data" );
            xub_StrLen nIdx = 0;
            String sSearchText = sUserData.GetToken( 0, ';', nIdx );
            m_aWholeWordsBox.Check( sUserData.GetToken( 0, ';', nIdx ).ToInt32() == 1 );
            m_aMatchCaseBox.Check( sUserData.GetToken( 0, ';', nIdx ).ToInt32() == 1 );
            m_aWrapAroundBox.Check( sUserData.GetToken( 0, ';', nIdx ).ToInt32() == 1 );
            m_aBackwardsBox.Check( sUserData.GetToken( 0, ';', nIdx ).ToInt32() == 1 );

            nIdx = 0;
            while ( nIdx != STRING_NOTFOUND )
                m_aSearchEdit.InsertEntry( sSearchText.GetToken( 0, '\t', nIdx ) );
            m_aSearchEdit.SelectEntryPos(0);
        }
    }
    else
        m_aWrapAroundBox.Check( TRUE );
}

void SearchDialog::SaveConfig()
{
    SvtViewOptions aViewOpt( E_DIALOG, m_sConfigName );
    aViewOpt.SetWindowState( rtl::OUString::createFromAscii( m_sWinState.GetBuffer() ) );
    String sUserData;
    USHORT i = 0, nCount = Min( m_aSearchEdit.GetEntryCount(), MAX_SAVE_COUNT );
    for ( ; i < nCount; ++i )
    {
        sUserData += m_aSearchEdit.GetEntry(i);
        sUserData += '\t';
    }
    sUserData.EraseTrailingChars( '\t' );
    sUserData += ';';
    sUserData += String::CreateFromInt32( m_aWholeWordsBox.IsChecked() ? 1 : 0 );
    sUserData += ';';
    sUserData += String::CreateFromInt32( m_aMatchCaseBox.IsChecked() ? 1 : 0 );
    sUserData += ';';
    sUserData += String::CreateFromInt32( m_aWrapAroundBox.IsChecked() ? 1 : 0 );
    sUserData += ';';
    sUserData += String::CreateFromInt32( m_aBackwardsBox.IsChecked() ? 1 : 0 );

    Any aUserItem = makeAny( ::rtl::OUString( sUserData ) );
    aViewOpt.SetUserItem( USERITEM_NAME, aUserItem );
}

IMPL_LINK( SearchDialog, FindHdl, PushButton*, EMPTYARG )
{
    String sSrchTxt = m_aSearchEdit.GetText();
    USHORT nPos = m_aSearchEdit.GetEntryPos( sSrchTxt );
    if ( nPos > 0 && nPos != COMBOBOX_ENTRY_NOTFOUND )
        m_aSearchEdit.RemoveEntry( nPos );
    if ( nPos > 0 )
        m_aSearchEdit.InsertEntry( sSrchTxt, 0 );
    m_aFindHdl.Call( this );
    return 0;
}

IMPL_LINK( SearchDialog, ToggleHdl, CheckBox*, EMPTYARG )
{
    String sTemp = m_aWrapAroundBox.GetText();
    m_aWrapAroundBox.SetText( m_sToggleText );
    m_sToggleText = sTemp;
    return 0;
}

void SearchDialog::SetFocusOnEdit()
{
    Selection aSelection( 0, m_aSearchEdit.GetText().Len() );
    m_aSearchEdit.SetSelection( aSelection );
    m_aSearchEdit.GrabFocus();
}

BOOL SearchDialog::Close()
{
    Point aPos = GetPosPixel();

    BOOL bRet = ModelessDialog::Close();
    m_aCloseHdl.Call( this );
    return bRet;
}

void SearchDialog::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        if ( m_sWinState.Len() )
            SetWindowState( m_sWinState );
        m_bIsConstructed = TRUE;
    }

    ModelessDialog::StateChanged( nStateChange );
}

void SearchDialog::Move()
{
    ModelessDialog::Move();
    if ( m_bIsConstructed && IsReallyVisible() )
        m_sWinState = GetWindowState( WINDOWSTATE_MASK_POS | WINDOWSTATE_MASK_STATE );
}

// ============================================================================

} // namespace sfx2

// ============================================================================

