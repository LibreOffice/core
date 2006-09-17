/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: checklbx.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:11:54 $
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

// include ---------------------------------------------------------------

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif

#define _SVX_CHECKLBX_CXX

#include "dialogs.hrc"
#include "checklbx.hrc"

#include "checklbx.hxx"
#include "dialmgr.hxx"

// class SvxCheckListBox -------------------------------------------------

SvxCheckListBox::SvxCheckListBox( Window* pParent, WinBits nWinStyle ) :

    SvTreeListBox( pParent, nWinStyle )

{
    Init_Impl();
}

// -----------------------------------------------------------------------

SvxCheckListBox::SvxCheckListBox( Window* pParent, const ResId& rResId ) :

    SvTreeListBox( pParent, rResId )

{
    Init_Impl();
}

// -----------------------------------------------------------------------

SvxCheckListBox::~SvxCheckListBox()
{
    delete pCheckButton;
}

// -----------------------------------------------------------------------

void SvxCheckListBox::Init_Impl()
{
    pCheckButton = new SvLBoxButtonData( this );
    EnableCheckButton( pCheckButton );
}

// -----------------------------------------------------------------------

void SvxCheckListBox::InsertEntry( const String& rStr, USHORT nPos )
{
    SvTreeListBox::InsertEntry( rStr, NULL, FALSE, nPos );
}

// -----------------------------------------------------------------------

void SvxCheckListBox::RemoveEntry( USHORT nPos )
{
    if ( nPos < GetEntryCount() )
        SvTreeListBox::GetModel()->Remove( GetEntry( nPos ) );
}

// -----------------------------------------------------------------------

void SvxCheckListBox::SelectEntryPos( USHORT nPos, BOOL bSelect )
{
    if ( nPos < GetEntryCount() )
        Select( GetEntry( nPos ), bSelect );
}

// -----------------------------------------------------------------------

USHORT SvxCheckListBox::GetSelectEntryPos() const
{
    SvLBoxEntry* pEntry = GetCurEntry();

    if ( pEntry )
        return (USHORT)GetModel()->GetAbsPos( pEntry );
    return LISTBOX_ENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

String SvxCheckListBox::GetText( USHORT nPos ) const
{
    SvLBoxEntry* pEntry = GetEntry( nPos );

    if ( pEntry )
        return GetEntryText( pEntry );
    return String();
}

// -----------------------------------------------------------------------

USHORT SvxCheckListBox::GetCheckedEntryCount() const
{
    USHORT nCheckCount = 0;
    USHORT nCount = (USHORT)GetEntryCount();

    for ( USHORT i = 0; i < nCount; ++i )
    {
        if ( IsChecked( i ) )
            nCheckCount++;
    }
    return nCheckCount;
}

// -----------------------------------------------------------------------

void SvxCheckListBox::CheckEntryPos( USHORT nPos, BOOL bCheck )
{
    if ( nPos < GetEntryCount() )
        SetCheckButtonState(
            GetEntry( nPos ), bCheck ? SvButtonState( SV_BUTTON_CHECKED ) :
                                       SvButtonState( SV_BUTTON_UNCHECKED ) );
}

// -----------------------------------------------------------------------

BOOL SvxCheckListBox::IsChecked( USHORT nPos ) const
{
    if ( nPos < GetEntryCount() )
        return (GetCheckButtonState( GetEntry( nPos ) ) == SV_BUTTON_CHECKED);
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void* SvxCheckListBox::SetEntryData ( USHORT nPos, void* pNewData )
{
    void* pOld = NULL;

    if ( nPos < GetEntryCount() )
    {
        pOld = GetEntry( nPos )->GetUserData();
        GetEntry( nPos )->SetUserData( pNewData );
    }
    return pOld;
}

// -----------------------------------------------------------------------

void* SvxCheckListBox::GetEntryData( USHORT nPos ) const
{
    if ( nPos < GetEntryCount() )
        return GetEntry( nPos )->GetUserData();
    else
        return NULL;
}

// -----------------------------------------------------------------------

void SvxCheckListBox::ToggleCheckButton( SvLBoxEntry* pEntry )
{
    if ( pEntry )
    {
        if ( !IsSelected( pEntry ) )
            Select( pEntry );
        else
            CheckEntryPos( GetSelectEntryPos(), !IsChecked( GetSelectEntryPos() ) );
    }
}

// -----------------------------------------------------------------------

void SvxCheckListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        Point aPnt = rMEvt.GetPosPixel();
        SvLBoxEntry* pEntry = GetEntry( aPnt );

        if ( pEntry )
        {
            BOOL bCheck = ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED );
            SvLBoxItem* pItem = GetItem( pEntry, aPnt.X() );

            if ( pItem && pItem->IsA() == SV_ITEM_ID_LBOXBUTTON )
            {
                SvTreeListBox::MouseButtonDown( rMEvt );
                Select( pEntry, TRUE );
                return;
            }
            else
            {
                ToggleCheckButton( pEntry );
                SvTreeListBox::MouseButtonDown( rMEvt );
                if ( bCheck != ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED ) )
                    CheckButtonHdl();
                return;
            }
        }
    }
    SvTreeListBox::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void SvxCheckListBox::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode& rKey = rKEvt.GetKeyCode();

    if ( rKey.GetCode() == KEY_RETURN || rKey.GetCode() == KEY_SPACE )
    {
        SvLBoxEntry* pEntry = GetCurEntry();

        if ( pEntry )
        {
            BOOL bCheck = ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED );
            ToggleCheckButton( pEntry );
            if ( bCheck != ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED ) )
                CheckButtonHdl();
        }
    }
    else if ( GetEntryCount() )
        SvTreeListBox::KeyInput( rKEvt );
}

