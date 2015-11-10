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

#include <svtools/treelistentry.hxx>
#include <svx/checklbx.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <vcl/builderfactory.hxx>

SvxCheckListBox::SvxCheckListBox( vcl::Window* pParent, WinBits nWinStyle ) :

    SvTreeListBox( pParent, nWinStyle )

{
    Init_Impl();
}

VCL_BUILDER_DECL_FACTORY(SvxCheckListBox)
{
    WinBits nWinStyle = WB_TABSTOP;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
        nWinStyle |= WB_BORDER;
    rRet = VclPtr<SvxCheckListBox>::Create(pParent, nWinStyle);
}

void SvxCheckListBox::SetNormalStaticImage(const Image& rNormalStaticImage)
{
    pCheckButton->SetImage(SvBmp::STATICIMAGE, rNormalStaticImage);
}

SvxCheckListBox::~SvxCheckListBox()
{
    disposeOnce();
}

void SvxCheckListBox::dispose()
{
    delete pCheckButton;
    SvTreeListBox::dispose();
}

void SvxCheckListBox::Init_Impl()
{
    pCheckButton = new SvLBoxButtonData( this );
    EnableCheckButton( pCheckButton );
}

void SvxCheckListBox::InsertEntry( const OUString& rStr, sal_uLong nPos,
                                   void* pUserData,
                                   SvLBoxButtonKind eButtonKind )
{
    SvTreeListBox::InsertEntry( rStr, nullptr, false, nPos, pUserData,
                                eButtonKind );
}



void SvxCheckListBox::RemoveEntry( sal_uLong nPos )
{
    if ( nPos < GetEntryCount() )
        SvTreeListBox::GetModel()->Remove( GetEntry( nPos ) );
}



void SvxCheckListBox::SelectEntryPos( sal_uLong nPos, bool bSelect )
{
    if ( nPos < GetEntryCount() )
        Select( GetEntry( nPos ), bSelect );
}



sal_uLong SvxCheckListBox::GetSelectEntryPos() const
{
    SvTreeListEntry* pEntry = GetCurEntry();

    if ( pEntry )
        return GetModel()->GetAbsPos( pEntry );
    return TREELIST_ENTRY_NOTFOUND;
}



OUString SvxCheckListBox::GetText( sal_uLong nPos ) const
{
    SvTreeListEntry* pEntry = GetEntry( nPos );

    if ( pEntry )
        return GetEntryText( pEntry );
    return OUString();
}



sal_uLong SvxCheckListBox::GetCheckedEntryCount() const
{
    sal_uLong nCheckCount = 0;
    sal_uLong nCount = GetEntryCount();

    for ( sal_uLong i = 0; i < nCount; ++i )
    {
        if ( IsChecked( i ) )
            nCheckCount++;
    }
    return nCheckCount;
}



void SvxCheckListBox::CheckEntryPos( sal_uLong nPos, bool bCheck )
{
    if ( nPos < GetEntryCount() )
        SetCheckButtonState(
            GetEntry( nPos ), bCheck ? SvButtonState( SV_BUTTON_CHECKED ) :
                                       SvButtonState( SV_BUTTON_UNCHECKED ) );
}



bool SvxCheckListBox::IsChecked( sal_uLong nPos ) const
{
    if ( nPos < GetEntryCount() )
        return (GetCheckButtonState( GetEntry( nPos ) ) == SV_BUTTON_CHECKED);
    else
        return false;
}



void* SvxCheckListBox::SetEntryData ( sal_uLong nPos, void* pNewData )
{
    void* pOld = nullptr;

    if ( nPos < GetEntryCount() )
    {
        pOld = GetEntry( nPos )->GetUserData();
        GetEntry( nPos )->SetUserData( pNewData );
    }
    return pOld;
}



void* SvxCheckListBox::GetEntryData( sal_uLong nPos ) const
{
    if ( nPos < GetEntryCount() )
        return GetEntry( nPos )->GetUserData();
    else
        return nullptr;
}



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



void SvxCheckListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsLeft() )
    {
        const Point aPnt = rMEvt.GetPosPixel();
        SvTreeListEntry* pEntry = GetEntry( aPnt );

        if ( pEntry )
        {
            bool bCheck = ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED );
            SvLBoxItem* pItem = GetItem( pEntry, aPnt.X() );

            if (pItem && pItem->GetType() == SV_ITEM_ID_LBOXBUTTON)
            {
                SvTreeListBox::MouseButtonDown( rMEvt );
                Select( pEntry );
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



void SvxCheckListBox::KeyInput( const KeyEvent& rKEvt )
{
    const vcl::KeyCode& rKey = rKEvt.GetKeyCode();

    if ( rKey.GetCode() == KEY_RETURN || rKey.GetCode() == KEY_SPACE )
    {
        SvTreeListEntry* pEntry = GetCurEntry();

        if ( pEntry )
        {
            bool bCheck = ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED );
            ToggleCheckButton( pEntry );
            if ( bCheck != ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED ) )
                CheckButtonHdl();
        }
    }
    else if ( GetEntryCount() )
        SvTreeListBox::KeyInput( rKEvt );
}



SvTreeListEntry* SvxCheckListBox::InsertEntry( const OUString& rText, SvTreeListEntry* pParent, bool bChildrenOnDemand, sal_uIntPtr nPos, void* pUserData, SvLBoxButtonKind eButtonKind )
{
    return SvTreeListBox::InsertEntry( rText, pParent, bChildrenOnDemand, nPos, pUserData, eButtonKind );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
