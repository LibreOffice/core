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

#include "sal/config.h"

#include <comphelper/processfactory.hxx>
#include <tools/debug.hxx>
#include <tools/rc.h>

#include <vcl/svapp.hxx>
#include <vcl/idle.hxx>
#include <vcl/help.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>
#include <vcl/IconThemeInfo.hxx>
#include <vcl/commandinfoprovider.hxx>

#include <svdata.hxx>
#include <brdwin.hxx>
#include <toolbox.h>

#include <unotools/confignode.hxx>

using namespace vcl;

#define TB_SEP_SIZE     8  // Separator size


ImplToolBoxPrivateData::ImplToolBoxPrivateData() :
        m_pLayoutData( nullptr )
{
    meButtonSize = ToolBoxButtonSize::DontCare;
    mpMenu = VclPtr<PopupMenu>::Create();
    mnEventId = nullptr;

    maMenuType = ToolBoxMenuType::NONE;
    maMenubuttonItem.maItemSize = Size( TB_MENUBUTTON_SIZE+TB_MENUBUTTON_OFFSET, TB_MENUBUTTON_SIZE+TB_MENUBUTTON_OFFSET );
    maMenubuttonItem.meState = TRISTATE_FALSE;
    mnMenuButtonWidth = TB_MENUBUTTON_SIZE;

    mbIsLocked = false;
    mbNativeButtons = false;
    mbIsPaintLocked = false;
    mbAssumeDocked = false;
    mbAssumePopupMode = false;
    mbAssumeFloating = false;
    mbKeyInputDisabled = false;
    mbMenubuttonSelected = false;
    mbPageScroll = false;
    mbWillUsePopupMode = false;
    mbDropDownByKeyboard = false;
}

ImplToolBoxPrivateData::~ImplToolBoxPrivateData()
{
    delete m_pLayoutData;
    mpMenu.disposeAndClear();
}

void ImplToolItem::init(sal_uInt16 nItemId, ToolBoxItemBits nItemBits,
                        bool bEmptyBtn)
{
    mnId            = nItemId;
    mpWindow        = nullptr;
    mpUserData      = nullptr;
    meType          = ToolBoxItemType::BUTTON;
    mnBits          = nItemBits;
    meState         = TRISTATE_FALSE;
    mbEnabled       = true;
    mbVisible       = true;
    mbEmptyBtn      = bEmptyBtn;
    mbShowWindow    = false;
    mbBreak         = false;
    mnSepSize       = TB_SEP_SIZE;
    mnDropDownArrowWidth = TB_DROPDOWNARROWWIDTH;
    mnImageAngle    = 0;
    mbMirrorMode    = false;
    mbVisibleText   = false;
    mbExpand        = false;
}

ImplToolItem::ImplToolItem()
{
    init(0, ToolBoxItemBits::NONE, true);
}

ImplToolItem::ImplToolItem( sal_uInt16 nItemId, const Image& rImage,
                            ToolBoxItemBits nItemBits ) :
    maImage( rImage )
{
    init(nItemId, nItemBits, false);
}

ImplToolItem::ImplToolItem( sal_uInt16 nItemId, const OUString& rText,
                            ToolBoxItemBits nItemBits ) :
    maText( rText )
{
    init(nItemId, nItemBits, false);
}

ImplToolItem::ImplToolItem( sal_uInt16 nItemId, const Image& rImage,
                            const OUString& rText, ToolBoxItemBits nItemBits ) :
    maImage( rImage ),
    maText( rText )
{
    init(nItemId, nItemBits, false);
}

Size ImplToolItem::GetSize( bool bHorz, bool bCheckMaxWidth, long maxWidth, const Size& rDefaultSize )
{
    Size aSize( rDefaultSize ); // the size of 'standard' toolbox items
                                // non-standard items are eg windows or buttons with text

    if ( (meType == ToolBoxItemType::BUTTON) || (meType == ToolBoxItemType::SPACE) )
    {
        aSize = maItemSize;

        if ( mpWindow && bHorz )
        {
            // get size of item window and check if it fits
            // no windows in vertical toolbars (the default is mbShowWindow=false)
            Size aWinSize = mpWindow->GetSizePixel();

            if (mpWindow->GetStyle() & WB_NOLABEL)
                // Window wants no label? Then don't check width, it'll be just
                // clipped.
                bCheckMaxWidth = false;

            if ( !bCheckMaxWidth || (aWinSize.Width() <= maxWidth) )
            {
                aSize.Width()   = aWinSize.Width();
                aSize.Height()  = aWinSize.Height();
                mbShowWindow = true;
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
    else if ( meType == ToolBoxItemType::SEPARATOR )
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
    else if ( meType == ToolBoxItemType::BREAK )
    {
        aSize.Width()   = 0;
        aSize.Height()  = 0;
    }

    return aSize;
}

void ImplToolItem::DetermineButtonDrawStyle( ButtonType eButtonType, bool& rbImage, bool& rbText ) const
{
    if ( meType != ToolBoxItemType::BUTTON )
    {
        // no button -> draw nothing
        rbImage = rbText = false;
        return;
    }

    bool bHasImage;
    bool bHasText;

    // check for image and/or text
    if ( !(maImage) )
        bHasImage = false;
    else
        bHasImage = true;
    if ( maText.isEmpty() )
        bHasText = false;
    else
        bHasText = true;

    // prefer images if symbolonly buttons are drawn
    // prefer texts if textonly buttons are drawn

    if ( eButtonType == ButtonType::SYMBOLONLY )         // drawing icons only
    {
        if( bHasImage || !bHasText )
        {
            rbImage = true;
            rbText  = false;
        }
        else
        {
            rbImage = false;
            rbText  = true;
        }
    }
    else if ( eButtonType == ButtonType::TEXT )      // drawing text only
    {
        if( bHasText || !bHasImage )
        {
            rbImage = false;
            rbText  = true;
        }
        else
        {
            rbImage = true;
            rbText  = false;
        }
    }
    else                                        // drawing icons and text both
    {
        rbImage = true;
        rbText  = true;
    }
}

Rectangle ImplToolItem::GetDropDownRect( bool bHorz ) const
{
    Rectangle aRect;
    if( (mnBits & ToolBoxItemBits::DROPDOWN) && !maRect.IsEmpty() )
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

bool ImplToolItem::IsClipped() const
{
    return ( meType == ToolBoxItemType::BUTTON && mbVisible && maRect.IsEmpty() );
}

bool ImplToolItem::IsItemHidden() const
{
    return ( meType == ToolBoxItemType::BUTTON && !mbVisible );
}

void ToolBox::ImplInvalidate( bool bNewCalc, bool bFullPaint )
{
    ImplUpdateInputEnable();

    if ( bNewCalc )
        mbCalc = true;

    if ( bFullPaint )
    {
        mbFormat = true;

        // do we need to redraw?
        if ( IsReallyVisible() && IsUpdateMode() )
        {
            Invalidate( Rectangle( mnLeftBorder, mnTopBorder,
                                   mnDX-mnRightBorder-1, mnDY-mnBottomBorder-1 ) );
            mpIdle->Stop();
        }
    }
    else
    {
        if ( !mbFormat )
        {
            mbFormat = true;

            // do we need to redraw?
            if ( IsReallyVisible() && IsUpdateMode() )
                mpIdle->Start();
        }
    }

    // request new layout by layoutmanager
    CallEventListeners( VclEventId::ToolboxFormatChanged );
}

void ToolBox::ImplUpdateItem( sal_uInt16 nIndex )
{
    // do we need to redraw?
    if ( IsReallyVisible() && IsUpdateMode() )
    {
        if ( nIndex == 0xFFFF )
        {
            // #i52217# no immediate draw as this might lead to paint problems
            Invalidate( Rectangle( mnLeftBorder, mnTopBorder, mnDX-mnRightBorder-1, mnDY-mnBottomBorder-1 ) );
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

void ToolBox::Click()
{
    CallEventListeners( VclEventId::ToolboxClick );
    maClickHdl.Call( this );
}

void ToolBox::DoubleClick()
{
    CallEventListeners( VclEventId::ToolboxDoubleClick );
    maDoubleClickHdl.Call( this );
}

void ToolBox::Activate()
{
    mnActivateCount++;
    CallEventListeners( VclEventId::ToolboxActivate );
    maActivateHdl.Call( this );
}

void ToolBox::Deactivate()
{
    mnActivateCount--;
    CallEventListeners( VclEventId::ToolboxDeactivate );
    maDeactivateHdl.Call( this );
}

void ToolBox::Highlight()
{
    CallEventListeners( VclEventId::ToolboxHighlight );
}

void ToolBox::Select()
{
    VclPtr<vcl::Window> xWindow = this;

    CallEventListeners( VclEventId::ToolboxSelect );
    maSelectHdl.Call( this );

    if ( xWindow->IsDisposed() )
        return;

    // TODO: GetFloatingWindow in DockingWindow is currently inline, change it to check dockingwrapper
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper && pWrapper->GetFloatingWindow() && pWrapper->GetFloatingWindow()->IsInPopupMode() )
        pWrapper->GetFloatingWindow()->EndPopupMode();
}

void ToolBox::InsertItem( sal_uInt16 nItemId, const Image& rImage, ToolBoxItemBits nBits, sal_uInt16 nPos )
{
    SAL_WARN_IF( !nItemId, "vcl", "ToolBox::InsertItem(): ItemId == 0" );
    SAL_WARN_IF( GetItemPos( nItemId ) != TOOLBOX_ITEM_NOTFOUND, "vcl",
                "ToolBox::InsertItem(): ItemId already exists" );

    // create item and add to list
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(),
                             ImplToolItem( nItemId, rImage, nBits ) );
    SetItemImage(nItemId, rImage);
    mpData->ImplClearLayoutData();

    ImplInvalidate( true );

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >(nNewPos ) );
}

void ToolBox::InsertItem( sal_uInt16 nItemId, const Image& rImage, const OUString& rText, ToolBoxItemBits nBits,
                          sal_uInt16 nPos )
{
    SAL_WARN_IF( !nItemId, "vcl", "ToolBox::InsertItem(): ItemId == 0" );
    SAL_WARN_IF( GetItemPos( nItemId ) != TOOLBOX_ITEM_NOTFOUND, "vcl",
                "ToolBox::InsertItem(): ItemId already exists" );

    // create item and add to list
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(),
                             ImplToolItem( nItemId, rImage, MnemonicGenerator::EraseAllMnemonicChars(rText), nBits ) );
    SetItemImage(nItemId, rImage);
    mpData->ImplClearLayoutData();

    ImplInvalidate( true );

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::InsertItem( sal_uInt16 nItemId, const OUString& rText, ToolBoxItemBits nBits, sal_uInt16 nPos )
{
    SAL_WARN_IF( !nItemId, "vcl", "ToolBox::InsertItem(): ItemId == 0" );
    SAL_WARN_IF( GetItemPos( nItemId ) != TOOLBOX_ITEM_NOTFOUND, "vcl",
                "ToolBox::InsertItem(): ItemId already exists" );

    // create item and add to list
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(),
                             ImplToolItem( nItemId, MnemonicGenerator::EraseAllMnemonicChars(rText), nBits ) );
    mpData->ImplClearLayoutData();

    ImplInvalidate( true );

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::InsertItem(const OUString& rCommand, const css::uno::Reference<css::frame::XFrame>& rFrame, ToolBoxItemBits nBits,
                         const Size& rRequestedSize, sal_uInt16 nPos)
{
    OUString aLabel(vcl::CommandInfoProvider::GetLabelForCommand(rCommand, rFrame));
    OUString aTooltip(vcl::CommandInfoProvider::GetTooltipForCommand(rCommand, rFrame));

    vcl::ImageType eImageType = vcl::ImageType::Size16;

    if (GetToolboxButtonSize() == ToolBoxButtonSize::Large)
        eImageType = vcl::ImageType::Size26;
    else if (GetToolboxButtonSize() == ToolBoxButtonSize::Size32)
        eImageType = vcl::ImageType::Size32;

    Image aImage(CommandInfoProvider::GetImageForCommand(rCommand, rFrame, eImageType));

    sal_uInt16 nItemId = GetItemCount() + 1;
    InsertItem(nItemId, aImage, aLabel, nBits, nPos);
    SetItemCommand(nItemId, rCommand);
    SetQuickHelpText(nItemId, aTooltip);

    // set the minimal size
    ImplToolItem* pItem = ImplGetItem( nItemId );
    if ( pItem )
        pItem->maMinimalItemSize = rRequestedSize;
}

void ToolBox::InsertWindow( sal_uInt16 nItemId, vcl::Window* pWindow,
                            ToolBoxItemBits nBits, sal_uInt16 nPos )
{
    SAL_WARN_IF( !nItemId, "vcl", "ToolBox::InsertWindow(): ItemId == 0" );
    SAL_WARN_IF( GetItemPos( nItemId ) != TOOLBOX_ITEM_NOTFOUND, "vcl",
                "ToolBox::InsertWindow(): ItemId already exists" );

    // create item and add to list
    ImplToolItem aItem;
    aItem.mnId       = nItemId;
    aItem.meType     = ToolBoxItemType::BUTTON;
    aItem.mnBits     = nBits;
    aItem.mpWindow   = pWindow;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    if ( pWindow )
        pWindow->Hide();

    ImplInvalidate( true );

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::InsertSpace()
{
    // create item and add to list
    ImplToolItem aItem;
    aItem.meType     = ToolBoxItemType::SPACE;
    aItem.mbEnabled  = false;
    mpData->m_aItems.push_back( aItem );
    mpData->ImplClearLayoutData();

    ImplInvalidate();

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(mpData->m_aItems.size() - 1);
    CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::InsertSeparator( sal_uInt16 nPos, sal_uInt16 nPixSize )
{
    // create item and add to list
    ImplToolItem aItem;
    aItem.meType     = ToolBoxItemType::SEPARATOR;
    aItem.mbEnabled  = false;
    if ( nPixSize )
        aItem.mnSepSize = nPixSize;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    ImplInvalidate();

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::InsertBreak( sal_uInt16 nPos )
{
    // create item and add to list
    ImplToolItem aItem;
    aItem.meType     = ToolBoxItemType::BREAK;
    aItem.mbEnabled  = false;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    ImplInvalidate();

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::RemoveItem( sal_uInt16 nPos )
{
    if( nPos < mpData->m_aItems.size() )
    {
        bool bMustCalc;
        if ( mpData->m_aItems[nPos].meType == ToolBoxItemType::BUTTON )
            bMustCalc = true;
        else
            bMustCalc = false;

        if ( mpData->m_aItems[nPos].mpWindow )
            mpData->m_aItems[nPos].mpWindow->Hide();

        // add the removed item to PaintRect
        maPaintRect.Union( mpData->m_aItems[nPos].maRect );

        // ensure not to delete in the Select-Handler
        if ( mpData->m_aItems[nPos].mnId == mnCurItemId )
            mnCurItemId = 0;
        if ( mpData->m_aItems[nPos].mnId == mnHighItemId )
            mnHighItemId = 0;

        ImplInvalidate( bMustCalc );

        mpData->m_aItems.erase( mpData->m_aItems.begin()+nPos );
        mpData->ImplClearLayoutData();

        // Notify
        CallEventListeners( VclEventId::ToolboxItemRemoved, reinterpret_cast< void* >( nPos ) );
    }
}

void ToolBox::CopyItem( const ToolBox& rToolBox, sal_uInt16 nItemId )
{
    SAL_WARN_IF( GetItemPos( nItemId ) != TOOLBOX_ITEM_NOTFOUND, "vcl",
                "ToolBox::CopyItem(): ItemId already exists" );

    sal_uInt16 nPos = rToolBox.GetItemPos( nItemId );

    // found item
    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        // push ToolBox item onto the list
        ImplToolItem aNewItem = rToolBox.mpData->m_aItems[nPos];
        // reset state
        aNewItem.mpWindow      = nullptr;
        aNewItem.mbShowWindow = false;

        mpData->m_aItems.push_back( aNewItem );
        mpData->ImplClearLayoutData();
        // redraw ToolBox
        ImplInvalidate();

        // Notify
        sal_uInt16 nNewPos2 = sal::static_int_cast<sal_uInt16>(mpData->m_aItems.size() - 1);
        CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >( nNewPos2 ) );
    }
}

void ToolBox::Clear()
{
    mpData->m_aItems.clear();
    mpData->ImplClearLayoutData();

    // ensure not to delete in the Select-Handler
    mnCurItemId = 0;
    mnHighItemId = 0;

    ImplInvalidate( true, true );

    // Notify
    CallEventListeners( VclEventId::ToolboxAllItemsChanged );
}

void ToolBox::SetButtonType( ButtonType eNewType )
{
    if ( meButtonType != eNewType )
    {
        meButtonType = eNewType;

        // better redraw everything, as otherwise there might be problems
        // with regions that were copied with CopyBits
        ImplInvalidate( true );
    }
}

void ToolBox::SetToolboxButtonSize( ToolBoxButtonSize eSize )
{
    if( mpData->meButtonSize != eSize )
    {
        mpData->meButtonSize = eSize;
        mbCalc = true;
        mbFormat = true;
    }
}

ToolBoxButtonSize ToolBox::GetToolboxButtonSize() const
{
    return mpData->meButtonSize;
}

/*static*/ Size ToolBox::GetDefaultImageSize(ToolBoxButtonSize eToolBoxButtonSize)
{
    float fScaleFactor = Application::GetDefaultDevice()->GetDPIScaleFactor();

    Size aUnscaledSize = Size(16, 16);

    if (eToolBoxButtonSize == ToolBoxButtonSize::Large)
    {
        OUString iconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
        aUnscaledSize = vcl::IconThemeInfo::SizeByThemeName(iconTheme);
    }
    else if (eToolBoxButtonSize == ToolBoxButtonSize::Size32)
    {
        aUnscaledSize = Size(32, 32);
    }
    return Size(aUnscaledSize.Width()  * fScaleFactor,
                aUnscaledSize.Height() * fScaleFactor);
}

Size ToolBox::GetDefaultImageSize() const
{
    return GetDefaultImageSize(GetToolboxButtonSize());
}

void ToolBox::SetAlign( WindowAlign eNewAlign )
{
    if ( meAlign != eNewAlign )
    {
        meAlign = eNewAlign;

        if ( !ImplIsFloatingMode() )
        {
            // set horizontal/vertical alignment
            if ( (eNewAlign == WindowAlign::Left) || (eNewAlign == WindowAlign::Right) )
                mbHorz = false;
            else
                mbHorz = true;

            // Update the background according to Persona if necessary
            ImplInitSettings( false, false, true );

            // redraw everything, as the border has changed
            mbCalc = true;
            mbFormat = true;
            if ( IsReallyVisible() && IsUpdateMode() )
                Invalidate();
        }
    }
}

void ToolBox::SetLineCount( sal_uInt16 nNewLines )
{
    if ( !nNewLines )
        nNewLines = 1;

    if ( mnLines != nNewLines )
    {
        mnLines = nNewLines;

        // better redraw everything, as otherwise there might be problems
        // with regions that were copied with CopyBits
        Invalidate();
    }
}

void ToolBox::SetPageScroll( bool b )
{
    mpData->mbPageScroll = b;
}

sal_uInt16 ToolBox::GetItemCount() const
{
    return mpData ? (sal_uInt16)mpData->m_aItems.size() : 0;
}

ToolBoxItemType ToolBox::GetItemType( sal_uInt16 nPos ) const
{
    return (nPos < mpData->m_aItems.size()) ? mpData->m_aItems[nPos].meType : ToolBoxItemType::DONTKNOW;
}

sal_uInt16 ToolBox::GetItemPos( sal_uInt16 nItemId ) const
{
    if (mpData)
    {
        int nCount = mpData->m_aItems.size();
        for( int nPos = 0; nPos < nCount; nPos++ )
            if( mpData->m_aItems[nPos].mnId == nItemId )
                return (sal_uInt16)nPos;
    }
    return TOOLBOX_ITEM_NOTFOUND;
}

sal_uInt16 ToolBox::GetItemPos( const Point& rPos ) const
{
    // search the item position on the given point
    sal_uInt16 nRet = TOOLBOX_ITEM_NOTFOUND;
    sal_uInt16 nPos = 0;
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

sal_uInt16 ToolBox::GetItemId( sal_uInt16 nPos ) const
{
    return (nPos < mpData->m_aItems.size()) ? mpData->m_aItems[nPos].mnId : 0;
}

sal_uInt16 ToolBox::GetItemId( const Point& rPos ) const
{
    // find item that was clicked
    std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
    while( it != mpData->m_aItems.end() )
    {
        // is it this item?
        if ( it->maRect.IsInside( rPos ) )
        {
            if ( it->meType == ToolBoxItemType::BUTTON )
                return it->mnId;
            else
                return 0;
        }

        ++it;
    }

    return 0;
}

Size ToolBox::GetItemContentSize( sal_uInt16 nItemId ) const
{
    if ( mbCalc || mbFormat )
        (const_cast<ToolBox*>(this))->ImplFormat();

    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos < mpData->m_aItems.size() )
        return mpData->m_aItems[nPos].maContentSize;
    else
        return Size();
}

sal_uInt16 ToolBox::GetItemId(const OUString &rCommand) const
{
    if (!mpData)
        return TOOLBOX_ITEM_NOTFOUND;

    for (std::vector<ImplToolItem>::const_iterator it = mpData->m_aItems.begin(); it != mpData->m_aItems.end(); ++it)
    {
        if (it->maCommandStr == rCommand)
            return it->mnId;
    }

    return 0;
}

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
            case WindowAlign::Top:
                aPos = rRect.BottomLeft();
                aPos.Y()++;
                devPos = OutputToAbsoluteScreenPixel( aPos );
                if( devPos.Y() + rSize.Height() >= aScreen.Bottom() )
                    aPos.Y() = rRect.Top() - rSize.Height();
                break;
            case WindowAlign::Bottom:
                aPos = rRect.TopLeft();
                aPos.Y()--;
                devPos = OutputToAbsoluteScreenPixel( aPos );
                if( devPos.Y() - rSize.Height() > aScreen.Top() )
                    aPos.Y() -= rSize.Height();
                else
                    aPos.Y() = rRect.Bottom();
                break;
            case WindowAlign::Left:
                aPos = rRect.TopRight();
                aPos.X()++;
                devPos = OutputToAbsoluteScreenPixel( aPos );
                if( devPos.X() + rSize.Width() >= aScreen.Right() )
                    aPos.X() = rRect.Left() - rSize.Width();
                break;
            case WindowAlign::Right:
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
        }
    }
    return aPos;
}

Rectangle ToolBox::GetItemRect( sal_uInt16 nItemId ) const
{
    if ( mbCalc || mbFormat )
        const_cast<ToolBox*>(this)->ImplFormat();

    sal_uInt16 nPos = GetItemPos( nItemId );
    return GetItemPosRect( nPos );
}

Rectangle ToolBox::GetItemPosRect( sal_uInt16 nPos ) const
{
    if ( mbCalc || mbFormat )
        const_cast<ToolBox*>(this)->ImplFormat();

    if ( nPos < mpData->m_aItems.size() )
        return mpData->m_aItems[nPos].maRect;
    else
        return Rectangle();
}

bool ToolBox::ImplHasExternalMenubutton()
{
    // check if the borderwindow (i.e. the decoration) provides the menu button
    bool bRet = false;
    if( ImplIsFloatingMode() )
    {
        // custom menu is placed in the decoration
        ImplBorderWindow *pBorderWin = dynamic_cast<ImplBorderWindow*>( GetWindow( GetWindowType::Border ) );
        if( pBorderWin && !pBorderWin->GetMenuRect().IsEmpty() )
            bRet = true;
    }
    return bRet;
}

void ToolBox::SetItemBits( sal_uInt16 nItemId, ToolBoxItemBits nBits )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos < mpData->m_aItems.size() )
    {
        ToolBoxItemBits nOldBits = mpData->m_aItems[nPos].mnBits;
        mpData->m_aItems[nPos].mnBits = nBits;
        nBits &= ToolBoxItemBits::LEFT | ToolBoxItemBits::AUTOSIZE | ToolBoxItemBits::DROPDOWN;
        nOldBits &= ToolBoxItemBits::LEFT | ToolBoxItemBits::AUTOSIZE | ToolBoxItemBits::DROPDOWN;
        // trigger reformat when the item width has changed (dropdown arrow)
        bool bFormat = ToolBoxItemBits(nBits & ToolBoxItemBits::DROPDOWN) != ToolBoxItemBits(nOldBits & ToolBoxItemBits::DROPDOWN);
        if ( nBits != nOldBits )
            ImplInvalidate( true, bFormat );
    }
}

ToolBoxItemBits ToolBox::GetItemBits( sal_uInt16 nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mnBits;
    else
        return ToolBoxItemBits::NONE;
}

void ToolBox::SetItemExpand( sal_uInt16 nItemId, bool bExpand )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );
    if (!pItem)
        return;

    if (pItem->mbExpand != bExpand)
    {
        pItem->mbExpand = bExpand;
        ImplInvalidate(true, true);
    }
}

void ToolBox::SetItemData( sal_uInt16 nItemId, void* pNewData )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos < mpData->m_aItems.size() )
    {
        mpData->m_aItems[nPos].mpUserData = pNewData;
        ImplUpdateItem( nPos );
    }
}

void* ToolBox::GetItemData( sal_uInt16 nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mpUserData;
    else
        return nullptr;
}

void ToolBox::SetItemImage( sal_uInt16 nItemId, const Image& rImage )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        Size aOldSize = pItem->maImage.GetSizePixel();

        pItem->maImage = rImage;

        // only once all is calculated, do extra work
        if (!mbCalc)
        {
            if (aOldSize != pItem->maImage.GetSizePixel())
                ImplInvalidate( true );
            else
                ImplUpdateItem( nPos );
        }
    }
}

static Image ImplRotImage( const Image& rImage, long nAngle10 )
{
    BitmapEx    aRotBitmapEx( rImage.GetBitmapEx() );

    aRotBitmapEx.Rotate( nAngle10, Color( COL_WHITE ) );

    return Image( aRotBitmapEx );
}

void ToolBox::SetItemImageAngle( sal_uInt16 nItemId, long nAngle10 )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

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
        }

        if (!mbCalc)
        {
            if (aOldSize != pItem->maImage.GetSizePixel())
                ImplInvalidate(true);
            else
                ImplUpdateItem(nPos);
        }
    }
}

static Image ImplMirrorImage( const Image& rImage )
{
    BitmapEx    aMirrBitmapEx( rImage.GetBitmapEx() );

    aMirrBitmapEx.Mirror( BmpMirrorFlags::Horizontal );

    return Image( aMirrBitmapEx );
}

void ToolBox::SetItemImageMirrorMode( sal_uInt16 nItemId, bool bMirror )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];

        if ((pItem->mbMirrorMode && !bMirror) ||
            (!pItem->mbMirrorMode && bMirror))
        {
            pItem->mbMirrorMode = bMirror;
            if (!!pItem->maImage)
            {
                pItem->maImage = ImplMirrorImage(pItem->maImage);
            }

            if (!mbCalc)
                ImplUpdateItem(nPos);
        }
    }
}

Image ToolBox::GetItemImage(sal_uInt16 nItemId) const
{
    ImplToolItem* pItem = ImplGetItem(nItemId);
    return pItem ? pItem->maImage : Image();
}

void ToolBox::SetItemText( sal_uInt16 nItemId, const OUString& rText )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        // only once all is calculated, do extra work
        if ( !mbCalc &&
             ((meButtonType != ButtonType::SYMBOLONLY) || !pItem->maImage) )
        {
            long nOldWidth = GetCtrlTextWidth( pItem->maText );
            pItem->maText = MnemonicGenerator::EraseAllMnemonicChars(rText);
            mpData->ImplClearLayoutData();
            if ( nOldWidth != GetCtrlTextWidth( pItem->maText ) )
                ImplInvalidate( true );
            else
                ImplUpdateItem( nPos );
        }
        else
            pItem->maText = MnemonicGenerator::EraseAllMnemonicChars(rText);

        // Notify button changed event to prepare accessibility bridge
        CallEventListeners( VclEventId::ToolboxButtonStateChanged, reinterpret_cast< void* >( nPos ) );

        // Notify
        CallEventListeners( VclEventId::ToolboxItemTextChanged, reinterpret_cast< void* >( nPos ) );
    }
}

const OUString& ToolBox::GetItemText( sal_uInt16 nItemId ) const
{

    ImplToolItem* pItem = ImplGetItem( nItemId );

    assert( pItem );

    return pItem->maText;
}

void ToolBox::SetItemWindow( sal_uInt16 nItemId, vcl::Window* pNewWindow )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        pItem->mpWindow = pNewWindow;
        if ( pNewWindow )
            pNewWindow->Hide();
        ImplInvalidate( true );
        CallEventListeners( VclEventId::ToolboxItemWindowChanged, reinterpret_cast< void* >( nPos ) );
    }
}

vcl::Window* ToolBox::GetItemWindow( sal_uInt16 nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mpWindow;
    else
        return nullptr;
}

void ToolBox::StartSelection()
{
    if ( mbDrag )
        EndSelection();

    if ( !mbSelection )
    {
        mbSelection  = true;
        mnCurPos     = TOOLBOX_ITEM_NOTFOUND;
        mnCurItemId  = 0;
        Activate();
    }
}

void ToolBox::EndSelection()
{
    mbCommandDrag = false;

    if ( mbDrag || mbSelection )
    {
        // reset
        mbDrag = false;
        mbSelection = false;
        if (mnCurPos != TOOLBOX_ITEM_NOTFOUND)
            InvalidateItem(mnCurPos);
        EndTracking();
        if (IsMouseCaptured())
            ReleaseMouse();
        Deactivate();
    }

    mnCurPos        = TOOLBOX_ITEM_NOTFOUND;
    mnCurItemId     = 0;
    mnDownItemId    = 0;
    mnMouseClicks   = 0;
    mnMouseModifier = 0;
}

void ToolBox::SetItemDown( sal_uInt16 nItemId, bool bDown )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        if ( bDown )
        {
            if ( nPos != mnCurPos )
            {
                mnCurPos = nPos;
                InvalidateItem(mnCurPos);
                Flush();
            }
        }
        else
        {
            if ( nPos == mnCurPos )
            {
                InvalidateItem(mnCurPos);
                Flush();
                mnCurPos = TOOLBOX_ITEM_NOTFOUND;
            }
        }

        if ( mbDrag || mbSelection )
        {
            mbDrag = false;
            mbSelection = false;
            EndTracking();
            if (IsMouseCaptured())
                ReleaseMouse();
            Deactivate();
        }

        mnCurItemId     = 0;
        mnDownItemId    = 0;
        mnMouseClicks   = 0;
        mnMouseModifier = 0;
    }
}

void ToolBox::SetItemState( sal_uInt16 nItemId, TriState eState )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];

        // the state has changed
        if ( pItem->meState != eState )
        {
            // if RadioCheck, un-check the previous
            if ( (eState == TRISTATE_TRUE) && (pItem->mnBits & ToolBoxItemBits::AUTOCHECK) &&
                 (pItem->mnBits & ToolBoxItemBits::RADIOCHECK) )
            {
                ImplToolItem*    pGroupItem;
                sal_uInt16          nGroupPos;
                sal_uInt16          nItemCount = GetItemCount();

                nGroupPos = nPos;
                while ( nGroupPos )
                {
                    pGroupItem = &mpData->m_aItems[nGroupPos-1];
                    if ( pGroupItem->mnBits & ToolBoxItemBits::RADIOCHECK )
                    {
                        if ( pGroupItem->meState != TRISTATE_FALSE )
                            SetItemState( pGroupItem->mnId, TRISTATE_FALSE );
                    }
                    else
                        break;
                    nGroupPos--;
                }

                nGroupPos = nPos+1;
                while ( nGroupPos < nItemCount )
                {
                    pGroupItem = &mpData->m_aItems[nGroupPos];
                    if ( pGroupItem->mnBits & ToolBoxItemBits::RADIOCHECK )
                    {
                        if ( pGroupItem->meState != TRISTATE_FALSE )
                            SetItemState( pGroupItem->mnId, TRISTATE_FALSE );
                    }
                    else
                        break;
                    nGroupPos++;
                }
            }

            pItem->meState = eState;
            ImplUpdateItem( nPos );

            // Notify button changed event to prepare accessibility bridge
            CallEventListeners( VclEventId::ToolboxButtonStateChanged, reinterpret_cast< void* >( nPos ) );

            // Call accessible listener to notify state_changed event
            CallEventListeners( VclEventId::ToolboxItemUpdated, reinterpret_cast< void* >(nPos) );
        }
    }
}

TriState ToolBox::GetItemState( sal_uInt16 nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->meState;
    else
        return TRISTATE_FALSE;
}

void ToolBox::EnableItem( sal_uInt16 nItemId, bool bEnable )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        if ( bEnable )
            bEnable = true;
        if ( pItem->mbEnabled != bEnable )
        {
            pItem->mbEnabled = bEnable;

            // if existing, also redraw the window
            if ( pItem->mpWindow )
                pItem->mpWindow->Enable( pItem->mbEnabled );

            // update item
            ImplUpdateItem( nPos );

            ImplUpdateInputEnable();

            // Notify button changed event to prepare accessibility bridge
            CallEventListeners( VclEventId::ToolboxButtonStateChanged, reinterpret_cast< void* >( nPos ) );

            CallEventListeners( bEnable ? VclEventId::ToolboxItemEnabled : VclEventId::ToolboxItemDisabled, reinterpret_cast< void* >( nPos ) );
        }
    }
}

bool ToolBox::IsItemEnabled( sal_uInt16 nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mbEnabled;
    else
        return false;
}

void ToolBox::ShowItem( sal_uInt16 nItemId, bool bVisible )
{
    sal_uInt16 nPos = GetItemPos( nItemId );
    mpData->ImplClearLayoutData();

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        if ( pItem->mbVisible != bVisible )
        {
            pItem->mbVisible = bVisible;
            ImplInvalidate();
        }
    }
}

bool ToolBox::IsItemVisible( sal_uInt16 nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mbVisible;
    else
        return false;
}

bool ToolBox::IsItemReallyVisible( sal_uInt16 nItemId ) const
{
    // is the item on the visible area of the toolbox?
    bool bRet = false;
    Rectangle aRect( mnLeftBorder, mnTopBorder, mnDX-mnRightBorder, mnDY-mnBottomBorder );
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem && pItem->mbVisible &&
         !pItem->maRect.IsEmpty() && aRect.IsOver( pItem->maRect ) )
    {
        bRet = true;
    }

    return bRet;
}

void ToolBox::SetItemCommand(sal_uInt16 nItemId, const OUString& rCommand)
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if (pItem)
        pItem->maCommandStr = rCommand;
}

const OUString ToolBox::GetItemCommand( sal_uInt16 nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if (pItem)
        return pItem->maCommandStr;

    return OUString();
}

void ToolBox::SetQuickHelpText( sal_uInt16 nItemId, const OUString& rText )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        pItem->maQuickHelpText = rText;
}

OUString ToolBox::GetQuickHelpText( sal_uInt16 nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maQuickHelpText;
    else
        return OUString();
}

void ToolBox::SetHelpText( sal_uInt16 nItemId, const OUString& rText )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        pItem->maHelpText = rText;
}

const OUString& ToolBox::GetHelpText( sal_uInt16 nItemId ) const
{
    return ImplGetHelpText( nItemId );
}

void ToolBox::SetHelpId( sal_uInt16 nItemId, const OString& rHelpId )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        pItem->maHelpId = rHelpId;
}

OString ToolBox::GetHelpId( sal_uInt16 nItemId ) const
{
    OString aRet;

    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
    {
        if ( !pItem->maHelpId.isEmpty() )
            aRet = pItem->maHelpId;
        else
            aRet = OUStringToOString( pItem->maCommandStr, RTL_TEXTENCODING_UTF8 );
    }

    return aRet;
}

void ToolBox::SetOutStyle( sal_uInt16 nNewStyle )
{
    // always force flat looking toolbars since NWF
    nNewStyle |= TOOLBOX_STYLE_FLAT;

    if ( mnOutStyle != nNewStyle )
    {
        mnOutStyle = nNewStyle;
        ImplDisableFlatButtons();

        // so as to redo the ButtonDevice
        if ( !(mnOutStyle & TOOLBOX_STYLE_FLAT) )
        {
            mnMaxItemWidth  = 1;
            mnMaxItemHeight = 1;
        }

        ImplInvalidate( true, true );
    }
}

// disable key input if all items are disabled
void ToolBox::ImplUpdateInputEnable()
{
    for( std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
         it != mpData->m_aItems.end(); ++it )
    {
        if( it->mbEnabled )
        {
            // at least one useful entry
            mpData->mbKeyInputDisabled = false;
            return;
        }
    }
    mpData->mbKeyInputDisabled = true;
}

void ToolBox::ImplFillLayoutData() const
{
    mpData->m_pLayoutData = new ToolBoxLayoutData();

    sal_uInt16 nCount = (sal_uInt16)mpData->m_aItems.size();
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        ImplToolItem* pItem = &mpData->m_aItems[i];

        // only draw, if the rectangle is within PaintRectangle
        if (!pItem->maRect.IsEmpty())
            const_cast<ToolBox*>(this)->InvalidateItem(i);
    }
}

OUString ToolBox::GetDisplayText() const
{
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    return mpData->m_pLayoutData ? OUString(mpData->m_pLayoutData->m_aDisplayText) : OUString();
}

Rectangle ToolBox::GetCharacterBounds( sal_uInt16 nItemID, long nIndex ) const
{
    long nItemIndex = -1;
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    if( mpData->m_pLayoutData )
    {
        for( sal_uLong i = 0; i < mpData->m_pLayoutData->m_aLineItemIds.size(); i++ )
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

long ToolBox::GetIndexForPoint( const Point& rPoint, sal_uInt16& rItemID ) const
{
    long nIndex = -1;
    rItemID = 0;
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    if( mpData->m_pLayoutData )
    {
        nIndex = mpData->m_pLayoutData->GetIndexForPoint( rPoint );
        for( sal_uLong i = 0; i < mpData->m_pLayoutData->m_aLineIndices.size(); i++ )
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

void ToolBox::SetDropdownClickHdl( const Link<ToolBox *, void>& rLink )
{
    if (mpData != nullptr) {
        mpData->maDropdownClickHdl = rLink;
    }
}

void ToolBox::SetMenuType( ToolBoxMenuType aType )
{
    if( aType != mpData->maMenuType )
    {
        mpData->maMenuType = aType;
        if( IsFloatingMode() )
        {
            // the menu button may have to be moved into the decoration which changes the layout
            ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
            if( pWrapper )
                pWrapper->ShowTitleButton( TitleButton::Menu, bool( aType & ToolBoxMenuType::Customize) );

            mbFormat = true;
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

ToolBoxMenuType ToolBox::GetMenuType() const
{
    return mpData->maMenuType;
}

bool ToolBox::IsMenuEnabled() const
{
    return mpData->maMenuType != ToolBoxMenuType::NONE;
}

PopupMenu* ToolBox::GetMenu() const
{
    return mpData == nullptr ? nullptr : mpData->mpMenu;
}

void ToolBox::SetMenuButtonHdl( const Link<ToolBox *, void>& rLink )
{
    mpData->maMenuButtonHdl = rLink;
}

bool ToolBox::ImplHasClippedItems()
{
    // are any items currently clipped ?
    ImplFormat();
    std::vector< ImplToolItem >::const_iterator it = mpData->m_aItems.begin();
    while ( it != mpData->m_aItems.end() )
    {
        if( it->IsClipped() )
            return true;
        ++it;
    }
    return false;
}

namespace
{
    MenuItemBits ConvertBitsFromToolBoxToMenu(ToolBoxItemBits nToolItemBits)
    {
        MenuItemBits nMenuItemBits = MenuItemBits::NONE;
        if ((nToolItemBits & ToolBoxItemBits::CHECKABLE) ||
            (nToolItemBits & ToolBoxItemBits::DROPDOWN))
        {
            nMenuItemBits |= MenuItemBits::CHECKABLE;
        }
        return nMenuItemBits;
    }
}

void ToolBox::UpdateCustomMenu()
{
    // fill clipped items into menu
    if( !IsMenuEnabled() )
        return;

    PopupMenu *pMenu = GetMenu();

    sal_uInt16 i = 0;
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

    // add menu items: first the overflow items, then hidden items, both in the
    // order they would usually appear in the toolbar. Separators that would be
    // in the toolbar are ignored as they would introduce too much clutter,
    // instead we have a single separator to help distinguish between overflow
    // and hidden items.
    if ( !mpData->m_aItems.empty() )
    {
        // nStartPos will hold the number of clipped items appended from first loop
        for ( std::vector< ImplToolItem >::iterator it(mpData->m_aItems.begin());
                it != mpData->m_aItems.end(); ++it)
        {
            if( it->IsClipped() )
            {
                sal_uInt16 id = it->mnId + TOOLBOX_MENUITEM_START;
                MenuItemBits nMenuItemBits = ConvertBitsFromToolBoxToMenu(it->mnBits);
                pMenu->InsertItem( id, it->maText, it->maImage, nMenuItemBits);
                pMenu->SetItemCommand( id, it->maCommandStr );
                pMenu->EnableItem( id, it->mbEnabled );
                pMenu->CheckItem ( id, it->meState == TRISTATE_TRUE );
            }
        }

        // add a separator below the inserted clipped-items
        pMenu->InsertSeparator();

        // now append the items that are explicitly disabled
        for ( std::vector< ImplToolItem >::iterator it(mpData->m_aItems.begin());
                it != mpData->m_aItems.end(); ++it)
        {
            if( it->IsItemHidden() )
            {
                sal_uInt16 id = it->mnId + TOOLBOX_MENUITEM_START;
                MenuItemBits nMenuItemBits = ConvertBitsFromToolBoxToMenu(it->mnBits);
                pMenu->InsertItem( id, it->maText, it->maImage, nMenuItemBits );
                pMenu->SetItemCommand( id, it->maCommandStr );
                pMenu->EnableItem( id, it->mbEnabled );
                pMenu->CheckItem( id, it->meState == TRISTATE_TRUE );
            }
        }

    }
}

IMPL_LINK( ToolBox, ImplCustomMenuListener, VclMenuEvent&, rEvent, void )
{
    if( rEvent.GetMenu() == GetMenu() && rEvent.GetId() == VclEventId::MenuSelect )
    {
        sal_uInt16 id = GetMenu()->GetItemId( rEvent.GetItemPos() );
        if( id >= TOOLBOX_MENUITEM_START )
            TriggerItem( id - TOOLBOX_MENUITEM_START );
    }
}

IMPL_LINK_NOARG(ToolBox, ImplCallExecuteCustomMenu, void*, void)
{
    mpData->mnEventId = nullptr;
    if( IsMenuEnabled() )
    {
        if( GetMenuType() & ToolBoxMenuType::Customize )
            // call button handler to allow for menu customization
            mpData->maMenuButtonHdl.Call( this );

        // We specifically only register this event listener when executing our
        // overflow menu (and remove it directly afterwards), as the same menu
        // is reused for both the overflow menu (as managed here in ToolBox),
        // but also by ToolBarManager for its context menu. If we leave event
        // listeners alive beyond when the menu is showing in the desired mode
        // then duplicate events can happen as the context menu "duplicates"
        // items from the overflow menu, which both listeners would then act on.
        GetMenu()->AddEventListener( LINK( this, ToolBox, ImplCustomMenuListener ) );

        // make sure all disabled entries will be shown
        GetMenu()->SetMenuFlags(
            GetMenu()->GetMenuFlags() | MenuFlags::AlwaysShowDisabledEntries );

        // toolbox might be destroyed during execute
        bool bBorderDel = false;

        VclPtr<vcl::Window> pWin = this;
        Rectangle aMenuRect = mpData->maMenubuttonItem.maRect;
        VclPtr<ImplBorderWindow> pBorderWin;
        if( IsFloatingMode() )
        {
            // custom menu is placed in the decoration
            pBorderWin = dynamic_cast<ImplBorderWindow*>( GetWindow( GetWindowType::Border ) );
            if( pBorderWin && !pBorderWin->GetMenuRect().IsEmpty() )
            {
                pWin = pBorderWin;
                aMenuRect = pBorderWin->GetMenuRect();
                bBorderDel = true;
            }
        }

        sal_uInt16 uId = GetMenu()->Execute( pWin, Rectangle( ImplGetPopupPosition( aMenuRect, Size() ), Size() ),
                                PopupMenuFlags::ExecuteDown | PopupMenuFlags::NoMouseUpClose );

        if ( pWin->IsDisposed() )
            return;

        if( GetMenu() )
            GetMenu()->RemoveEventListener( LINK( this, ToolBox, ImplCustomMenuListener ) );
        if( bBorderDel )
        {
            if( pBorderWin->IsDisposed() )
                return;
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
        UpdateCustomMenu();
        mpData->mnEventId = Application::PostUserEvent( LINK( this, ToolBox, ImplCallExecuteCustomMenu ), nullptr, true );
    }
}

// checks override first, useful during calculation of sizes
bool ToolBox::ImplIsFloatingMode() const
{
    SAL_WARN_IF( mpData->mbAssumeDocked && mpData->mbAssumeFloating, "vcl",
        "cannot assume docked and floating" );

    if( mpData->mbAssumeDocked )
        return false;
    else if( mpData->mbAssumeFloating )
        return true;
    else
        return IsFloatingMode();
}

// checks override first, useful during calculation of sizes
bool ToolBox::ImplIsInPopupMode() const
{
    if( mpData->mbAssumePopupMode )
        return true;
    else
    {
        ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
        return ( pWrapper && pWrapper->GetFloatingWindow() && pWrapper->GetFloatingWindow()->IsInPopupMode() );
    }
}

void ToolBox::Lock( bool bLock )
{
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( !pWrapper )
        return;
    if( mpData->mbIsLocked != bLock )
    {
        mpData->mbIsLocked = bLock;
        if( !ImplIsFloatingMode() )
        {
            mbCalc = true;
            mbFormat = true;
            SetSizePixel( CalcWindowSizePixel(1) );
            Invalidate();
        }
    }
}

bool ToolBox::AlwaysLocked()
{
    // read config item to determine toolbox behaviour, used for subtoolbars

    static int nAlwaysLocked = -1;

    if( nAlwaysLocked == -1 )
    {
        nAlwaysLocked = 0; // ask configuration only once

        utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithComponentContext(
            comphelper::getProcessComponentContext(),
            "/org.openoffice.Office.UI.GlobalSettings/Toolbars" );    // note: case sensitive !
        if ( aNode.isValid() )
        {
            // feature enabled ?
            bool bStatesEnabled = bool();
            css::uno::Any aValue = aNode.getNodeValue( OUString("StatesEnabled") );
            if( aValue >>= bStatesEnabled )
            {
                if( bStatesEnabled )
                {
                    // now read the locking state
                    utl::OConfigurationNode aNode2 = utl::OConfigurationTreeRoot::tryCreateWithComponentContext(
                        comphelper::getProcessComponentContext(),
                        "/org.openoffice.Office.UI.GlobalSettings/Toolbars/States" );    // note: case sensitive !

                    bool bLocked = bool();
                    css::uno::Any aValue2 = aNode2.getNodeValue( OUString("Locked") );
                    if( aValue2 >>= bLocked )
                        nAlwaysLocked = bLocked ? 1 : 0;
                }
            }
        }
    }

    return nAlwaysLocked == 1;
}

bool ToolBox::WillUsePopupMode() const
{
    return mpData->mbWillUsePopupMode;
}

void ToolBox::WillUsePopupMode( bool b )
{
    mpData->mbWillUsePopupMode = b;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
