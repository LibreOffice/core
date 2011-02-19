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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/debug.hxx>

#include <tools/rc.h>
#include <vcl/svdata.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/toolbox.h>
#include <vcl/mnemonic.hxx>
#include <vcl/menu.hxx>
#include <vcl/brdwin.hxx>

#include <vcl/unohelp.hxx>
#include <unotools/confignode.hxx>

#include <vcl/ImageListProvider.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace vcl;
using namespace rtl;

// =======================================================================

#define TB_SEP_SIZE             8

// -----------------------------------------------------------------------

ImplToolBoxPrivateData::ImplToolBoxPrivateData() :
        m_pLayoutData( NULL ),
        mpImageListProvider( NULL ),
        meImageListType( vcl::IMAGELISTTYPE_UNKNOWN )
{
    meButtonSize = TOOLBOX_BUTTONSIZE_DONTCARE;
    mpMenu = new PopupMenu();
    mnEventId = 0;

    maMenuType = TOOLBOX_MENUTYPE_NONE;
    maMenubuttonItem.maItemSize = Size( TB_MENUBUTTON_SIZE+TB_MENUBUTTON_OFFSET, TB_MENUBUTTON_SIZE+TB_MENUBUTTON_OFFSET );
    maMenubuttonItem.meState = STATE_NOCHECK;
    mnMenuButtonWidth = TB_MENUBUTTON_SIZE;


    mbIsLocked = FALSE;
    mbNativeButtons = FALSE;
    mbIsPaintLocked = FALSE;
    mbAssumeDocked = FALSE;
    mbAssumePopupMode = FALSE;
    mbAssumeFloating = FALSE;
    mbKeyInputDisabled = FALSE;
    mbMenubuttonSelected = FALSE;
    mbPageScroll = FALSE;
    mbWillUsePopupMode = FALSE;
    mbDropDownByKeyboard = FALSE;
}

ImplToolBoxPrivateData::~ImplToolBoxPrivateData()
{
    if( m_pLayoutData )
        delete m_pLayoutData;
    delete mpMenu;
}

// -----------------------------------------------------------------------
ImplToolItem::ImplToolItem()
{
    mnId            = 0;
    mpWindow        = NULL;
    mpUserData      = NULL;
    mnHelpId        = 0;
    meType          = TOOLBOXITEM_BUTTON;
    mnBits          = 0;
    meState         = STATE_NOCHECK;
    mbEnabled       = TRUE;
    mbVisible       = TRUE;
    mbEmptyBtn      = TRUE;
    mbShowWindow    = FALSE;
    mbBreak         = FALSE;
    mnSepSize       = TB_SEP_SIZE;
    mnDropDownArrowWidth = TB_DROPDOWNARROWWIDTH;
    mnImageAngle    = 0;
    mbMirrorMode    = FALSE;
    mbVisibleText   = FALSE;
}

// -----------------------------------------------------------------------

ImplToolItem::ImplToolItem( USHORT nItemId, const Image& rImage,
                            ToolBoxItemBits nItemBits ) :
    maImage( rImage )
{
    mnId            = nItemId;
    mpWindow        = NULL;
    mpUserData      = NULL;
    mnHelpId        = 0;
    meType          = TOOLBOXITEM_BUTTON;
    mnBits          = nItemBits;
    meState         = STATE_NOCHECK;
    mbEnabled       = TRUE;
    mbVisible       = TRUE;
    mbEmptyBtn      = FALSE;
    mbShowWindow    = FALSE;
    mbBreak         = FALSE;
    mnSepSize       = TB_SEP_SIZE;
    mnDropDownArrowWidth = TB_DROPDOWNARROWWIDTH;
    mnImageAngle    = 0;
    mbMirrorMode    = false;
    mbVisibleText   = false;
}

// -----------------------------------------------------------------------

ImplToolItem::ImplToolItem( USHORT nItemId, const XubString& rText,
                            ToolBoxItemBits nItemBits ) :
    maText( rText )
{
    mnId            = nItemId;
    mpWindow        = NULL;
    mpUserData      = NULL;
    mnHelpId        = 0;
    meType          = TOOLBOXITEM_BUTTON;
    mnBits          = nItemBits;
    meState         = STATE_NOCHECK;
    mbEnabled       = TRUE;
    mbVisible       = TRUE;
    mbEmptyBtn      = FALSE;
    mbShowWindow    = FALSE;
    mbBreak         = FALSE;
    mnSepSize       = TB_SEP_SIZE;
    mnDropDownArrowWidth = TB_DROPDOWNARROWWIDTH;
    mnImageAngle    = 0;
    mbMirrorMode    = false;
    mbVisibleText   = false;
}

// -----------------------------------------------------------------------

ImplToolItem::ImplToolItem( USHORT nItemId, const Image& rImage,
                            const XubString& rText, ToolBoxItemBits nItemBits ) :
    maImage( rImage ),
    maText( rText )
{
    mnId            = nItemId;
    mpWindow        = NULL;
    mpUserData      = NULL;
    mnHelpId        = 0;
    meType          = TOOLBOXITEM_BUTTON;
    mnBits          = nItemBits;
    meState         = STATE_NOCHECK;
    mbEnabled       = TRUE;
    mbVisible       = TRUE;
    mbEmptyBtn      = FALSE;
    mbShowWindow    = FALSE;
    mbBreak         = FALSE;
    mnSepSize       = TB_SEP_SIZE;
    mnDropDownArrowWidth = TB_DROPDOWNARROWWIDTH;
    mnImageAngle    = 0;
    mbMirrorMode    = false;
    mbVisibleText   = false;
}

// -----------------------------------------------------------------------

ImplToolItem::ImplToolItem( const ImplToolItem& rItem ) :
        mpWindow                ( rItem.mpWindow ),
        mpUserData              ( rItem.mpUserData ),
        maImage                 ( rItem.maImage ),
        maHighImage             ( rItem.maHighImage ),
        mnImageAngle            ( rItem.mnImageAngle ),
        mbMirrorMode            ( rItem.mbMirrorMode ),
        maText                  ( rItem.maText ),
        maQuickHelpText         ( rItem.maQuickHelpText ),
        maHelpText              ( rItem.maHelpText ),
        maCommandStr            ( rItem.maCommandStr ),
        mnHelpId                ( rItem.mnHelpId ),
        maRect                  ( rItem.maRect ),
        maCalcRect              ( rItem.maCalcRect ),
        maItemSize              ( rItem.maItemSize ),
        mnSepSize               ( rItem.mnSepSize ),
        mnDropDownArrowWidth    ( rItem.mnDropDownArrowWidth ),
        meType                  ( rItem.meType ),
        mnBits                  ( rItem.mnBits ),
        meState                 ( rItem.meState ),
        mnId                    ( rItem.mnId ),
        mbEnabled               ( rItem.mbEnabled ),
        mbVisible               ( rItem.mbVisible ),
        mbEmptyBtn              ( rItem.mbEmptyBtn ),
        mbShowWindow            ( rItem.mbShowWindow ),
        mbBreak                 ( rItem.mbBreak ),
        mbVisibleText           ( rItem.mbVisibleText )
{
}

// -----------------------------------------------------------------------

ImplToolItem::~ImplToolItem()
{
}

// -----------------------------------------------------------------------

ImplToolItem& ImplToolItem::operator=( const ImplToolItem& rItem )
{
    mpWindow                = rItem.mpWindow;
    mpUserData              = rItem.mpUserData;
    maImage                 = rItem.maImage;
    maHighImage             = rItem.maHighImage;
    mnImageAngle            = rItem.mnImageAngle;
    mbMirrorMode            = rItem.mbMirrorMode;
    maText                  = rItem.maText;
    maQuickHelpText         = rItem.maQuickHelpText;
    maHelpText              = rItem.maHelpText;
    maCommandStr            = rItem.maCommandStr;
    mnHelpId                = rItem.mnHelpId;
    maRect                  = rItem.maRect;
    maCalcRect              = rItem.maCalcRect;
    mnSepSize               = rItem.mnSepSize;
    mnDropDownArrowWidth    = rItem.mnDropDownArrowWidth;
    maItemSize              = rItem.maItemSize;
    mbVisibleText           = rItem.mbVisibleText;
    meType                  = rItem.meType;
    mnBits                  = rItem.mnBits;
    meState                 = rItem.meState;
    mnId                    = rItem.mnId;
    mbEnabled               = rItem.mbEnabled;
    mbVisible               = rItem.mbVisible;
    mbEmptyBtn              = rItem.mbEmptyBtn;
    mbShowWindow            = rItem.mbShowWindow;
    mbBreak                 = rItem.mbBreak;
    return *this;
}

// -----------------------------------------------------------------------

Size ImplToolItem::GetSize( BOOL bHorz, BOOL bCheckMaxWidth, long maxWidth, const Size& rDefaultSize )
{
    Size aSize( rDefaultSize ); // the size of 'standard' toolbox items
                                // non-standard items are eg windows or buttons with text

    if ( (meType == TOOLBOXITEM_BUTTON) || (meType == TOOLBOXITEM_SPACE) )
    {
        aSize = maItemSize;

        if ( mpWindow && bHorz )
        {
            // get size of item window and check if it fits
            // no windows in vertical toolbars (the default is mbShowWindow=FALSE)
            Size aWinSize = mpWindow->GetSizePixel();
            if ( !bCheckMaxWidth || (aWinSize.Width() <= maxWidth) )
            {
                aSize.Width()   = aWinSize.Width();
                aSize.Height()  = aWinSize.Height();
                mbShowWindow = TRUE;
            }
            else
            {
                if ( mbEmptyBtn )
                {
                    aSize.Width()   = 0;
                    aSize.Height()  = 0;
                }
            }
        }
    }
    else if ( meType == TOOLBOXITEM_SEPARATOR )
    {
        if ( bHorz )
        {
            aSize.Width()   = mnSepSize;
            aSize.Height()  = rDefaultSize.Height();
        }
        else
        {
            aSize.Width()   = rDefaultSize.Width();
            aSize.Height()  = mnSepSize;
        }
    }
    else if ( meType == TOOLBOXITEM_BREAK )
    {
        aSize.Width()   = 0;
        aSize.Height()  = 0;
    }

    return aSize;
}

// -----------------------------------------------------------------------

void ImplToolItem::DetermineButtonDrawStyle( ButtonType eButtonType, BOOL& rbImage, BOOL& rbText ) const
{
    if ( meType != TOOLBOXITEM_BUTTON )
    {
        // no button -> draw nothing
        rbImage = rbText = FALSE;
        return;
    }

    BOOL bHasImage;
    BOOL bHasText;

    // check for image and/or text
    if ( !(maImage) )
        bHasImage = FALSE;
    else
        bHasImage = TRUE;
    if ( !maText.Len() )
        bHasText = FALSE;
    else
        bHasText = TRUE;

    // prefer images if symbolonly buttons are drawn
    // prefer texts if textonly buttons are dreawn

    if ( eButtonType == BUTTON_SYMBOL )         // drawing icons only
    {
        if( bHasImage || !bHasText )
        {
            rbImage = TRUE;
            rbText  = FALSE;
        }
        else
        {
            rbImage = FALSE;
            rbText  = TRUE;
        }
    }
    else if ( eButtonType == BUTTON_TEXT )      // drawing text only
    {
        if( bHasText || !bHasImage )
        {
            rbImage = FALSE;
            rbText  = TRUE;
        }
        else
        {
            rbImage = TRUE;
            rbText  = FALSE;
        }
    }
    else                                        // drawing icons and text both
    {
        rbImage = TRUE;
        rbText  = TRUE;
    }
}

// -----------------------------------------------------------------------

Rectangle ImplToolItem::GetDropDownRect( BOOL bHorz ) const
{
    Rectangle aRect;
    if( (mnBits & TIB_DROPDOWN) && !maRect.IsEmpty() )
    {
        aRect = maRect;
        if( mbVisibleText && !bHorz )
            // item will be rotated -> place dropdown to the bottom
            aRect.Top() = aRect.Bottom() - mnDropDownArrowWidth;
        else
            // place dropdown to the right
            aRect.Left() = aRect.Right() - mnDropDownArrowWidth;
    }
    return aRect;
}

// -----------------------------------------------------------------------

BOOL ImplToolItem::IsClipped() const
{
    return ( meType == TOOLBOXITEM_BUTTON && mbVisible && maRect.IsEmpty() );
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

const XubString& ToolBox::ImplConvertMenuString( const XubString& rStr )
{
    maCvtStr = rStr;
    if ( mbMenuStrings )
        maCvtStr.EraseTrailingChars( '.' );
    maCvtStr = MnemonicGenerator::EraseAllMnemonicChars( maCvtStr );
    return maCvtStr;
}

// -----------------------------------------------------------------------

void ToolBox::ImplInvalidate( BOOL bNewCalc, BOOL bFullPaint )
{
    ImplUpdateInputEnable();

    if ( bNewCalc )
        mbCalc = TRUE;

    if ( bFullPaint )
    {
        mbFormat = TRUE;

        // Muss ueberhaupt eine neue Ausgabe erfolgen
        if ( IsReallyVisible() && IsUpdateMode() )
        {
            Invalidate( Rectangle( mnLeftBorder, mnTopBorder,
                                   mnDX-mnRightBorder-1, mnDY-mnBottomBorder-1 ) );
            maTimer.Stop();
        }
    }
    else
    {
        if ( !mbFormat )
        {
            mbFormat = TRUE;

            // Muss ueberhaupt eine neue Ausgabe erfolgen
            if ( IsReallyVisible() && IsUpdateMode() )
                maTimer.Start();
        }
    }

    // request new layout by layoutmanager
    ImplCallEventListeners( VCLEVENT_TOOLBOX_FORMATCHANGED );
}

// -----------------------------------------------------------------------

void ToolBox::ImplUpdateItem( USHORT nIndex )
{
    // Muss ueberhaupt eine neue Ausgabe erfolgen
    if ( IsReallyVisible() && IsUpdateMode() )
    {
        if ( nIndex == 0xFFFF )
        {
            // #i52217# no immediate draw as this might lead to paint problems
            Invalidate( Rectangle( mnLeftBorder, mnTopBorder,
                                    mnDX-mnRightBorder-1, mnDY-mnBottomBorder-1 ) );
        }
        else
        {
            if ( !mbFormat )
            {
                // #i52217# no immediate draw as this might lead to paint problems
                Invalidate( mpData->m_aItems[nIndex].maRect );
            }
            else
                maPaintRect.Union( mpData->m_aItems[nIndex].maRect );
        }
    }
}

// -----------------------------------------------------------------------

void ToolBox::Click()
{
    ImplCallEventListeners( VCLEVENT_TOOLBOX_CLICK );
    maClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void ToolBox::DoubleClick()
{
    ImplCallEventListeners( VCLEVENT_TOOLBOX_DOUBLECLICK );
    maDoubleClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void ToolBox::Activate()
{
    mnActivateCount++;
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ACTIVATE );
    maActivateHdl.Call( this );
}

// -----------------------------------------------------------------------

void ToolBox::Deactivate()
{
    mnActivateCount--;
    ImplCallEventListeners( VCLEVENT_TOOLBOX_DEACTIVATE );
    maDeactivateHdl.Call( this );
}

// -----------------------------------------------------------------------

void ToolBox::Highlight()
{
    ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHT );
    maHighlightHdl.Call( this );
}

// -----------------------------------------------------------------------

void ToolBox::Select()
{
    ImplDelData aDelData;
    ImplAddDel( &aDelData );

    ImplCallEventListeners( VCLEVENT_TOOLBOX_SELECT );
    maSelectHdl.Call( this );

    if ( aDelData.IsDelete() )
        return;
    ImplRemoveDel( &aDelData );

    // TODO: GetFloatingWindow in DockingWindow is currently inline, change it to check dockingwrapper
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper && pWrapper->GetFloatingWindow() && pWrapper->GetFloatingWindow()->IsInPopupMode() )
        pWrapper->GetFloatingWindow()->EndPopupMode();
}

// -----------------------------------------------------------------------

void ToolBox::NextToolBox()
{
    maNextToolBoxHdl.Call( this );
}

// -----------------------------------------------------------------------

void ToolBox::Customize( const ToolBoxCustomizeEvent& )
{
}

// -----------------------------------------------------------------------

void ToolBox::UserDraw( const UserDrawEvent& )
{
}

// -----------------------------------------------------------------------

void ToolBox::InsertItem( const ResId& rResId, USHORT nPos )
{
    ULONG                   nObjMask;
    BOOL                    bImage = FALSE;     // Wurde Image gesetzt

    // Item anlegen
    ImplToolItem aItem;

    GetRes( rResId.SetRT( RSC_TOOLBOXITEM ) );
    nObjMask            = ReadLongRes();

    if ( nObjMask & RSC_TOOLBOXITEM_ID )
        aItem.mnId = sal::static_int_cast<USHORT>(ReadLongRes());
    else
        aItem.mnId = 1;

    if ( nObjMask & RSC_TOOLBOXITEM_TYPE )
        aItem.meType = (ToolBoxItemType)ReadLongRes();

    if ( nObjMask & RSC_TOOLBOXITEM_STATUS )
        aItem.mnBits = (ToolBoxItemBits)ReadLongRes();

    if( nObjMask & RSC_TOOLBOXITEM_HELPID )
        aItem.mnHelpId = ReadLongRes();

    if ( nObjMask & RSC_TOOLBOXITEM_TEXT )
    {
        aItem.maText = ReadStringRes();
        aItem.maText = ImplConvertMenuString( aItem.maText );
    }
    if ( nObjMask & RSC_TOOLBOXITEM_HELPTEXT )
        aItem.maHelpText = ReadStringRes();

    if ( nObjMask & RSC_TOOLBOXITEM_BITMAP )
    {
        Bitmap aBmp = Bitmap( ResId( (RSHEADER_TYPE*)GetClassRes(), *rResId.GetResMgr() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
        aItem.maImage = Image( aBmp, IMAGE_STDBTN_COLOR );
        bImage = TRUE;
    }
    if ( nObjMask & RSC_TOOLBOXITEM_IMAGE )
    {
        aItem.maImage = Image( ResId( (RSHEADER_TYPE*)GetClassRes(), *rResId.GetResMgr() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
        bImage = TRUE;
    }
    if ( nObjMask & RSC_TOOLBOXITEM_DISABLE )
        aItem.mbEnabled = !(BOOL)ReadShortRes();

    if ( nObjMask & RSC_TOOLBOXITEM_STATE )
        aItem.meState   = (TriState)ReadLongRes();

    if ( nObjMask & RSC_TOOLBOXITEM_HIDE )
        aItem.mbVisible = !((BOOL)ReadShortRes());

    if ( nObjMask & RSC_TOOLBOXITEM_COMMAND )
        aItem.maCommandStr = ReadStringRes();

    // Wenn kein Image geladen wurde, versuchen wir das Image aus der
    // Image-Liste zu holen
    if ( !bImage && aItem.mnId )
        aItem.maImage = maImageList.GetImage( aItem.mnId );

    // Wenn es sich um ein ButtonItem handelt, die ID ueberpruefen
    BOOL bNewCalc;
    if ( aItem.meType != TOOLBOXITEM_BUTTON )
    {
        bNewCalc = FALSE;
        aItem.mnId = 0;
    }
    else
    {
        bNewCalc = TRUE;

        DBG_ASSERT( aItem.mnId, "ToolBox::InsertItem(): ItemId == 0" );
        DBG_ASSERT( GetItemPos( aItem.mnId ) == TOOLBOX_ITEM_NOTFOUND,
                    "ToolBox::InsertItem(): ItemId already exists" );
    }

    // Item anlegen und in die Liste einfuegen
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    // ToolBox neu brechnen und neu ausgeben
    ImplInvalidate( bNewCalc );

    // Notify
    USHORT nNewPos = sal::static_int_cast<USHORT>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos ) );
}

// -----------------------------------------------------------------------

void ToolBox::InsertItem( USHORT nItemId, const Image& rImage,
                          ToolBoxItemBits nBits, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertItem(): ItemId already exists" );

    // Item anlegen und in die Liste einfuegen
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), ImplToolItem( nItemId, rImage, nBits ) );
    mpData->ImplClearLayoutData();

    ImplInvalidate( TRUE );

    // Notify
    USHORT nNewPos = sal::static_int_cast<USHORT>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >(nNewPos ) );
}

// -----------------------------------------------------------------------

void ToolBox::InsertItem( USHORT nItemId, const Image& rImage,
                          const XubString& rText,
                          ToolBoxItemBits nBits, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertItem(): ItemId already exists" );

    // Item anlegen und in die Liste einfuegen
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), ImplToolItem( nItemId, rImage, ImplConvertMenuString( rText ), nBits ) );
    mpData->ImplClearLayoutData();

    ImplInvalidate( TRUE );

    // Notify
    USHORT nNewPos = sal::static_int_cast<USHORT>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos ) );
}

// -----------------------------------------------------------------------

void ToolBox::InsertItem( USHORT nItemId, const XubString& rText,
                          ToolBoxItemBits nBits, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertItem(): ItemId already exists" );

    // Item anlegen und in die Liste einfuegen
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), ImplToolItem( nItemId, ImplConvertMenuString( rText ), nBits ) );
    mpData->ImplClearLayoutData();

    ImplInvalidate( TRUE );

    // Notify
    USHORT nNewPos = sal::static_int_cast<USHORT>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos ) );
}

// -----------------------------------------------------------------------

void ToolBox::InsertWindow( USHORT nItemId, Window* pWindow,
                            ToolBoxItemBits nBits, USHORT nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertWindow(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertWindow(): ItemId already exists" );

    // Item anlegen und in die Liste einfuegen
    ImplToolItem aItem;
    aItem.mnId       = nItemId;
    aItem.meType     = TOOLBOXITEM_BUTTON;
    aItem.mnBits     = nBits;
    aItem.mpWindow   = pWindow;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    if ( pWindow )
        pWindow->Hide();

    ImplInvalidate( TRUE );

    // Notify
    USHORT nNewPos = sal::static_int_cast<USHORT>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos ) );
}

// -----------------------------------------------------------------------

void ToolBox::InsertSpace( USHORT nPos )
{
    // Item anlegen und in die Liste einfuegen
    ImplToolItem aItem;
    aItem.meType     = TOOLBOXITEM_SPACE;
    aItem.mbEnabled  = FALSE;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    ImplInvalidate( FALSE );

    // Notify
    USHORT nNewPos = sal::static_int_cast<USHORT>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos ) );
}

// -----------------------------------------------------------------------

void ToolBox::InsertSeparator( USHORT nPos, USHORT nPixSize )
{
    // Item anlegen und in die Liste einfuegen
    ImplToolItem aItem;
    aItem.meType     = TOOLBOXITEM_SEPARATOR;
    aItem.mbEnabled  = FALSE;
    if ( nPixSize )
        aItem.mnSepSize = nPixSize;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    ImplInvalidate( FALSE );

    // Notify
    USHORT nNewPos = sal::static_int_cast<USHORT>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos ) );
}

// -----------------------------------------------------------------------

void ToolBox::InsertBreak( USHORT nPos )
{
    // Item anlegen und in die Liste einfuegen
    ImplToolItem aItem;
    aItem.meType     = TOOLBOXITEM_BREAK;
    aItem.mbEnabled  = FALSE;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    ImplInvalidate( FALSE );

    // Notify
    USHORT nNewPos = sal::static_int_cast<USHORT>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos ) );
}

// -----------------------------------------------------------------------

void ToolBox::RemoveItem( USHORT nPos )
{
    if( nPos < mpData->m_aItems.size() )
    {
        BOOL bMustCalc;
        if ( mpData->m_aItems[nPos].meType == TOOLBOXITEM_BUTTON )
            bMustCalc = TRUE;
        else
            bMustCalc = FALSE;

        if ( mpData->m_aItems[nPos].mpWindow )
            mpData->m_aItems[nPos].mpWindow->Hide();

        // PaintRect um das removete Item erweitern
        maPaintRect.Union( mpData->m_aItems[nPos].maRect );

        // Absichern gegen das Loeschen im Select-Handler
        if ( mpData->m_aItems[nPos].mnId == mnCurItemId )
            mnCurItemId = 0;
        if ( mpData->m_aItems[nPos].mnId == mnHighItemId )
            mnHighItemId = 0;

        ImplInvalidate( bMustCalc );

        mpData->m_aItems.erase( mpData->m_aItems.begin()+nPos );
        mpData->ImplClearLayoutData();

        // Notify
        ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMREMOVED, reinterpret_cast< void* >( nPos ) );
    }
}

// -----------------------------------------------------------------------

void ToolBox::MoveItem( USHORT nItemId, USHORT nNewPos )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos == nNewPos )
        return;

    if ( nPos < nNewPos )
        nNewPos--;

    // Existiert Item
    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        // ToolBox-Item in der Liste verschieben
        ImplToolItem aItem = mpData->m_aItems[nPos];
        mpData->m_aItems.erase( mpData->m_aItems.begin()+nPos );
        mpData->m_aItems.insert( (nNewPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nNewPos : mpData->m_aItems.end(), aItem );
        mpData->ImplClearLayoutData();

        // ToolBox neu ausgeben
        ImplInvalidate( FALSE );

        // Notify
        if( nPos < nNewPos )    // only send one event, all indices above this item are invalid anyway
            ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMREMOVED, reinterpret_cast< void* >( nPos ) );
        else
        {
            USHORT nNewPos2 = sal::static_int_cast<USHORT>(( nNewPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nNewPos);
            ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos2 ) );
        }
    }
}

// -----------------------------------------------------------------------

void ToolBox::CopyItem( const ToolBox& rToolBox, USHORT nItemId,
                        USHORT nNewPos )
{
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::CopyItem(): ItemId already exists" );

    USHORT nPos = rToolBox.GetItemPos( nItemId );

    // Existiert Item
    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        // ToolBox-Item in der Liste verschieben
        ImplToolItem aNewItem = rToolBox.mpData->m_aItems[nPos];
        // Bestimme Daten zuruecksetzen
        aNewItem.mpWindow      = NULL;
        aNewItem.mbShowWindow = FALSE;

        mpData->m_aItems.insert( (nNewPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nNewPos : mpData->m_aItems.end(), aNewItem );
        mpData->ImplClearLayoutData();
        // ToolBox neu ausgeben
        ImplInvalidate( FALSE );

        // Notify
        USHORT nNewPos2 = sal::static_int_cast<USHORT>(( nNewPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nNewPos);
        ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos2 ) );
    }
}

// -----------------------------------------------------------------------

void ToolBox::CopyItems( const ToolBox& rToolBox )
{
    mpData->ImplClearLayoutData();
    mpData->m_aItems = rToolBox.mpData->m_aItems;
    // Absichern gegen das Loeschen im Select-Handler
    mnCurItemId = 0;
    mnHighItemId = 0;

    for( std::vector< ImplToolItem >::iterator it = mpData->m_aItems.begin();
         it != mpData->m_aItems.end(); ++it )
    {
        it->mpWindow        = NULL;
        it->mbShowWindow    = FALSE;
    }

    ImplInvalidate( TRUE, TRUE );

    // Notify
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ALLITEMSCHANGED );
}

// -----------------------------------------------------------------------

void ToolBox::Clear()
{
    mpData->m_aItems.clear();
    mpData->ImplClearLayoutData();

    // Absichern gegen das Loeschen im Select-Handler
    mnCurItemId = 0;
    mnHighItemId = 0;

    ImplInvalidate( TRUE, TRUE );

    // Notify
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ALLITEMSCHANGED );
}

// -----------------------------------------------------------------------

void ToolBox::SetButtonType( ButtonType eNewType )
{
    if ( meButtonType != eNewType )
    {
        meButtonType = eNewType;

        // Hier besser alles neu ausgeben, da es ansonsten zu Problemen
        // mit den per CopyBits kopierten Bereichen geben kann
        ImplInvalidate( TRUE );
    }
}

// -----------------------------------------------------------------------

void ToolBox::SetToolboxButtonSize( ToolBoxButtonSize eSize )
{
    if( mpData->meButtonSize != eSize )
    {
        mpData->meButtonSize = eSize;
        mbCalc = TRUE;
        mbFormat = TRUE;
    }
}

ToolBoxButtonSize ToolBox::GetToolboxButtonSize() const
{
    return mpData->meButtonSize;
}

// -----------------------------------------------------------------------

const Size& ToolBox::GetDefaultImageSize() const
{
    static Size aSmallButtonSize( TB_SMALLIMAGESIZE, TB_SMALLIMAGESIZE );

    static ULONG s_nSymbolsStyle = STYLE_SYMBOLS_DEFAULT;
    static Size aLargeButtonSize( TB_LARGEIMAGESIZE, TB_LARGEIMAGESIZE );

    ULONG nSymbolsStyle = Application::GetSettings().GetStyleSettings().GetCurrentSymbolsStyle();
    if ( s_nSymbolsStyle != nSymbolsStyle )
    {
        s_nSymbolsStyle = nSymbolsStyle;
        switch ( nSymbolsStyle )
        {
            case STYLE_SYMBOLS_INDUSTRIAL:
                aLargeButtonSize = Size( TB_LARGEIMAGESIZE_INDUSTRIAL, TB_LARGEIMAGESIZE_INDUSTRIAL );
                break;
            case STYLE_SYMBOLS_CRYSTAL:
                aLargeButtonSize = Size( TB_LARGEIMAGESIZE_CRYSTAL, TB_LARGEIMAGESIZE_CRYSTAL );
                break;
            case STYLE_SYMBOLS_OXYGEN:
                aLargeButtonSize = Size( TB_LARGEIMAGESIZE_OXYGEN, TB_LARGEIMAGESIZE_OXYGEN );
                break;
            default:
                aLargeButtonSize = Size( TB_LARGEIMAGESIZE, TB_LARGEIMAGESIZE );
        }
    }

    return GetToolboxButtonSize() == TOOLBOX_BUTTONSIZE_LARGE ? aLargeButtonSize : aSmallButtonSize;
}

// -----------------------------------------------------------------------

void ToolBox::SetAlign( WindowAlign eNewAlign )
{
    if ( meAlign != eNewAlign )
    {
        meAlign = eNewAlign;

        if ( !ImplIsFloatingMode() )
        {
            // Setzen, ob Items horizontal oder vertikal angeordnet werden sollen
            if ( (eNewAlign == WINDOWALIGN_LEFT) || (eNewAlign == WINDOWALIGN_RIGHT) )
                mbHorz = FALSE;
            else
                mbHorz = TRUE;

            // Hier alles neu ausgeben, da sich Border auch aendert
            mbCalc = TRUE;
            mbFormat = TRUE;
            if ( IsReallyVisible() && IsUpdateMode() )
                Invalidate();
        }
    }
}

// -----------------------------------------------------------------------

void ToolBox::SetLineCount( USHORT nNewLines )
{
    if ( !nNewLines )
        nNewLines = 1;

    if ( mnLines != nNewLines )
    {
        mnLines = nNewLines;

        // Hier besser alles neu ausgeben, da es ansonsten zu Problemen
        // mit den per CopyBits kopierten Bereichen geben kann
        ImplInvalidate( FALSE );
    }
}

// -----------------------------------------------------------------------

void ToolBox::SetPageScroll( BOOL b )
{
    mpData->mbPageScroll = b;
}

BOOL ToolBox::GetPageScroll()
{
    return mpData->mbPageScroll;
}

// -----------------------------------------------------------------------

void ToolBox::SetNextToolBox( const XubString& rStr )
{
    BOOL bCalcNew = (!maNextToolBoxStr.Len() != !rStr.Len());
    maNextToolBoxStr = rStr;
    if ( bCalcNew )
        ImplInvalidate( TRUE, FALSE );
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetItemCount() const
{
    return (USHORT)mpData->m_aItems.size();
}

// -----------------------------------------------------------------------

ToolBoxItemType ToolBox::GetItemType( USHORT nPos ) const
{
    return (nPos < mpData->m_aItems.size()) ? mpData->m_aItems[nPos].meType : TOOLBOXITEM_DONTKNOW;
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetItemPos( USHORT nItemId ) const
{
    int nCount = mpData->m_aItems.size();
    for( int nPos = 0; nPos < nCount; nPos++ )
        if( mpData->m_aItems[nPos].mnId == nItemId )
            return (USHORT)nPos;

    return TOOLBOX_ITEM_NOTFOUND;
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetItemPos( const Point& rPos ) const
{
    // search the item position on the given point
    USHORT nRet = TOOLBOX_ITEM_NOTFOUND;
    USHORT nPos = 0;
    std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
    while( it != mpData->m_aItems.end() )
    {
        if ( it->maRect.IsInside( rPos ) )
        {
            // item found -> save position and break
            nRet = nPos;
            break;
        }

        ++it;
        ++nPos;
    }

    return nRet;
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetItemId( USHORT nPos ) const
{
    return (nPos < mpData->m_aItems.size()) ? mpData->m_aItems[nPos].mnId : 0;
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetItemId( const Point& rPos ) const
{
    // Item suchen, das geklickt wurde
    std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
    while( it != mpData->m_aItems.end() )
    {
        // Ist es dieses Item
        if ( it->maRect.IsInside( rPos ) )
        {
            if ( it->meType == TOOLBOXITEM_BUTTON )
                return it->mnId;
            else
                return 0;
        }

        ++it;
    }

    return 0;
}

// -----------------------------------------------------------------------

Point ToolBox::ImplGetPopupPosition( const Rectangle& rRect, const Size& rSize ) const
{
    Point aPos;
    if( !rRect.IsEmpty() )
    {
        Rectangle aScreen = GetDesktopRectPixel();

        // the popup should be positioned so that it will not cover
        // the item rect and that it fits the desktop
        // the preferred direction is always towards the center of
        // the application window

        Point devPos;           // the position in device coordinates for screen comparison
        switch( meAlign )
        {
            case WINDOWALIGN_TOP:
                aPos = rRect.BottomLeft();
                aPos.Y()++;
                devPos = OutputToAbsoluteScreenPixel( aPos );
                if( devPos.Y() + rSize.Height() >= aScreen.Bottom() )
                    aPos.Y() = rRect.Top() - rSize.Height();
                break;
            case WINDOWALIGN_BOTTOM:
                aPos = rRect.TopLeft();
                aPos.Y()--;
                devPos = OutputToAbsoluteScreenPixel( aPos );
                if( devPos.Y() - rSize.Height() > aScreen.Top() )
                    aPos.Y() -= rSize.Height();
                else
                    aPos.Y() = rRect.Bottom();
                break;
            case WINDOWALIGN_LEFT:
                aPos = rRect.TopRight();
                aPos.X()++;
                devPos = OutputToAbsoluteScreenPixel( aPos );
                if( devPos.X() + rSize.Width() >= aScreen.Right() )
                    aPos.X() = rRect.Left() - rSize.Width();
                break;
            case WINDOWALIGN_RIGHT:
                aPos = rRect.TopLeft();
                aPos.X()--;
                devPos = OutputToAbsoluteScreenPixel( aPos );
                if( devPos.X() - rSize.Width() > aScreen.Left() )
                    aPos.X() -= rSize.Width();
                else
                    aPos.X() = rRect.Right();
                break;
            default:
                break;
        };
    }
    return aPos;
}


Point ToolBox::GetItemPopupPosition( USHORT nItemId, const Size& rSize ) const
{
    return ImplGetPopupPosition( GetItemRect( nItemId ), rSize );
}

// -----------------------------------------------------------------------

Rectangle ToolBox::GetItemRect( USHORT nItemId ) const
{
    if ( mbCalc || mbFormat )
        ((ToolBox*)this)->ImplFormat();

    USHORT nPos = GetItemPos( nItemId );
    return GetItemPosRect( nPos );
}

// -----------------------------------------------------------------------

Rectangle ToolBox::GetItemPosRect( USHORT nPos ) const
{
    if ( mbCalc || mbFormat )
        ((ToolBox*)this)->ImplFormat();

    if ( nPos < mpData->m_aItems.size() )
        return mpData->m_aItems[nPos].maRect;
    else
        return Rectangle();
}

// -----------------------------------------------------------------------
Rectangle ToolBox::GetItemDropDownRect( USHORT nItemId ) const
{
    if ( mbCalc || mbFormat )
        ((ToolBox*)this)->ImplFormat();

    USHORT nPos = GetItemPos( nItemId );
    return GetItemPosDropDownRect( nPos );
}

// -----------------------------------------------------------------------

Rectangle ToolBox::GetItemPosDropDownRect( USHORT nPos ) const
{
    if ( mbCalc || mbFormat )
        ((ToolBox*)this)->ImplFormat();

    if ( nPos < mpData->m_aItems.size() )
        return mpData->m_aItems[nPos].GetDropDownRect( mbHorz );
    else
        return Rectangle();
}

// -----------------------------------------------------------------------

Rectangle ToolBox::GetMenubuttonRect() const
{
    return mpData->maMenubuttonItem.maRect;
}

BOOL ToolBox::ImplHasExternalMenubutton()
{
    // check if the borderwindow (i.e. the decoration) provides the menu button
    BOOL bRet = FALSE;
    if( ImplIsFloatingMode() )
    {
        // custom menu is placed in the decoration
        ImplBorderWindow *pBorderWin = dynamic_cast<ImplBorderWindow*>( GetWindow( WINDOW_BORDER ) );
        if( pBorderWin && !pBorderWin->GetMenuRect().IsEmpty() )
            bRet = TRUE;
    }
    return bRet;
}
// -----------------------------------------------------------------------

void ToolBox::SetItemBits( USHORT nItemId, ToolBoxItemBits nBits )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos < mpData->m_aItems.size() )
    {
        ToolBoxItemBits nOldBits = mpData->m_aItems[nPos].mnBits;
        mpData->m_aItems[nPos].mnBits = nBits;
        nBits &= TIB_LEFT | TIB_AUTOSIZE | TIB_DROPDOWN;
        nOldBits &= TIB_LEFT | TIB_AUTOSIZE | TIB_DROPDOWN;
        // trigger reformat when the item width has changed (dropdown arrow)
        BOOL bFormat = (nBits & TIB_DROPDOWN) != (nOldBits & TIB_DROPDOWN);
        if ( nBits != nOldBits )
            ImplInvalidate( TRUE, bFormat );
    }
}

// -----------------------------------------------------------------------

ToolBoxItemBits ToolBox::GetItemBits( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mnBits;
    else
        return 0;
}

// -----------------------------------------------------------------------

void ToolBox::SetItemData( USHORT nItemId, void* pNewData )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos < mpData->m_aItems.size() )
    {
        mpData->m_aItems[nPos].mpUserData = pNewData;
        ImplUpdateItem( nPos );
    }
}

// -----------------------------------------------------------------------

void* ToolBox::GetItemData( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mpUserData;
    else
        return NULL;
}

// -----------------------------------------------------------------------

void ToolBox::SetItemImage( USHORT nItemId, const Image& rImage )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        // Nur wenn alles berechnet ist, mehr Aufwand treiben
        if ( !mbCalc )
        {
            Size aOldSize = pItem->maImage.GetSizePixel();
            pItem->maImage = rImage;
            if ( aOldSize != pItem->maImage.GetSizePixel() )
                ImplInvalidate( TRUE );
            else
                ImplUpdateItem( nPos );
        }
        else
            pItem->maImage = rImage;
    }
}

// -----------------------------------------------------------------------

void ToolBox::SetImageList( const ImageList& rImageList )
{
    maImageList = rImageList;

    USHORT nCount = (USHORT)mpData->m_aItems.size();
    for( USHORT i = 0; i < nCount; i++ )
    {
        Image aImage;
        if ( mpData->m_aItems[i].mnId )
            aImage = maImageList.GetImage( mpData->m_aItems[i].mnId );
        if( !!aImage )
            SetItemImage( mpData->m_aItems[i].mnId, aImage );
    }
}

// -----------------------------------------------------------------------

static Image ImplRotImage( const Image& rImage, long nAngle10 )
{
    Image       aRet;
    BitmapEx    aRotBitmapEx( rImage.GetBitmapEx() );

    aRotBitmapEx.Rotate( nAngle10, Color( COL_WHITE ) );

    return Image( aRotBitmapEx );
}

void ToolBox::SetItemImageAngle( USHORT nItemId, long nAngle10 )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        Size aOldSize = pItem->maImage.GetSizePixel();

        long nDeltaAngle = (nAngle10 - pItem->mnImageAngle) % 3600;
        while( nDeltaAngle < 0 )
            nDeltaAngle += 3600;

        pItem->mnImageAngle = nAngle10;
        if( nDeltaAngle && !!pItem->maImage )
        {
            pItem->maImage = ImplRotImage( pItem->maImage, nDeltaAngle );
            if( !!pItem->maHighImage )
                pItem->maHighImage = ImplRotImage( pItem->maHighImage, nDeltaAngle );
        }

        if ( !mbCalc )
        {
            if ( aOldSize != pItem->maImage.GetSizePixel() )
                ImplInvalidate( TRUE );
            else
                ImplUpdateItem( nPos );
        }
    }
}

// -----------------------------------------------------------------------

static Image ImplMirrorImage( const Image& rImage )
{
    Image       aRet;
    BitmapEx    aMirrBitmapEx( rImage.GetBitmapEx() );

    aMirrBitmapEx.Mirror( BMP_MIRROR_HORZ );

    return Image( aMirrBitmapEx );
}

void ToolBox::SetItemImageMirrorMode( USHORT nItemId, BOOL bMirror )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];

        if( ( pItem->mbMirrorMode && ! bMirror ) ||
            ( ! pItem->mbMirrorMode && bMirror )
            )
        {
            pItem->mbMirrorMode = bMirror ? true : false;
            if( !!pItem->maImage )
            {
                pItem->maImage = ImplMirrorImage( pItem->maImage );
                if( !!pItem->maHighImage )
                    pItem->maHighImage = ImplMirrorImage( pItem->maHighImage );
            }

            if ( !mbCalc )
                ImplUpdateItem( nPos );
        }
    }
}

// -----------------------------------------------------------------------

Image ToolBox::GetItemImage( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maImage;
    else
        return Image();
}

// -----------------------------------------------------------------------

long ToolBox::GetItemImageAngle( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mnImageAngle;
    else
        return 0;
}

// -----------------------------------------------------------------------

BOOL ToolBox::GetItemImageMirrorMode( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mbMirrorMode;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void ToolBox::SetItemHighImage( USHORT nItemId, const Image& rImage )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );
    if ( pItem )
    {
        DBG_ASSERT( (pItem->maImage.GetSizePixel() == rImage.GetSizePixel()) ||
                    ((!rImage) == TRUE), "ToolBox::SetItemHighImage() - ImageSize != HighImageSize" );
        pItem->maHighImage = rImage;
    }
}

// -----------------------------------------------------------------------

Image ToolBox::GetItemHighImage( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maHighImage;
    else
        return Image();
}

// -----------------------------------------------------------------------

void ToolBox::SetItemText( USHORT nItemId, const XubString& rText )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        // Nur wenn alles berechnet ist, mehr Aufwand treiben
        if ( !mbCalc &&
             ((meButtonType != BUTTON_SYMBOL) || !pItem->maImage) )
        {
            long nOldWidth = GetCtrlTextWidth( pItem->maText );
            pItem->maText = ImplConvertMenuString( rText );
            mpData->ImplClearLayoutData();
            if ( nOldWidth != GetCtrlTextWidth( pItem->maText ) )
                ImplInvalidate( TRUE );
            else
                ImplUpdateItem( nPos );
        }
        else
            pItem->maText = ImplConvertMenuString( rText );

        // Notify button changed event to prepare accessibility bridge
        ImplCallEventListeners( VCLEVENT_TOOLBOX_BUTTONSTATECHANGED, reinterpret_cast< void* >( nPos ) );

        // Notify
        ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMTEXTCHANGED, reinterpret_cast< void* >( nPos ) );
    }
}

// -----------------------------------------------------------------------

const XubString& ToolBox::GetItemText( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maText;
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void ToolBox::SetItemWindow( USHORT nItemId, Window* pNewWindow )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        pItem->mpWindow = pNewWindow;
        if ( pNewWindow )
            pNewWindow->Hide();
        ImplInvalidate( TRUE );
        ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMWINDOWCHANGED, reinterpret_cast< void* >( nPos ) );
    }
}

// -----------------------------------------------------------------------

Window* ToolBox::GetItemWindow( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mpWindow;
    else
        return NULL;
}

// -----------------------------------------------------------------------

void ToolBox::StartSelection()
{
    if ( mbDrag )
        EndSelection();

    if ( !mbSelection )
    {
        mbSelection  = TRUE;
        mnCurPos     = TOOLBOX_ITEM_NOTFOUND;
        mnCurItemId  = 0;
        Activate();
    }
}

// -----------------------------------------------------------------------

void ToolBox::EndSelection()
{
    mbCommandDrag = FALSE;

    if ( mbDrag || mbSelection )
    {
        // Daten zuruecksetzen
        mbDrag = FALSE;
        mbSelection = FALSE;
        if ( mnCurPos != TOOLBOX_ITEM_NOTFOUND )
            ImplDrawItem( mnCurPos );
        EndTracking();
        ReleaseMouse();
        Deactivate();
    }

    mnCurPos        = TOOLBOX_ITEM_NOTFOUND;
    mnCurItemId     = 0;
    mnDownItemId    = 0;
    mnMouseClicks   = 0;
    mnMouseModifier = 0;
}

// -----------------------------------------------------------------------

void ToolBox::SetItemDown( USHORT nItemId, BOOL bDown, BOOL bRelease )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        if ( bDown )
        {
            if ( nPos != mnCurPos )
            {
                mnCurPos = nPos;
                ImplDrawItem( mnCurPos, TRUE );
                Flush();
            }
        }
        else
        {
            if ( nPos == mnCurPos )
            {
                ImplDrawItem( mnCurPos, FALSE );
                Flush();
                mnCurPos = TOOLBOX_ITEM_NOTFOUND;
            }
        }

        if ( bRelease )
        {
            if ( mbDrag || mbSelection )
            {
                mbDrag = FALSE;
                mbSelection = FALSE;
                EndTracking();
                ReleaseMouse();
                Deactivate();
            }

            mnCurItemId     = 0;
            mnDownItemId    = 0;
            mnMouseClicks   = 0;
            mnMouseModifier = 0;
        }
    }
}

// -----------------------------------------------------------------------

BOOL ToolBox::IsItemDown( USHORT nItemId ) const
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
        return (nPos == mnCurPos);
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void ToolBox::SetItemState( USHORT nItemId, TriState eState )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];

        // Hat sich der Status geaendert
        if ( pItem->meState != eState )
        {
            // Wenn RadioCheck, dann vorherigen unchecken
            if ( (eState == STATE_CHECK) && (pItem->mnBits & TIB_AUTOCHECK) &&
                 (pItem->mnBits & TIB_RADIOCHECK) )
            {
                ImplToolItem*    pGroupItem;
                USHORT          nGroupPos;
                USHORT          nItemCount = GetItemCount();

                nGroupPos = nPos;
                while ( nGroupPos )
                {
                    pGroupItem = &mpData->m_aItems[nGroupPos-1];
                    if ( pGroupItem->mnBits & TIB_RADIOCHECK )
                    {
                        if ( pGroupItem->meState != STATE_NOCHECK )
                            SetItemState( pGroupItem->mnId, STATE_NOCHECK );
                    }
                    else
                        break;
                    nGroupPos--;
                }

                nGroupPos = nPos+1;
                while ( nGroupPos < nItemCount )
                {
                    pGroupItem = &mpData->m_aItems[nGroupPos];
                    if ( pGroupItem->mnBits & TIB_RADIOCHECK )
                    {
                        if ( pGroupItem->meState != STATE_NOCHECK )
                            SetItemState( pGroupItem->mnId, STATE_NOCHECK );
                    }
                    else
                        break;
                    nGroupPos++;
                }
            }

            pItem->meState = eState;
            ImplUpdateItem( nPos );

            // Notify button changed event to prepare accessibility bridge
            ImplCallEventListeners( VCLEVENT_TOOLBOX_BUTTONSTATECHANGED, reinterpret_cast< void* >( nPos ) );

            // Notify
            ImplCallEventListeners( VCLEVENT_TOOLBOX_CLICK, reinterpret_cast< void* >( nPos ) );
        }
    }
}

// -----------------------------------------------------------------------

TriState ToolBox::GetItemState( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->meState;
    else
        return STATE_NOCHECK;
}

// -----------------------------------------------------------------------

void ToolBox::EnableItem( USHORT nItemId, BOOL bEnable )
{
    USHORT nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        if ( bEnable )
            bEnable = TRUE;
        if ( pItem->mbEnabled != bEnable )
        {
            pItem->mbEnabled = bEnable;

            // Gegebenenfalls das Fenster mit updaten
            if ( pItem->mpWindow )
                pItem->mpWindow->Enable( pItem->mbEnabled );

            // Item updaten
            ImplUpdateItem( nPos );

            ImplUpdateInputEnable();

            // Notify button changed event to prepare accessibility bridge
            ImplCallEventListeners( VCLEVENT_TOOLBOX_BUTTONSTATECHANGED, reinterpret_cast< void* >( nPos ) );

            ImplCallEventListeners( bEnable ? VCLEVENT_TOOLBOX_ITEMENABLED : VCLEVENT_TOOLBOX_ITEMDISABLED, reinterpret_cast< void* >( nPos ) );
        }
    }
}

// -----------------------------------------------------------------------

BOOL ToolBox::IsItemEnabled( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mbEnabled;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void ToolBox::ShowItem( USHORT nItemId, BOOL bVisible )
{
    USHORT nPos = GetItemPos( nItemId );
    mpData->ImplClearLayoutData();

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        if ( pItem->mbVisible != bVisible )
        {
            pItem->mbVisible = bVisible;
            ImplInvalidate( FALSE );
        }
    }
}

// -----------------------------------------------------------------------

BOOL ToolBox::IsItemVisible( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mbVisible;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

BOOL ToolBox::IsItemReallyVisible( USHORT nItemId ) const
{
    // is the item on the visible area of the toolbox?
    BOOL bRet = FALSE;
    Rectangle aRect( mnLeftBorder, mnTopBorder, mnDX-mnRightBorder, mnDY-mnBottomBorder );
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem && pItem->mbVisible &&
         !pItem->maRect.IsEmpty() && aRect.IsOver( pItem->maRect ) )
    {
        bRet = TRUE;
    }

    return bRet;
}

// -----------------------------------------------------------------------

void ToolBox::SetItemCommand( USHORT nItemId, const XubString& rCommand )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        pItem->maCommandStr = rCommand;
}

// -----------------------------------------------------------------------

const XubString& ToolBox::GetItemCommand( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maCommandStr;
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void ToolBox::SetQuickHelpText( USHORT nItemId, const XubString& rText )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        pItem->maQuickHelpText = rText;
}

// -----------------------------------------------------------------------

const XubString& ToolBox::GetQuickHelpText( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maQuickHelpText;
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void ToolBox::SetHelpText( USHORT nItemId, const XubString& rText )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        pItem->maHelpText = rText;
}

// -----------------------------------------------------------------------

const XubString& ToolBox::GetHelpText( USHORT nItemId ) const
{
    return ImplGetHelpText( nItemId );
}

// -----------------------------------------------------------------------

void ToolBox::SetHelpId( USHORT nItemId, ULONG nHelpId )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        pItem->mnHelpId = nHelpId;
}

// -----------------------------------------------------------------------

ULONG ToolBox::GetHelpId( USHORT nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mnHelpId;
    else
        return 0;
}

// -----------------------------------------------------------------------

void ToolBox::SetBorder( long nX, long nY )
{
    mnBorderX = nX;
    mnBorderY = nY;

    ImplInvalidate( TRUE, TRUE );
}

// -----------------------------------------------------------------------

void ToolBox::SetOutStyle( USHORT nNewStyle )
{
    // always force flat looking toolbars since NWF
    nNewStyle |= TOOLBOX_STYLE_FLAT;

    if ( mnOutStyle != nNewStyle )
    {
        mnOutStyle = nNewStyle;
        ImplDisableFlatButtons();

        // Damit das ButtonDevice neu angelegt wird
        if ( !(mnOutStyle & TOOLBOX_STYLE_FLAT) )
        {
            mnMaxItemWidth  = 1;
            mnMaxItemHeight = 1;
        }

        ImplInvalidate( TRUE, TRUE );
    }
}

// -----------------------------------------------------------------------

void ToolBox::RecalcItems()
{
    ImplInvalidate( TRUE );
}

// -----------------------------------------------------------------------

// disable key input if all items are disabled

void ToolBox::ImplUpdateInputEnable()
{
    for( std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
         it != mpData->m_aItems.end(); ++it )
    {
        if( it->mbEnabled )
        {
            // at least one useful entry
            mpData->mbKeyInputDisabled = FALSE;
            return;
        }
    }
    mpData->mbKeyInputDisabled = TRUE;
}

// -----------------------------------------------------------------------

void ToolBox::ImplFillLayoutData() const
{
    mpData->m_pLayoutData = new ToolBoxLayoutData();

    USHORT nCount = (USHORT)mpData->m_aItems.size();
    for( USHORT i = 0; i < nCount; i++ )
    {
        ImplToolItem* pItem = &mpData->m_aItems[i];

        // Nur malen, wenn Rechteck im PaintRectangle liegt
        if ( !pItem->maRect.IsEmpty() )
            const_cast<ToolBox*>(this)->ImplDrawItem( i, FALSE, FALSE, TRUE );
    }
}

// -----------------------------------------------------------------------

String ToolBox::GetDisplayText() const
{
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    return mpData->m_pLayoutData ? mpData->m_pLayoutData->m_aDisplayText : String();
}

// -----------------------------------------------------------------------

Rectangle ToolBox::GetCharacterBounds( USHORT nItemID, long nIndex ) const
{
    long nItemIndex = -1;
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    if( mpData->m_pLayoutData )
    {
        for( ULONG i = 0; i < mpData->m_pLayoutData->m_aLineItemIds.size(); i++ )
        {
            if( mpData->m_pLayoutData->m_aLineItemIds[i] == nItemID )
            {
                nItemIndex = mpData->m_pLayoutData->m_aLineIndices[i];
                break;
            }
        }
    }
    return (mpData->m_pLayoutData && nItemIndex != -1) ? mpData->m_pLayoutData->GetCharacterBounds( nItemIndex+nIndex ) : Rectangle();
}

// -----------------------------------------------------------------------

long ToolBox::GetIndexForPoint( const Point& rPoint, USHORT& rItemID ) const
{
    long nIndex = -1;
    rItemID = 0;
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    if( mpData->m_pLayoutData )
    {
        nIndex = mpData->m_pLayoutData->GetIndexForPoint( rPoint );
        for( ULONG i = 0; i < mpData->m_pLayoutData->m_aLineIndices.size(); i++ )
        {
            if( mpData->m_pLayoutData->m_aLineIndices[i] <= nIndex &&
                (i == mpData->m_pLayoutData->m_aLineIndices.size()-1 || mpData->m_pLayoutData->m_aLineIndices[i+1] > nIndex) )
            {
                rItemID = mpData->m_pLayoutData->m_aLineItemIds[i];
                break;
            }
        }
    }
    return nIndex;
}

// -----------------------------------------------------------------------

long ToolBox::GetTextCount() const
{
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    return mpData->m_pLayoutData ? mpData->m_pLayoutData->GetLineCount() : 0;
}

// -----------------------------------------------------------------------

Pair ToolBox::GetTextStartEnd( long nText ) const
{
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    return mpData->m_pLayoutData ? mpData->m_pLayoutData->GetLineStartEnd( nText ) : Pair( -1, -1 );
}

// -----------------------------------------------------------------------

USHORT ToolBox::GetDisplayItemId( long nText ) const
{
    USHORT nItemId = 0;
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    if( mpData->m_pLayoutData && nText >= 0 && (ULONG)nText < mpData->m_pLayoutData->m_aLineItemIds.size() )
        nItemId = mpData->m_pLayoutData->m_aLineItemIds[nText];
    return nItemId;
}


// -----------------------------------------------------------------------

void ToolBox::SetDropdownClickHdl( const Link& rLink )
{
    mpData->maDropdownClickHdl = rLink;
}

const Link& ToolBox::GetDropdownClickHdl() const
{
    return mpData->maDropdownClickHdl;
}

// -----------------------------------------------------------------------

void ToolBox::SetMenuType( USHORT aType )
{
    if( aType != mpData->maMenuType )
    {
        mpData->maMenuType = aType;
        if( IsFloatingMode() )
        {
            // the menu button may have to be moved into the decoration which changes the layout
            ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
            if( pWrapper )
                pWrapper->ShowTitleButton( TITLE_BUTTON_MENU, ( aType & TOOLBOX_MENUTYPE_CUSTOMIZE) ? TRUE : FALSE );

            mbFormat = TRUE;
            ImplFormat();
            ImplSetMinMaxFloatSize( this );
        }
        else
        {
            // trigger redraw of menu button
            if( !mpData->maMenubuttonItem.maRect.IsEmpty() )
                Invalidate(mpData->maMenubuttonItem.maRect);
        }
    }
}

USHORT ToolBox::GetMenuType() const
{
    return mpData->maMenuType;
}

BOOL ToolBox::IsMenuEnabled() const
{
    return mpData->maMenuType != TOOLBOX_MENUTYPE_NONE;
}

PopupMenu* ToolBox::GetMenu() const
{
    return mpData->mpMenu;
}

void ToolBox::SetMenuButtonHdl( const Link& rLink )
{
    mpData->maMenuButtonHdl = rLink;
}

const Link& ToolBox::GetMenuButtonHdl() const
{
    return mpData->maMenuButtonHdl;
}

// -----------------------------------------------------------------------

BOOL ToolBox::ImplHasClippedItems()
{
    // are any items currently clipped ?
    ImplFormat();
    std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
    while ( it != mpData->m_aItems.end() )
    {
        if( it->IsClipped() )
            return TRUE;
        ++it;
    }
    return FALSE;
}

void ToolBox::ImplUpdateCustomMenu()
{
    // fill clipped items into menu
    if( !IsMenuEnabled() )
        return;

    PopupMenu *pMenu = GetMenu();

    USHORT i = 0;
    // remove old entries
    while( i < pMenu->GetItemCount() )
    {
        if( pMenu->GetItemId( i ) >= TOOLBOX_MENUITEM_START )
        {
            pMenu->RemoveItem( i );
            i = 0;
        }
        else
            i++;
    }

    // add menu items, starting from the end and inserting at pos 0
    if ( !mpData->m_aItems.empty() )
    {
        for ( std::vector< ImplToolItem >::reverse_iterator it(mpData->m_aItems.rbegin());
                it != mpData->m_aItems.rend(); ++it)
        {
            if( it->IsClipped() )
            {
                USHORT id = it->mnId + TOOLBOX_MENUITEM_START;
                pMenu->InsertItem( id, it->maText, it->maImage, 0, 0 );
                pMenu->EnableItem( id, it->mbEnabled );
                pMenu->CheckItem( id, it->meState == STATE_CHECK );
            }
        }
    }
}

IMPL_LINK( ToolBox, ImplCustomMenuListener, VclMenuEvent*, pEvent )
{
    if( pEvent->GetMenu() == GetMenu() && pEvent->GetId() == VCLEVENT_MENU_SELECT )
    {
        USHORT id = GetMenu()->GetItemId( pEvent->GetItemPos() );
        if( id >= TOOLBOX_MENUITEM_START )
            TriggerItem( id - TOOLBOX_MENUITEM_START, FALSE, FALSE );
    }
    return 0;
}

IMPL_LINK( ToolBox, ImplCallExecuteCustomMenu, void*, EMPTYARG )
{
    mpData->mnEventId = 0;
    ImplExecuteCustomMenu();
    return 0;
}

void ToolBox::ImplExecuteCustomMenu()
{
    if( IsMenuEnabled() )
    {
        if( GetMenuType() & TOOLBOX_MENUTYPE_CUSTOMIZE )
            // call button handler to allow for menu customization
            mpData->maMenuButtonHdl.Call( this );

        // register handler
        GetMenu()->AddEventListener( LINK( this, ToolBox, ImplCustomMenuListener ) );

        // make sure all disabled entries will be shown
        GetMenu()->SetMenuFlags(
            GetMenu()->GetMenuFlags() | MENU_FLAG_ALWAYSSHOWDISABLEDENTRIES );

        // toolbox might be destroyed during execute
        ImplDelData aDelData;
        ImplAddDel( &aDelData );
        ImplDelData aBorderDel;
        bool bBorderDel = false;

        Window *pWin = this;
        Rectangle aMenuRect = mpData->maMenubuttonItem.maRect;
        if( IsFloatingMode() )
        {
            // custom menu is placed in the decoration
            ImplBorderWindow *pBorderWin = dynamic_cast<ImplBorderWindow*>( GetWindow( WINDOW_BORDER ) );
            if( pBorderWin && !pBorderWin->GetMenuRect().IsEmpty() )
            {
                pWin = pBorderWin;
                aMenuRect = pBorderWin->GetMenuRect();
                pWin->ImplAddDel( &aBorderDel );
                bBorderDel = true;
            }
        }

        USHORT uId = GetMenu()->Execute( pWin, Rectangle( ImplGetPopupPosition( aMenuRect, Size() ), Size() ),
                                POPUPMENU_EXECUTE_DOWN | POPUPMENU_NOMOUSEUPCLOSE );

        if ( aDelData.IsDelete() )
            return;
        ImplRemoveDel( &aDelData );

        if( GetMenu() )
            GetMenu()->RemoveEventListener( LINK( this, ToolBox, ImplCustomMenuListener ) );
        if( bBorderDel )
        {
            if( aBorderDel.IsDelete() )
                return;
            pWin->ImplRemoveDel( &aBorderDel );
        }

        pWin->Invalidate( aMenuRect );

        if( uId )
            GrabFocusToDocument();
    }
}

void ToolBox::ExecuteCustomMenu()
{
    if( IsMenuEnabled() )
    {
        // handle custom menu asynchronously
        // to avoid problems if the toolbox is closed during menu execute
        ImplUpdateCustomMenu();
        Application::PostUserEvent( mpData->mnEventId, LINK( this, ToolBox, ImplCallExecuteCustomMenu ) );
    }
}

// -----------------------------------------------------------------------

// checks override first, useful during calculation of sizes
BOOL ToolBox::ImplIsFloatingMode() const
{
    DBG_ASSERT( !(mpData->mbAssumeDocked && mpData->mbAssumeFloating),
        "ToolBox::ImplIsFloatingMode(): cannot assume docked and floating" );

    if( mpData->mbAssumeDocked )
        return FALSE;
    else if( mpData->mbAssumeFloating )
        return TRUE;
    else
        return IsFloatingMode();
}

// checks override first, useful during calculation of sizes
BOOL ToolBox::ImplIsInPopupMode() const
{
    if( mpData->mbAssumePopupMode )
        return TRUE;
    else
    {
        ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
        return ( pWrapper && pWrapper->GetFloatingWindow() && pWrapper->GetFloatingWindow()->IsInPopupMode() );
    }
}

// -----------------------------------------------------------------------

void ToolBox::Lock( BOOL bLock )
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( !pWrapper )
        return;
    if( mpData->mbIsLocked != bLock )
    {
        mpData->mbIsLocked = bLock;
        if( !ImplIsFloatingMode() )
        {
            mbCalc = TRUE;
            mbFormat = TRUE;
            SetSizePixel( CalcWindowSizePixel(1) );
            Invalidate();
        }
    }
}

// -----------------------------------------------------------------------

BOOL ToolBox::AlwaysLocked()
{
    // read config item to determine toolbox behaviour, used for subtoolbars

    static int nAlwaysLocked = -1;

    if( nAlwaysLocked == -1 )
    {
        nAlwaysLocked = 0; // ask configuration only once

        utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithServiceFactory(
            vcl::unohelper::GetMultiServiceFactory(),
            OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Office.UI.GlobalSettings/Toolbars")) );    // note: case sensisitive !
        if ( aNode.isValid() )
        {
            // feature enabled ?
            BOOL bStatesEnabled = BOOL();
            ::com::sun::star::uno::Any aValue = aNode.getNodeValue( OUString(RTL_CONSTASCII_USTRINGPARAM("StatesEnabled")) );
            if( aValue >>= bStatesEnabled )
            {
                if( bStatesEnabled == TRUE )
                {
                    // now read the locking state
                    utl::OConfigurationNode aNode2 = utl::OConfigurationTreeRoot::tryCreateWithServiceFactory(
                        vcl::unohelper::GetMultiServiceFactory(),
                        OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Office.UI.GlobalSettings/Toolbars/States")) );    // note: case sensisitive !

                    BOOL bLocked = BOOL();
                    ::com::sun::star::uno::Any aValue2 = aNode2.getNodeValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Locked")) );
                    if( aValue2 >>= bLocked )
                        nAlwaysLocked = (bLocked == TRUE) ? 1 : 0;
                }
            }
        }
    }

    return nAlwaysLocked == 1 ? TRUE : FALSE;
}

BOOL ToolBox::WillUsePopupMode() const
{
    return mpData->mbWillUsePopupMode;
}

void ToolBox::WillUsePopupMode( BOOL b )
{
    mpData->mbWillUsePopupMode = b;
}

void ToolBox::ImplUpdateImageList()
{
    if (mpData->mpImageListProvider != NULL)
    {
        try
        {
            ImageListType eType = vcl::HIGHCONTRAST_NO;
            if (eType != mpData->meImageListType)
            {
                vcl::IImageListProvider* pImageListProvider = mpData->mpImageListProvider;
                SetImageList( pImageListProvider->getImageList(eType) );
                mpData->meImageListType = eType;
            }
        }
        catch (com::sun::star::lang::IllegalArgumentException &) {}
    }
}

void ToolBox::SetImageListProvider(vcl::IImageListProvider* _pProvider)
{
    mpData->mpImageListProvider = _pProvider;
    ImplUpdateImageList();
}
// -----------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
