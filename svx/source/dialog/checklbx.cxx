/*************************************************************************
 *
 *  $RCSfile: checklbx.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:07 $
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

// include ---------------------------------------------------------------

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#pragma hdrstop

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

__EXPORT SvxCheckListBox::~SvxCheckListBox()
{
    delete pCheckButton;
}

// -----------------------------------------------------------------------

void SvxCheckListBox::Init_Impl()
{
    SvxCheckListBoxBitmaps theBmps;

    pCheckButton = new SvLBoxButtonData();
    pCheckButton->aBmps[SV_BMP_UNCHECKED]   = theBmps.GetUncheckedBmp();
    pCheckButton->aBmps[SV_BMP_CHECKED]     = theBmps.GetCheckedBmp();
    pCheckButton->aBmps[SV_BMP_HICHECKED]   = theBmps.GetHiCheckedBmp();
    pCheckButton->aBmps[SV_BMP_HIUNCHECKED] = theBmps.GetHiUncheckedBmp();
    pCheckButton->aBmps[SV_BMP_TRISTATE]    = theBmps.GetTriStateBmp();
    pCheckButton->aBmps[SV_BMP_HITRISTATE]  = theBmps.GetHiTriStateBmp();
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
    {
        SvTreeListBox::GetModel()->Remove( GetEntry( nPos ) );
    }
}

// -----------------------------------------------------------------------

void SvxCheckListBox::SelectEntryPos( USHORT nPos, BOOL bSelect )
{
    if ( nPos < GetEntryCount() )
    {
        Select( GetEntry( nPos ), bSelect );
    }
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
        if ( IsChecked( i ) )
            nCheckCount++;
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
    if ( pEntry && IsSelected( pEntry ) )
        CheckEntryPos( GetSelectEntryPos(),
                       !IsChecked( GetSelectEntryPos() ) );
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
            BOOL bCheck =
                ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED );
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

                if ( bCheck !=
                     ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED ) )
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
            BOOL bCheck = ( GetCheckButtonState( pEntry ) ==
                            SV_BUTTON_CHECKED );
            ToggleCheckButton( pEntry );

            if ( bCheck != ( GetCheckButtonState( pEntry ) ==
                             SV_BUTTON_CHECKED ) )
                CheckButtonHdl();
        }
    }
    else if ( GetEntryCount() )
        SvTreeListBox::KeyInput( rKEvt );
}

// class SvxCheckListBoxBitmaps ------------------------------------------

SvxCheckListBoxBitmaps::SvxCheckListBoxBitmaps() :

    Resource        ( ResId( RID_CHECKLISTBOX_BITMAPS, DIALOG_MGR() ) ),

    aUncheckedBmp   ( ResId( BMP_UNCHECKED ) ),
    aCheckedBmp     ( ResId( BMP_CHECKED ) ),
    aHiCheckedBmp   ( ResId( BMP_HICHECKED ) ),
    aHiUncheckedBmp ( ResId( BMP_HIUNCHECKED ) ),
    aTriStateBmp    ( ResId( BMP_TRISTATE ) ),
    aHiTriStateBmp  ( ResId( BMP_HITRISTATE ) )

{
    FreeResource();
}


