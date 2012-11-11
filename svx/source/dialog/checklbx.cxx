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

#include <tools/shl.hxx>

#define _SVX_CHECKLBX_CXX

#include <svx/checklbx.hxx>
#include <svx/dialmgr.hxx>

#include <svx/dialogs.hrc>

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

SvxCheckListBox::SvxCheckListBox( Window* pParent, const ResId& rResId,
                                  const Image& rNormalStaticImage )
    : SvTreeListBox( pParent, rResId )

{
    Init_Impl();
    pCheckButton->aBmps[SV_BMP_STATICIMAGE] = rNormalStaticImage;
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

void SvxCheckListBox::InsertEntry( const String& rStr, sal_uInt16 nPos,
                                   void* pUserData,
                                   SvLBoxButtonKind eButtonKind )
{
    SvTreeListBox::InsertEntry( rStr, NULL, sal_False, nPos, pUserData,
                                eButtonKind );
}

// -----------------------------------------------------------------------

void SvxCheckListBox::RemoveEntry( sal_uInt16 nPos )
{
    if ( nPos < GetEntryCount() )
        SvTreeListBox::GetModel()->Remove( GetEntry( nPos ) );
}

// -----------------------------------------------------------------------

void SvxCheckListBox::SelectEntryPos( sal_uInt16 nPos, sal_Bool bSelect )
{
    if ( nPos < GetEntryCount() )
        Select( GetEntry( nPos ), bSelect );
}

// -----------------------------------------------------------------------

sal_uInt16 SvxCheckListBox::GetSelectEntryPos() const
{
    SvTreeListEntry* pEntry = GetCurEntry();

    if ( pEntry )
        return (sal_uInt16)GetModel()->GetAbsPos( pEntry );
    return LISTBOX_ENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

String SvxCheckListBox::GetText( sal_uInt16 nPos ) const
{
    SvTreeListEntry* pEntry = GetEntry( nPos );

    if ( pEntry )
        return GetEntryText( pEntry );
    return String();
}

// -----------------------------------------------------------------------

sal_uInt16 SvxCheckListBox::GetCheckedEntryCount() const
{
    sal_uInt16 nCheckCount = 0;
    sal_uInt16 nCount = (sal_uInt16)GetEntryCount();

    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        if ( IsChecked( i ) )
            nCheckCount++;
    }
    return nCheckCount;
}

// -----------------------------------------------------------------------

void SvxCheckListBox::CheckEntryPos( sal_uInt16 nPos, sal_Bool bCheck )
{
    if ( nPos < GetEntryCount() )
        SetCheckButtonState(
            GetEntry( nPos ), bCheck ? SvButtonState( SV_BUTTON_CHECKED ) :
                                       SvButtonState( SV_BUTTON_UNCHECKED ) );
}

// -----------------------------------------------------------------------

sal_Bool SvxCheckListBox::IsChecked( sal_uInt16 nPos ) const
{
    if ( nPos < GetEntryCount() )
        return (GetCheckButtonState( GetEntry( nPos ) ) == SV_BUTTON_CHECKED);
    else
        return sal_False;
}

// -----------------------------------------------------------------------

void* SvxCheckListBox::SetEntryData ( sal_uInt16 nPos, void* pNewData )
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

void* SvxCheckListBox::GetEntryData( sal_uInt16 nPos ) const
{
    if ( nPos < GetEntryCount() )
        return GetEntry( nPos )->GetUserData();
    else
        return NULL;
}

// -----------------------------------------------------------------------

void SvxCheckListBox::ToggleCheckButton( SvTreeListEntry* pEntry )
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
        const Point aPnt = rMEvt.GetPosPixel();
        SvTreeListEntry* pEntry = GetEntry( aPnt );

        if ( pEntry )
        {
            sal_Bool bCheck = ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED );
            SvLBoxItem* pItem = GetItem( pEntry, aPnt.X() );

            if ( pItem && pItem->IsA() == SV_ITEM_ID_LBOXBUTTON )
            {
                SvTreeListBox::MouseButtonDown( rMEvt );
                Select( pEntry, sal_True );
                return;
            }
            else
            {
                ToggleCheckButton( pEntry );
                SvTreeListBox::MouseButtonDown( rMEvt );

                // check if the entry below the mouse changed during the base method call. This is possible if,
                // for instance, a handler invoked by the base class tampers with the list entries.
                const SvTreeListEntry* pNewEntry = GetEntry( aPnt );
                if ( pNewEntry != pEntry )
                    return;

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
        SvTreeListEntry* pEntry = GetCurEntry();

        if ( pEntry )
        {
            sal_Bool bCheck = ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED );
            ToggleCheckButton( pEntry );
            if ( bCheck != ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED ) )
                CheckButtonHdl();
        }
    }
    else if ( GetEntryCount() )
        SvTreeListBox::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

SvTreeListEntry* SvxCheckListBox::InsertEntry( const XubString& rText, SvTreeListEntry* pParent, sal_Bool bChildrenOnDemand, sal_uIntPtr nPos, void* pUserData, SvLBoxButtonKind eButtonKind )
{
    return SvTreeListBox::InsertEntry( rText, pParent, bChildrenOnDemand, nPos, pUserData, eButtonKind );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
