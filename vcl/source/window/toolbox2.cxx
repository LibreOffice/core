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

#include <sal/config.h>
#include <utility>
#include <vcl/uitest/logger.hxx>
#include <sal/log.hxx>

#include <comphelper/base64.hxx>
#include <comphelper/processfactory.hxx>
#include <boost/property_tree/ptree.hpp>

#include <vcl/cvtgrf.hxx>
#include <vcl/svapp.hxx>
#include <vcl/idle.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/toolkit/floatwin.hxx>
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
#include <tools/json_writer.hxx>

#include <vcl/uitest/uiobject.hxx>

#include "impldockingwrapper.hxx"

using namespace vcl;

#define TB_SEP_SIZE     8  // Separator size


ImplToolBoxPrivateData::ImplToolBoxPrivateData()
{
    meButtonSize = ToolBoxButtonSize::DontCare;
    mpMenu = VclPtr<PopupMenu>::Create();

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
    mbMenubuttonWasLastSelected = false;
    mbWillUsePopupMode = false;
    mbDropDownByKeyboard = false;
}

ImplToolBoxPrivateData::~ImplToolBoxPrivateData()
{
    m_pLayoutData.reset();
    mpMenu.disposeAndClear();
}

void ImplToolItem::init(ToolBoxItemId nItemId, ToolBoxItemBits nItemBits,
                        bool bEmptyBtn)
{
    mnId            = nItemId;
    mpWindow        = nullptr;
    mbNonInteractiveWindow = false;
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
    mnImageAngle    = 0_deg10;
    mbMirrorMode    = false;
    mbVisibleText   = false;
    mbExpand        = false;
}

ImplToolItem::ImplToolItem()
{
    init(ToolBoxItemId(0), ToolBoxItemBits::NONE, true);
}

ImplToolItem::ImplToolItem( ToolBoxItemId nItemId, Image aImage,
                            ToolBoxItemBits nItemBits ) :
    maImage(std::move( aImage ))
{
    init(nItemId, nItemBits, false);
}

ImplToolItem::ImplToolItem( ToolBoxItemId nItemId, OUString aText,
                            OUString aCommand, ToolBoxItemBits nItemBits ) :
    maText(std::move( aText )),
    maCommandStr(std::move( aCommand ))
{
    init(nItemId, nItemBits, false);
}

ImplToolItem::ImplToolItem( ToolBoxItemId nItemId, Image aImage,
                            OUString aText, ToolBoxItemBits nItemBits ) :
    maImage(std::move( aImage )),
    maText(std::move( aText ))
{
    init(nItemId, nItemBits, false);
}

Size ImplToolItem::GetSize( bool bHorz, bool bCheckMaxWidth, tools::Long maxWidth, const Size& rDefaultSize )
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
                aSize.setWidth( aWinSize.Width() );
                aSize.setHeight( aWinSize.Height() );
                mbShowWindow = true;
            }
            else
            {
                if ( mbEmptyBtn )
                {
                    aSize.setWidth( 0 );
                    aSize.setHeight( 0 );
                }
            }
        }
    }
    else if ( meType == ToolBoxItemType::SEPARATOR )
    {
        if ( bHorz )
        {
            aSize.setWidth( mnSepSize );
            aSize.setHeight( rDefaultSize.Height() );
        }
        else
        {
            aSize.setWidth( rDefaultSize.Width() );
            aSize.setHeight( mnSepSize );
        }
    }
    else if ( meType == ToolBoxItemType::BREAK )
    {
        aSize.setWidth( 0 );
        aSize.setHeight( 0 );
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
    bHasImage = !!maImage;
    bHasText = !maText.isEmpty();

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

tools::Rectangle ImplToolItem::GetDropDownRect( bool bHorz ) const
{
    tools::Rectangle aRect;
    if( (mnBits & ToolBoxItemBits::DROPDOWN) && !maRect.IsEmpty() )
    {
        aRect = maRect;
        if( mbVisibleText && !bHorz )
            // item will be rotated -> place dropdown to the bottom
            aRect.SetTop( aRect.Bottom() - mnDropDownArrowWidth );
        else
            // place dropdown to the right
            aRect.SetLeft( aRect.Right() - mnDropDownArrowWidth );
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
            Invalidate( tools::Rectangle( mnLeftBorder, mnTopBorder,
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

void ToolBox::ImplUpdateItem( ImplToolItems::size_type nIndex )
{
    // do we need to redraw?
    if ( !(IsReallyVisible() && IsUpdateMode()) )
        return;

    if ( nIndex == ITEM_NOTFOUND )
    {
        // #i52217# no immediate draw as this might lead to paint problems
        Invalidate( tools::Rectangle( mnLeftBorder, mnTopBorder, mnDX-mnRightBorder-1, mnDY-mnBottomBorder-1 ) );
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

void ToolBox::Click()
{
    CallEventListeners( VclEventId::ToolboxClick );
    maClickHdl.Call( this );
    UITestLogger::getInstance().logAction( this, VclEventId::ToolboxClick);
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

FactoryFunction ToolBox::GetUITestFactory() const
{
    return ToolBoxUIObject::create;
}

void ToolBox::Select()
{
    VclPtr<vcl::Window> xWindow = this;

    CallEventListeners( VclEventId::ToolboxSelect );
    maSelectHdl.Call( this );

    if ( xWindow->isDisposed() )
        return;

    // TODO: GetFloatingWindow in DockingWindow is currently inline, change it to check dockingwrapper
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper && pWrapper->GetFloatingWindow() && static_cast<FloatingWindow*>(pWrapper->GetFloatingWindow())->IsInPopupMode() )
        static_cast<FloatingWindow*>(pWrapper->GetFloatingWindow())->EndPopupMode();
}

void ToolBox::InsertItem( ToolBoxItemId nItemId, const Image& rImage, ToolBoxItemBits nBits, ImplToolItems::size_type nPos )
{
    SAL_WARN_IF( !nItemId, "vcl", "ToolBox::InsertItem(): ItemId == 0" );
    SAL_WARN_IF( GetItemPos( nItemId ) != ITEM_NOTFOUND, "vcl",
                "ToolBox::InsertItem(): ItemId already exists" );

    // create item and add to list
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(),
                             ImplToolItem( nItemId, rImage, nBits ) );
    mpData->ImplClearLayoutData();

    ImplInvalidate( true );

    // Notify
    ImplToolItems::size_type nNewPos = ( nPos == APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos;
    CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >(nNewPos ) );
}

void ToolBox::InsertItem( ToolBoxItemId nItemId, const Image& rImage, const OUString& rText, ToolBoxItemBits nBits,
                          ImplToolItems::size_type nPos )
{
    SAL_WARN_IF( !nItemId, "vcl", "ToolBox::InsertItem(): ItemId == 0" );
    SAL_WARN_IF( GetItemPos( nItemId ) != ITEM_NOTFOUND, "vcl",
                "ToolBox::InsertItem(): ItemId already exists" );

    // create item and add to list
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(),
                             ImplToolItem( nItemId, rImage, MnemonicGenerator::EraseAllMnemonicChars(rText), nBits ) );
    mpData->ImplClearLayoutData();

    ImplInvalidate( true );

    // Notify
    ImplToolItems::size_type nNewPos = ( nPos == APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos;
    CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::InsertItem( ToolBoxItemId nItemId, const OUString& rText, const OUString& rCommand, ToolBoxItemBits nBits,
                          ImplToolItems::size_type nPos )
{
    SAL_WARN_IF( !nItemId, "vcl", "ToolBox::InsertItem(): ItemId == 0" );
    SAL_WARN_IF( GetItemPos( nItemId ) != ITEM_NOTFOUND, "vcl",
                "ToolBox::InsertItem(): ItemId already exists" );

    // create item and add to list
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(),
                             ImplToolItem( nItemId, MnemonicGenerator::EraseAllMnemonicChars(rText), rCommand, nBits ) );
    mpData->ImplClearLayoutData();

    ImplInvalidate( true );

    // Notify
    ImplToolItems::size_type nNewPos = ( nPos == APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos;
    CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::InsertItem(const OUString& rCommand, const css::uno::Reference<css::frame::XFrame>& rFrame, ToolBoxItemBits nBits,
                         const Size& rRequestedSize, ImplToolItems::size_type nPos)
{
    OUString aModuleName(vcl::CommandInfoProvider::GetModuleIdentifier(rFrame));
    auto aProperties = vcl::CommandInfoProvider::GetCommandProperties(rCommand, aModuleName);
    OUString aLabel(vcl::CommandInfoProvider::GetLabelForCommand(aProperties));
    OUString aTooltip(vcl::CommandInfoProvider::GetTooltipForCommand(rCommand, aProperties, rFrame));
    Image aImage(CommandInfoProvider::GetImageForCommand(rCommand, rFrame, GetImageSize()));

    ToolBoxItemId nItemId(GetItemCount() + 1);
        //TODO: ImplToolItems::size_type -> sal_uInt16!
    InsertItem(nItemId, aLabel, rCommand, nBits, nPos);
    SetItemImage(nItemId, aImage);
    SetQuickHelpText(nItemId, aTooltip);

    // set the minimal size
    ImplToolItem* pItem = ImplGetItem( nItemId );
    if ( pItem )
        pItem->maMinimalItemSize = rRequestedSize;
}

void ToolBox::InsertWindow( ToolBoxItemId nItemId, vcl::Window* pWindow,
                            ToolBoxItemBits nBits, ImplToolItems::size_type nPos )
{
    SAL_WARN_IF( !nItemId, "vcl", "ToolBox::InsertWindow(): ItemId == 0" );
    SAL_WARN_IF( GetItemPos( nItemId ) != ITEM_NOTFOUND, "vcl",
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
    ImplToolItems::size_type nNewPos = ( nPos == APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos;
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
    ImplToolItems::size_type nNewPos = mpData->m_aItems.size() - 1;
    CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::InsertSeparator( ImplToolItems::size_type nPos, sal_uInt16 nPixSize )
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
    ImplToolItems::size_type nNewPos = ( nPos == APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos;
    CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::InsertBreak( ImplToolItems::size_type nPos )
{
    // create item and add to list
    ImplToolItem aItem;
    aItem.meType     = ToolBoxItemType::BREAK;
    aItem.mbEnabled  = false;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    ImplInvalidate();

    // Notify
    ImplToolItems::size_type nNewPos = ( nPos == APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos;
    CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::RemoveItem( ImplToolItems::size_type nPos )
{
    if( nPos >= mpData->m_aItems.size() )
        return;

    bool bMustCalc;
    bMustCalc = mpData->m_aItems[nPos].meType == ToolBoxItemType::BUTTON;

    if ( mpData->m_aItems[nPos].mpWindow )
        mpData->m_aItems[nPos].mpWindow->Hide();

    // add the removed item to PaintRect
    maPaintRect.Union( mpData->m_aItems[nPos].maRect );

    // ensure not to delete in the Select-Handler
    if ( mpData->m_aItems[nPos].mnId == mnCurItemId )
        mnCurItemId = ToolBoxItemId(0);
    if ( mpData->m_aItems[nPos].mnId == mnHighItemId )
        mnHighItemId = ToolBoxItemId(0);

    ImplInvalidate( bMustCalc );

    mpData->m_aItems.erase( mpData->m_aItems.begin()+nPos );
    mpData->ImplClearLayoutData();

    // Notify
    CallEventListeners( VclEventId::ToolboxItemRemoved, reinterpret_cast< void* >( nPos ) );
}

void ToolBox::CopyItem( const ToolBox& rToolBox, ToolBoxItemId nItemId )
{
    SAL_WARN_IF( GetItemPos( nItemId ) != ITEM_NOTFOUND, "vcl",
                "ToolBox::CopyItem(): ItemId already exists" );

    ImplToolItems::size_type nPos = rToolBox.GetItemPos( nItemId );

    // found item
    if ( nPos == ITEM_NOTFOUND )
        return;

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
    ImplToolItems::size_type nNewPos2 = mpData->m_aItems.size() - 1;
    CallEventListeners( VclEventId::ToolboxItemAdded, reinterpret_cast< void* >( nNewPos2 ) );
}

void ToolBox::Clear()
{
    mpData->m_aItems.clear();
    mpData->ImplClearLayoutData();

    // ensure not to delete in the Select-Handler
    mnCurItemId = ToolBoxItemId(0);
    mnHighItemId = ToolBoxItemId(0);

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

ImageType ToolBox::GetImageSize() const
{
    ImageType eImageType = ImageType::Size16;
    if (mpData->meButtonSize == ToolBoxButtonSize::Large)
        eImageType = ImageType::Size26;
    else if (mpData->meButtonSize == ToolBoxButtonSize::Size32)
        eImageType = ImageType::Size32;

    return eImageType;
}

/*static*/ Size ToolBox::GetDefaultImageSize(ToolBoxButtonSize eToolBoxButtonSize)
{
    OutputDevice *pDefault = Application::GetDefaultDevice();
    float fScaleFactor = pDefault ? pDefault->GetDPIScaleFactor() : 1.0;

    Size aUnscaledSize(16, 16);

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
    if ( meAlign == eNewAlign )
        return;

    meAlign = eNewAlign;

    if ( ImplIsFloatingMode() )
        return;

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

void ToolBox::SetLineCount( ImplToolItems::size_type nNewLines )
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

ToolBox::ImplToolItems::size_type ToolBox::GetItemCount() const
{
    return mpData ? mpData->m_aItems.size() : 0;
}

ToolBoxItemType ToolBox::GetItemType( ImplToolItems::size_type nPos ) const
{
    return (nPos < mpData->m_aItems.size()) ? mpData->m_aItems[nPos].meType : ToolBoxItemType::DONTKNOW;
}

ToolBox::ImplToolItems::size_type ToolBox::GetItemPos( ToolBoxItemId nItemId ) const
{
    if (mpData)
    {
        ImplToolItems::size_type nCount = mpData->m_aItems.size();
        for( ImplToolItems::size_type nPos = 0; nPos < nCount; nPos++ )
            if( mpData->m_aItems[nPos].mnId == nItemId )
                return nPos;
    }
    return ITEM_NOTFOUND;
}

ToolBox::ImplToolItems::size_type ToolBox::GetItemPos( const Point& rPos ) const
{
    // search the item position on the given point
    auto it = std::find_if(mpData->m_aItems.begin(), mpData->m_aItems.end(),
        [&rPos](const ImplToolItem& rItem) { return rItem.maRect.Contains( rPos ); });

    if( it != mpData->m_aItems.end() )
        return std::distance(mpData->m_aItems.begin(), it);

    return ITEM_NOTFOUND;
}

ToolBoxItemId ToolBox::GetItemId( ImplToolItems::size_type nPos ) const
{
    return (nPos < mpData->m_aItems.size()) ? mpData->m_aItems[nPos].mnId : ToolBoxItemId(0);
}

ToolBoxItemId ToolBox::GetItemId( const Point& rPos ) const
{
    // find item that was clicked
    auto it = std::find_if(mpData->m_aItems.begin(), mpData->m_aItems.end(),
        [&rPos](const ImplToolItem& rItem) { return rItem.maRect.Contains( rPos ); });

    if( (it != mpData->m_aItems.end()) && (it->meType == ToolBoxItemType::BUTTON) )
        return it->mnId;

    return ToolBoxItemId(0);
}

Size ToolBox::GetItemContentSize( ToolBoxItemId nItemId )
{
    if ( mbCalc || mbFormat )
        ImplFormat();

    ImplToolItems::size_type nPos = GetItemPos( nItemId );
    if ( nPos < mpData->m_aItems.size() )
        return mpData->m_aItems[nPos].maContentSize;
    else
        return Size();
}

ToolBoxItemId ToolBox::GetItemId(const OUString &rCommand) const
{
    if (!mpData)
        return ToolBoxItemId(0);

    auto it = std::find_if(mpData->m_aItems.begin(), mpData->m_aItems.end(),
        [&rCommand](const ImplToolItem& rItem) { return rItem.maCommandStr == rCommand; });
    if (it != mpData->m_aItems.end())
        return it->mnId;

    return ToolBoxItemId(0);
}

Point ToolBox::ImplGetPopupPosition( const tools::Rectangle& rRect ) const
{
    Point aPos;
    if( !rRect.IsEmpty() )
    {
        AbsoluteScreenPixelRectangle aScreen = GetDesktopRectPixel();

        // the popup should be positioned so that it will not cover
        // the item rect and that it fits the desktop
        // the preferred direction is always towards the center of
        // the application window

        AbsoluteScreenPixelPoint devPos; // the position in device coordinates for screen comparison
        switch( meAlign )
        {
            case WindowAlign::Top:
                aPos = rRect.BottomLeft();
                aPos.AdjustY( 1 );
                devPos = OutputToAbsoluteScreenPixel( aPos );
                if( devPos.Y() >= aScreen.Bottom() )
                    aPos.setY( rRect.Top() );
                break;
            case WindowAlign::Bottom:
                aPos = rRect.TopLeft();
                aPos.AdjustY( -1 );
                devPos = OutputToAbsoluteScreenPixel( aPos );
                if( devPos.Y() <= aScreen.Top() )
                    aPos.setY( rRect.Bottom() );
                break;
            case WindowAlign::Left:
                aPos = rRect.TopRight();
                aPos.AdjustX( 1 );
                devPos = OutputToAbsoluteScreenPixel( aPos );
                if( devPos.X() >= aScreen.Right() )
                    aPos.setX( rRect.Left() );
                break;
            case WindowAlign::Right:
                aPos = rRect.TopLeft();
                aPos.AdjustX( -1 );
                devPos = OutputToAbsoluteScreenPixel( aPos );
                if( devPos.X() <= aScreen.Left() )
                    aPos.setX( rRect.Right() );
                break;
            default:
                break;
        }
    }
    return aPos;
}

tools::Rectangle ToolBox::GetItemRect( ToolBoxItemId nItemId )
{
    if ( mbCalc || mbFormat )
        ImplFormat();

    ImplToolItems::size_type nPos = GetItemPos( nItemId );
    return GetItemPosRect( nPos );
}

tools::Rectangle ToolBox::GetItemPosRect( ImplToolItems::size_type nPos )
{
    if ( mbCalc || mbFormat )
        ImplFormat();

    if ( nPos < mpData->m_aItems.size() )
        return mpData->m_aItems[nPos].maRect;
    else
        return tools::Rectangle();
}

tools::Rectangle const & ToolBox::GetOverflowRect() const
{
    return mpData->maMenubuttonItem.maRect;
}

bool ToolBox::ImplHasExternalMenubutton() const
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

void ToolBox::SetItemBits( ToolBoxItemId nItemId, ToolBoxItemBits nBits )
{
    ImplToolItems::size_type nPos = GetItemPos( nItemId );

    if ( nPos < GetItemCount() )
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

void ToolBox::SetItemWindowNonInteractive(ToolBoxItemId nItemId, bool bNonInteractive)
{
    ImplToolItems::size_type nPos = GetItemPos( nItemId );

    if ( nPos < GetItemCount() )
    {
        mpData->m_aItems[nPos].mbNonInteractiveWindow = bNonInteractive;
    }
}

ToolBoxItemBits ToolBox::GetItemBits( ToolBoxItemId nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mnBits;
    else
        return ToolBoxItemBits::NONE;
}

void ToolBox::SetItemExpand( ToolBoxItemId nItemId, bool bExpand )
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

void ToolBox::SetItemData( ToolBoxItemId nItemId, void* pNewData )
{
    ImplToolItems::size_type nPos = GetItemPos( nItemId );

    if ( nPos < mpData->m_aItems.size() )
    {
        mpData->m_aItems[nPos].mpUserData = pNewData;
        ImplUpdateItem( nPos );
    }
}

void* ToolBox::GetItemData( ToolBoxItemId nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mpUserData;
    else
        return nullptr;
}

static Image ImplMirrorImage( const Image& rImage )
{
    BitmapEx    aMirrBitmapEx( rImage.GetBitmapEx() );

    aMirrBitmapEx.Mirror( BmpMirrorFlags::Horizontal );

    return Image( aMirrBitmapEx );
}

static Image ImplRotImage( const Image& rImage, Degree10 nAngle10 )
{
    BitmapEx    aRotBitmapEx( rImage.GetBitmapEx() );

    aRotBitmapEx.Rotate( nAngle10, COL_WHITE );

    return Image( aRotBitmapEx );
}

void ToolBox::SetItemImage( ToolBoxItemId nItemId, const Image& rImage )
{
    ImplToolItems::size_type nPos = GetItemPos( nItemId );

    if ( nPos == ITEM_NOTFOUND )
        return;

    ImplToolItem* pItem = &mpData->m_aItems[nPos];
    Size aOldSize = pItem->maImage.GetSizePixel();

    pItem->maImage = pItem->mbMirrorMode ? ImplMirrorImage(rImage) : rImage;
    if (pItem->mnImageAngle != 0_deg10)
        pItem->maImage = ImplRotImage(pItem->maImage, pItem->mnImageAngle);

    // only once all is calculated, do extra work
    if (!mbCalc)
    {
        if (aOldSize != pItem->maImage.GetSizePixel())
            ImplInvalidate( true );
        else
            ImplUpdateItem( nPos );
    }
}

void ToolBox::SetItemImageAngle( ToolBoxItemId nItemId, Degree10 nAngle10 )
{
    ImplToolItems::size_type nPos = GetItemPos( nItemId );

    if ( nPos == ITEM_NOTFOUND )
        return;

    ImplToolItem* pItem = &mpData->m_aItems[nPos];
    pItem->mnImageAngle = nAngle10;
}

void ToolBox::SetItemImageMirrorMode( ToolBoxItemId nItemId, bool bMirror )
{
    ImplToolItems::size_type nPos = GetItemPos( nItemId );

    if ( nPos == ITEM_NOTFOUND )
        return;

    ImplToolItem* pItem = &mpData->m_aItems[nPos];
    pItem->mbMirrorMode = bMirror;
}

Image ToolBox::GetItemImage(ToolBoxItemId nItemId) const
{
    ImplToolItem* pItem = ImplGetItem(nItemId);
    return pItem ? pItem->maImage : Image();
}

void ToolBox::SetItemText( ToolBoxItemId nItemId, const OUString& rText )
{
    ImplToolItems::size_type nPos = GetItemPos( nItemId );

    if ( nPos == ITEM_NOTFOUND )
        return;

    ImplToolItem* pItem = &mpData->m_aItems[nPos];
    // only once all is calculated, do extra work
    if ( !mbCalc &&
         ((meButtonType != ButtonType::SYMBOLONLY) || !pItem->maImage) )
    {
        tools::Long nOldWidth = GetOutDev()->GetCtrlTextWidth( pItem->maText );
        pItem->maText = MnemonicGenerator::EraseAllMnemonicChars(rText);
        mpData->ImplClearLayoutData();
        if ( nOldWidth != GetOutDev()->GetCtrlTextWidth( pItem->maText ) )
            ImplInvalidate( true );
        else
            ImplUpdateItem( nPos );
    }
    else
        pItem->maText = MnemonicGenerator::EraseAllMnemonicChars(rText);

    // Notify
    CallEventListeners( VclEventId::ToolboxItemTextChanged, reinterpret_cast< void* >( nPos ) );
}

const OUString& ToolBox::GetItemText( ToolBoxItemId nItemId ) const
{

    ImplToolItem* pItem = ImplGetItem( nItemId );

    assert( pItem );

    return pItem->maText;
}

void ToolBox::SetItemWindow( ToolBoxItemId nItemId, vcl::Window* pNewWindow )
{
    ImplToolItems::size_type nPos = GetItemPos( nItemId );

    if ( nPos != ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        pItem->mpWindow = pNewWindow;
        if ( pNewWindow )
            pNewWindow->Hide();
        ImplInvalidate( true );
        CallEventListeners( VclEventId::ToolboxItemWindowChanged, reinterpret_cast< void* >( nPos ) );
    }
}

vcl::Window* ToolBox::GetItemWindow( ToolBoxItemId nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mpWindow;
    else
        return nullptr;
}

void ToolBox::EndSelection()
{
    if ( mbDrag )
    {
        // reset
        mbDrag = false;
        if (mnCurPos != ITEM_NOTFOUND)
            InvalidateItem(mnCurPos);
        EndTracking();
        if (IsMouseCaptured())
            ReleaseMouse();
        Deactivate();
    }

    mnCurPos        = ITEM_NOTFOUND;
    mnCurItemId     = ToolBoxItemId(0);
    mnDownItemId    = ToolBoxItemId(0);
    mnMouseModifier = 0;
}

void ToolBox::SetItemDown( ToolBoxItemId nItemId, bool bDown )
{
    ImplToolItems::size_type nPos = GetItemPos( nItemId );

    if ( nPos == ITEM_NOTFOUND )
        return;

    if ( bDown )
    {
        if ( nPos != mnCurPos )
        {
            mnCurPos = nPos;
            InvalidateItem(mnCurPos);
            GetOutDev()->Flush();
        }
    }
    else
    {
        if ( nPos == mnCurPos )
        {
            InvalidateItem(mnCurPos);
            GetOutDev()->Flush();
            mnCurPos = ITEM_NOTFOUND;
        }
    }

    if ( mbDrag )
    {
        mbDrag = false;
        EndTracking();
        if (IsMouseCaptured())
            ReleaseMouse();
        Deactivate();
    }

    mnCurItemId     = ToolBoxItemId(0);
    mnDownItemId    = ToolBoxItemId(0);
    mnMouseModifier = 0;
}

void ToolBox::SetItemState( ToolBoxItemId nItemId, TriState eState )
{
    ImplToolItems::size_type nPos = GetItemPos( nItemId );

    if ( nPos == ITEM_NOTFOUND )
        return;

    ImplToolItem* pItem = &mpData->m_aItems[nPos];

    // the state has changed
    if ( pItem->meState == eState )
        return;

    // if RadioCheck, un-check the previous
    if ( (eState == TRISTATE_TRUE) && (pItem->mnBits & ToolBoxItemBits::AUTOCHECK) &&
         (pItem->mnBits & ToolBoxItemBits::RADIOCHECK) )
    {
        ImplToolItem*    pGroupItem;
        ImplToolItems::size_type nGroupPos;
        ImplToolItems::size_type nItemCount = GetItemCount();

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

    // Call accessible listener to notify state_changed event
    CallEventListeners( VclEventId::ToolboxItemUpdated, reinterpret_cast< void* >(nPos) );
}

TriState ToolBox::GetItemState( ToolBoxItemId nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->meState;
    else
        return TRISTATE_FALSE;
}

void ToolBox::EnableItem( ToolBoxItemId nItemId, bool bEnable )
{
    ImplToolItems::size_type nPos = GetItemPos( nItemId );

    if ( nPos == ITEM_NOTFOUND )
        return;

    ImplToolItem* pItem = &mpData->m_aItems[nPos];
    if ( pItem->mbEnabled == bEnable )
        return;

    pItem->mbEnabled = bEnable;

    // if existing, also redraw the window
    if ( pItem->mpWindow )
        pItem->mpWindow->Enable( pItem->mbEnabled );

    // update item
    ImplUpdateItem( nPos );

    ImplUpdateInputEnable();

    CallEventListeners( bEnable ? VclEventId::ToolboxItemEnabled : VclEventId::ToolboxItemDisabled, reinterpret_cast< void* >( nPos ) );
}

bool ToolBox::IsItemEnabled( ToolBoxItemId nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mbEnabled;
    else
        return false;
}

void ToolBox::ShowItem( ToolBoxItemId nItemId, bool bVisible )
{
    ImplToolItems::size_type nPos = GetItemPos( nItemId );
    mpData->ImplClearLayoutData();

    if ( nPos != ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        if ( pItem->mbVisible != bVisible )
        {
            pItem->mbVisible = bVisible;
            ImplInvalidate();
        }
    }
}

bool ToolBox::IsItemClipped( ToolBoxItemId nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->IsClipped();
    else
        return false;
}

bool ToolBox::IsItemVisible( ToolBoxItemId nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mbVisible;
    else
        return false;
}

bool ToolBox::IsItemReallyVisible( ToolBoxItemId nItemId ) const
{
    // is the item on the visible area of the toolbox?
    bool bRet = false;
    tools::Rectangle aRect( mnLeftBorder, mnTopBorder, mnDX-mnRightBorder, mnDY-mnBottomBorder );
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem && pItem->mbVisible &&
         !pItem->maRect.IsEmpty() && aRect.Overlaps( pItem->maRect ) )
    {
        bRet = true;
    }

    return bRet;
}

void ToolBox::SetItemCommand(ToolBoxItemId nItemId, const OUString& rCommand)
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if (pItem)
        pItem->maCommandStr = rCommand;
}

OUString ToolBox::GetItemCommand( ToolBoxItemId nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if (pItem)
        return pItem->maCommandStr;

    return OUString();
}

void ToolBox::SetQuickHelpText( ToolBoxItemId nItemId, const OUString& rText )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        pItem->maQuickHelpText = rText;
}

OUString ToolBox::GetQuickHelpText( ToolBoxItemId nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maQuickHelpText;
    else
        return OUString();
}

void ToolBox::SetHelpText( ToolBoxItemId nItemId, const OUString& rText )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        pItem->maHelpText = rText;
}

const OUString& ToolBox::GetHelpText( ToolBoxItemId nItemId ) const
{
    return ImplGetHelpText( nItemId );
}

void ToolBox::SetAccessibleName(ToolBoxItemId nItemId, const OUString& rText)
{
    ImplToolItem* pItem = ImplGetItem(nItemId);

    if (pItem)
        pItem->maAccessibleName = rText;
}

OUString ToolBox::GetAccessibleName(ToolBoxItemId nItemId) const
{
    ImplToolItem* pItem = ImplGetItem(nItemId);
    if (pItem)
        return pItem->maAccessibleName;

    return OUString();
}

void ToolBox::SetHelpId( ToolBoxItemId nItemId, const OUString& rHelpId )
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        pItem->maHelpId = rHelpId;
}

// disable key input if all items are disabled
void ToolBox::ImplUpdateInputEnable()
{
    mpData->mbKeyInputDisabled = std::none_of(mpData->m_aItems.begin(), mpData->m_aItems.end(),
        [](const ImplToolItem& rItem) {
            // at least one useful entry
            return rItem.mbEnabled;
        });
}

void ToolBox::ImplFillLayoutData()
{
    mpData->m_pLayoutData.emplace();

    ImplToolItems::size_type nCount = mpData->m_aItems.size();
    for( ImplToolItems::size_type i = 0; i < nCount; i++ )
    {
        ImplToolItem* pItem = &mpData->m_aItems[i];

        // only draw, if the rectangle is within PaintRectangle
        if (!pItem->maRect.IsEmpty())
            InvalidateItem(i);
    }
}

OUString ToolBox::GetDisplayText() const
{
    if( ! mpData->m_pLayoutData )
        const_cast<ToolBox *>(this)->ImplFillLayoutData();
    return mpData->m_pLayoutData ? mpData->m_pLayoutData->m_aDisplayText : OUString();
}

tools::Rectangle ToolBox::GetCharacterBounds( ToolBoxItemId nItemID, tools::Long nIndex )
{
    tools::Long nItemIndex = -1;
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    if( mpData->m_pLayoutData )
    {
        for( size_t i = 0; i < mpData->m_pLayoutData->m_aLineItemIds.size(); i++ )
        {
            if( mpData->m_pLayoutData->m_aLineItemIds[i] == nItemID )
            {
                nItemIndex = mpData->m_pLayoutData->m_aLineIndices[i];
                break;
            }
        }
    }
    return (mpData->m_pLayoutData && nItemIndex != -1) ? mpData->m_pLayoutData->GetCharacterBounds( nItemIndex+nIndex ) : tools::Rectangle();
}

tools::Long ToolBox::GetIndexForPoint( const Point& rPoint, ToolBoxItemId& rItemID )
{
    tools::Long nIndex = -1;
    rItemID = ToolBoxItemId(0);
    if( ! mpData->m_pLayoutData )
        ImplFillLayoutData();
    if( mpData->m_pLayoutData )
    {
        nIndex = mpData->m_pLayoutData->GetIndexForPoint( rPoint );
        for( size_t i = 0; i < mpData->m_pLayoutData->m_aLineIndices.size(); i++ )
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
    if( aType == mpData->maMenuType )
        return;

    mpData->maMenuType = aType;
    if( IsFloatingMode() )
    {
        // the menu button may have to be moved into the decoration which changes the layout
        ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
        if( pWrapper )
            pWrapper->ShowMenuTitleButton( bool( aType & ToolBoxMenuType::Customize) );

        mbFormat = true;
        ImplFormat();
        ImplSetMinMaxFloatSize();
    }
    else
    {
        // trigger redraw of menu button
        if( !mpData->maMenubuttonItem.maRect.IsEmpty() )
            Invalidate(mpData->maMenubuttonItem.maRect);
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

void ToolBox::SetMenuExecuteHdl( const Link<ToolBox *, void>& rLink )
{
    mpData->maMenuButtonHdl = rLink;
}

bool ToolBox::ImplHasClippedItems()
{
    // are any items currently clipped ?
    ImplFormat();
    return std::any_of(mpData->m_aItems.begin(), mpData->m_aItems.end(),
        [](const ImplToolItem& rItem) { return rItem.IsClipped(); });
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
    PopupMenu *pMenu = GetMenu();
    pMenu->Clear();

    // add menu items: first the overflow items, then hidden items, both in the
    // order they would usually appear in the toolbar. Separators that would be
    // in the toolbar are ignored as they would introduce too much clutter,
    // instead we have a single separator to help distinguish between overflow
    // and hidden items.
    if ( mpData->m_aItems.empty() )
        return;

    // nStartPos will hold the number of clipped items appended from first loop
    for ( const auto& rItem : mpData->m_aItems )
    {
        if( rItem.IsClipped() )
        {
            sal_uInt16 id = sal_uInt16(rItem.mnId) + TOOLBOX_MENUITEM_START;
            MenuItemBits nMenuItemBits = ConvertBitsFromToolBoxToMenu(rItem.mnBits);
            pMenu->InsertItem( id, rItem.maText, rItem.maImage, nMenuItemBits);
            pMenu->SetItemCommand( id, rItem.maCommandStr );
            pMenu->EnableItem( id, rItem.mbEnabled );
            pMenu->CheckItem ( id, rItem.meState == TRISTATE_TRUE );
        }
    }

    // add a separator below the inserted clipped-items
    pMenu->InsertSeparator();

    // now append the items that are explicitly disabled
    for ( const auto& rItem : mpData->m_aItems )
    {
        if( rItem.IsItemHidden() )
        {
            sal_uInt16 id = sal_uInt16(rItem.mnId) + TOOLBOX_MENUITEM_START;
            MenuItemBits nMenuItemBits = ConvertBitsFromToolBoxToMenu(rItem.mnBits);
            pMenu->InsertItem( id, rItem.maText, rItem.maImage, nMenuItemBits );
            pMenu->SetItemCommand( id, rItem.maCommandStr );
            pMenu->EnableItem( id, rItem.mbEnabled );
            pMenu->CheckItem( id, rItem.meState == TRISTATE_TRUE );
        }
    }
}

IMPL_LINK( ToolBox, ImplCustomMenuListener, VclMenuEvent&, rEvent, void )
{
    if( rEvent.GetMenu() == GetMenu() && rEvent.GetId() == VclEventId::MenuSelect )
    {
        sal_uInt16 id = GetMenu()->GetItemId( rEvent.GetItemPos() );
        if( id >= TOOLBOX_MENUITEM_START )
            TriggerItem( ToolBoxItemId(id - TOOLBOX_MENUITEM_START) );
    }
}

void ToolBox::ExecuteCustomMenu( const tools::Rectangle& rRect )
{
    if ( !IsMenuEnabled() || ImplIsInPopupMode() )
        return;

    UpdateCustomMenu();

    if( GetMenuType() & ToolBoxMenuType::Customize )
        // call button handler to allow for menu customization
        mpData->maMenuButtonHdl.Call( this );

    GetMenu()->AddEventListener( LINK( this, ToolBox, ImplCustomMenuListener ) );

    // make sure all disabled entries will be shown
    GetMenu()->SetMenuFlags(
        GetMenu()->GetMenuFlags() | MenuFlags::AlwaysShowDisabledEntries );

    // toolbox might be destroyed during execute
    bool bBorderDel = false;

    VclPtr<vcl::Window> pWin = this;
    tools::Rectangle aMenuRect = rRect;
    VclPtr<ImplBorderWindow> pBorderWin;
    if( aMenuRect.IsEmpty() && IsFloatingMode() )
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

    sal_uInt16 uId = GetMenu()->Execute( pWin, tools::Rectangle( ImplGetPopupPosition( aMenuRect ), Size() ),
                            PopupMenuFlags::ExecuteDown | PopupMenuFlags::NoMouseUpClose );

    if ( pWin->isDisposed() )
        return;

    if( GetMenu() )
        GetMenu()->RemoveEventListener( LINK( this, ToolBox, ImplCustomMenuListener ) );
    if( bBorderDel )
    {
        if( pBorderWin->isDisposed() )
            return;
    }

    pWin->Invalidate( aMenuRect );

    if( uId )
        GrabFocusToDocument();
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
        return ( pWrapper && pWrapper->GetFloatingWindow() && static_cast<FloatingWindow*>(pWrapper->GetFloatingWindow())->IsInPopupMode() );
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
            u"/org.openoffice.Office.UI.GlobalSettings/Toolbars"_ustr );    // note: case sensitive !
        if ( aNode.isValid() )
        {
            // feature enabled ?
            bool bStatesEnabled = bool();
            css::uno::Any aValue = aNode.getNodeValue( "StatesEnabled" );
            if( aValue >>= bStatesEnabled )
            {
                if( bStatesEnabled )
                {
                    // now read the locking state
                    utl::OConfigurationNode aNode2 = utl::OConfigurationTreeRoot::tryCreateWithComponentContext(
                        comphelper::getProcessComponentContext(),
                        u"/org.openoffice.Office.UI.GlobalSettings/Toolbars/States"_ustr );    // note: case sensitive !

                    bool bLocked = bool();
                    css::uno::Any aValue2 = aNode2.getNodeValue( "Locked" );
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

void ToolBox::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    DockingWindow::DumpAsPropertyTree(rJsonWriter);

    auto childrenNode = rJsonWriter.startArray("children");
    for (ToolBox::ImplToolItems::size_type i = 0; i < GetItemCount(); ++i)
    {
        auto childNode = rJsonWriter.startStruct();
        ToolBoxItemId nId = GetItemId(i);

        vcl::Window* pWindow = GetItemWindow(nId);
        if (pWindow)
        {
            pWindow->DumpAsPropertyTree(rJsonWriter);
        }
        else
        {
            OUString sCommand = GetItemCommand(nId);
            rJsonWriter.put("type", "toolitem");
            rJsonWriter.put("text", GetItemText(nId));
            rJsonWriter.put("command", sCommand);
            if (IsItemChecked(nId))
                rJsonWriter.put("selected", true);
            if (!IsItemVisible(nId))
                rJsonWriter.put("visible", false);
            if (GetItemBits(nId) & ToolBoxItemBits::DROPDOWN)
                rJsonWriter.put("dropdown", true);
            if (!IsItemEnabled(nId))
                rJsonWriter.put("enabled", false);

            Image aImage = GetItemImage(nId);
            if (!sCommand.startsWith(".uno:") && !!aImage)
            {
                SvMemoryStream aOStm(6535, 6535);
                if(GraphicConverter::Export(aOStm, aImage.GetBitmapEx(), ConvertDataFormat::PNG) == ERRCODE_NONE)
                {
                    css::uno::Sequence<sal_Int8> aSeq( static_cast<sal_Int8 const *>(aOStm.GetData()), aOStm.Tell());
                    OStringBuffer aBuffer("data:image/png;base64,");
                    ::comphelper::Base64::encode(aBuffer, aSeq);
                    rJsonWriter.put("image", aBuffer);
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
