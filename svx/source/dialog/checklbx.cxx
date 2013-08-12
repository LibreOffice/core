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

#include <tools/shl.hxx>

#include <svtools/treelistentry.hxx>

#include <svx/checklbx.hxx>
#include <svx/dialmgr.hxx>

#include <svx/dialogs.hrc>

#include <vcl/builder.hxx>

// class SvxCheckListBox -------------------------------------------------

SvxCheckListBox::SvxCheckListBox( Window* pParent, WinBits nWinStyle ) :

    SvTreeListBox( pParent, nWinStyle )

{
    Init_Impl();
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxCheckListBox(Window *pParent, VclBuilder::stringmap &rMap)
{
    WinBits nWinStyle = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    return new SvxCheckListBox(pParent, nWinStyle);
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

void SvxCheckListBox::InsertEntry( const OUString& rStr, sal_uInt16 nPos,
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

            if (pItem && pItem->GetType() == SV_ITEM_ID_LBOXBUTTON)
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

SvTreeListEntry* SvxCheckListBox::InsertEntry( const OUString& rText, SvTreeListEntry* pParent, sal_Bool bChildrenOnDemand, sal_uIntPtr nPos, void* pUserData, SvLBoxButtonKind eButtonKind )
{
    return SvTreeListBox::InsertEntry( rText, pParent, bChildrenOnDemand, nPos, pUserData, eButtonKind );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
