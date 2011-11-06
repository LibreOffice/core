/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
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
                                  const Image& rNormalStaticImage,
                                  const Image& /*TODO#i72485# rHighContrastStaticImage*/ ) :

    SvTreeListBox( pParent, rResId )

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
    SvLBoxEntry* pEntry = GetCurEntry();

    if ( pEntry )
        return (sal_uInt16)GetModel()->GetAbsPos( pEntry );
    return LISTBOX_ENTRY_NOTFOUND;
}

// -----------------------------------------------------------------------

String SvxCheckListBox::GetText( sal_uInt16 nPos ) const
{
    SvLBoxEntry* pEntry = GetEntry( nPos );

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
        const Point aPnt = rMEvt.GetPosPixel();
        SvLBoxEntry* pEntry = GetEntry( aPnt );

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
                const SvLBoxEntry* pNewEntry = GetEntry( aPnt );
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
        SvLBoxEntry* pEntry = GetCurEntry();

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

SvLBoxEntry* SvxCheckListBox::InsertEntry( const XubString& rText, SvLBoxEntry* pParent, sal_Bool bChildsOnDemand, sal_uIntPtr nPos, void* pUserData, SvLBoxButtonKind eButtonKind )
{
    return SvTreeListBox::InsertEntry( rText, pParent, bChildsOnDemand, nPos, pUserData, eButtonKind );
}

