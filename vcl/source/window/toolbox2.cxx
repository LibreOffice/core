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
#include <comphelper/string.hxx>
#include <tools/debug.hxx>
#include <tools/rc.h>

#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/menu.hxx>
#include <vcl/ImageListProvider.hxx>
#include <vcl/settings.hxx>
#include <vcl/IconThemeInfo.hxx>

#include <svdata.hxx>
#include <brdwin.hxx>
#include <toolbox.h>

#include <unotools/confignode.hxx>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>

using namespace vcl;
using namespace com::sun::star;

#define TB_SEP_SIZE             8

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
    if( m_pLayoutData )
        delete m_pLayoutData;
    delete mpMenu;
}


void ImplToolItem::init(sal_uInt16 nItemId, ToolBoxItemBits nItemBits,
                        bool bEmptyBtn)
{
    mnId            = nItemId;
    mpWindow        = NULL;
    mpUserData      = NULL;
    meType          = TOOLBOXITEM_BUTTON;
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
    init(0, 0, true);
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
        maHelpId                ( rItem.maHelpId ),
        maRect                  ( rItem.maRect ),
        maCalcRect              ( rItem.maCalcRect ),
        maMinimalItemSize       ( rItem.maMinimalItemSize ),
        maItemSize              ( rItem.maItemSize ),
        mnSepSize               ( rItem.mnSepSize ),
        mnDropDownArrowWidth    ( rItem.mnDropDownArrowWidth ),
        maContentSize           ( rItem.maContentSize ),
        meType                  ( rItem.meType ),
        mnBits                  ( rItem.mnBits ),
        meState                 ( rItem.meState ),
        mnId                    ( rItem.mnId ),
        mbEnabled               ( rItem.mbEnabled ),
        mbVisible               ( rItem.mbVisible ),
        mbEmptyBtn              ( rItem.mbEmptyBtn ),
        mbShowWindow            ( rItem.mbShowWindow ),
        mbBreak                 ( rItem.mbBreak ),
        mbVisibleText           ( rItem.mbVisibleText ),
        mbExpand                ( rItem.mbExpand )
{
}

ImplToolItem::~ImplToolItem()
{
}

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
    maHelpId                = rItem.maHelpId;
    maRect                  = rItem.maRect;
    maCalcRect              = rItem.maCalcRect;
    mnSepSize               = rItem.mnSepSize;
    mnDropDownArrowWidth    = rItem.mnDropDownArrowWidth;
    maContentSize           = rItem.maContentSize;
    maMinimalItemSize       = rItem.maMinimalItemSize;
    maItemSize              = rItem.maItemSize;
    mbVisibleText           = rItem.mbVisibleText;
    mbExpand                = rItem.mbExpand;
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

Size ImplToolItem::GetSize( bool bHorz, bool bCheckMaxWidth, long maxWidth, const Size& rDefaultSize )
{
    Size aSize( rDefaultSize ); // the size of 'standard' toolbox items
                                // non-standard items are eg windows or buttons with text

    if ( (meType == TOOLBOXITEM_BUTTON) || (meType == TOOLBOXITEM_SPACE) )
    {
        aSize = maItemSize;

        if ( mpWindow && bHorz )
        {
            // get size of item window and check if it fits
            // no windows in vertical toolbars (the default is mbShowWindow=false)
            Size aWinSize = mpWindow->GetSizePixel();
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

void ImplToolItem::DetermineButtonDrawStyle( ButtonType eButtonType, bool& rbImage, bool& rbText ) const
{
    if ( meType != TOOLBOXITEM_BUTTON )
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
    // prefer texts if textonly buttons are dreawn

    if ( eButtonType == BUTTON_SYMBOL )         // drawing icons only
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
    else if ( eButtonType == BUTTON_TEXT )      // drawing text only
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

bool ImplToolItem::IsClipped() const
{
    return ( meType == TOOLBOXITEM_BUTTON && mbVisible && maRect.IsEmpty() );
}

bool ImplToolItem::IsItemHidden() const
{
    return ( meType == TOOLBOXITEM_BUTTON && !mbVisible );
}

const OUString ToolBox::ImplConvertMenuString( const OUString& rStr )
{
    OUString aCvtStr( rStr );
    if ( mbMenuStrings )
        aCvtStr = comphelper::string::stripEnd(aCvtStr, '.');
    aCvtStr = MnemonicGenerator::EraseAllMnemonicChars( aCvtStr );
    return aCvtStr;
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
            maTimer.Stop();
        }
    }
    else
    {
        if ( !mbFormat )
        {
            mbFormat = true;

            // do we need to redraw?
            if ( IsReallyVisible() && IsUpdateMode() )
                maTimer.Start();
        }
    }

    // request new layout by layoutmanager
    ImplCallEventListeners( VCLEVENT_TOOLBOX_FORMATCHANGED );
}

void ToolBox::ImplUpdateItem( sal_uInt16 nIndex )
{
    // do we need to redraw?
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

void ToolBox::Click()
{
    ImplCallEventListeners( VCLEVENT_TOOLBOX_CLICK );
    maClickHdl.Call( this );
}

void ToolBox::DoubleClick()
{
    ImplCallEventListeners( VCLEVENT_TOOLBOX_DOUBLECLICK );
    maDoubleClickHdl.Call( this );
}

void ToolBox::Activate()
{
    mnActivateCount++;
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ACTIVATE );
    maActivateHdl.Call( this );
}

void ToolBox::Deactivate()
{
    mnActivateCount--;
    ImplCallEventListeners( VCLEVENT_TOOLBOX_DEACTIVATE );
    maDeactivateHdl.Call( this );
}

void ToolBox::Highlight()
{
    ImplCallEventListeners( VCLEVENT_TOOLBOX_HIGHLIGHT );
    maHighlightHdl.Call( this );
}

void ToolBox::Select()
{
    ImplDelData aDelData;
    ImplAddDel( &aDelData );

    ImplCallEventListeners( VCLEVENT_TOOLBOX_SELECT );
    maSelectHdl.Call( this );

    if ( aDelData.IsDead() )
        return;
    ImplRemoveDel( &aDelData );

    // TODO: GetFloatingWindow in DockingWindow is currently inline, change it to check dockingwrapper
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper && pWrapper->GetFloatingWindow() && pWrapper->GetFloatingWindow()->IsInPopupMode() )
        pWrapper->GetFloatingWindow()->EndPopupMode();
}

void ToolBox::Customize( const ToolBoxCustomizeEvent& )
{
}

void ToolBox::UserDraw( const UserDrawEvent& )
{
}

void ToolBox::InsertItem( const ResId& rResId, sal_uInt16 nPos )
{
    sal_uLong nObjMask;
    bool      bImage = false;     // has image

    // create item
    ImplToolItem aItem;

    GetRes( rResId.SetRT( RSC_TOOLBOXITEM ) );
    nObjMask            = ReadLongRes();

    if ( nObjMask & RSC_TOOLBOXITEM_ID )
        aItem.mnId = sal::static_int_cast<sal_uInt16>(ReadLongRes());
    else
        aItem.mnId = 1;

    if ( nObjMask & RSC_TOOLBOXITEM_TYPE )
        aItem.meType = (ToolBoxItemType)ReadLongRes();

    if ( nObjMask & RSC_TOOLBOXITEM_STATUS )
        aItem.mnBits = (ToolBoxItemBits)ReadLongRes();

    if( nObjMask & RSC_TOOLBOXITEM_HELPID )
        aItem.maHelpId = ReadByteStringRes();

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
        bImage = true;
    }
    if ( nObjMask & RSC_TOOLBOXITEM_IMAGE )
    {
        aItem.maImage = Image( ResId( (RSHEADER_TYPE*)GetClassRes(), *rResId.GetResMgr() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
        bImage = true;
    }
    if ( nObjMask & RSC_TOOLBOXITEM_DISABLE )
        aItem.mbEnabled = ReadShortRes() == 0;

    if ( nObjMask & RSC_TOOLBOXITEM_STATE )
        aItem.meState   = (TriState)ReadLongRes();

    if ( nObjMask & RSC_TOOLBOXITEM_HIDE )
        aItem.mbVisible = ReadShortRes() == 0;

    if ( nObjMask & RSC_TOOLBOXITEM_COMMAND )
        aItem.maCommandStr = ReadStringRes();

    // if no image is loaded, try to load one from the image list
    if ( !bImage && aItem.mnId )
        aItem.maImage = maImageList.GetImage( aItem.mnId );

    // if this is a ButtonItem, check ID
    bool bNewCalc;
    if ( aItem.meType != TOOLBOXITEM_BUTTON )
    {
        bNewCalc = false;
        aItem.mnId = 0;
    }
    else
    {
        bNewCalc = true;

        DBG_ASSERT( aItem.mnId, "ToolBox::InsertItem(): ItemId == 0" );
        DBG_ASSERT( GetItemPos( aItem.mnId ) == TOOLBOX_ITEM_NOTFOUND,
                    "ToolBox::InsertItem(): ItemId already exists" );
    }

    // create item and add to list
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    // recalculate ToolBox and redraw
    ImplInvalidate( bNewCalc );

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::InsertItem( sal_uInt16 nItemId, const Image& rImage,
                          ToolBoxItemBits nBits, sal_uInt16 nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertItem(): ItemId already exists" );

    // create item and add to list
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), ImplToolItem( nItemId, rImage, nBits ) );
    SetItemImage(nItemId, rImage);
    mpData->ImplClearLayoutData();

    ImplInvalidate( true );

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >(nNewPos ) );
}

void ToolBox::InsertItem( sal_uInt16 nItemId, const Image& rImage,
                          const OUString& rText,
                          ToolBoxItemBits nBits, sal_uInt16 nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertItem(): ItemId already exists" );

    // create item and add to list
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), ImplToolItem( nItemId, rImage, ImplConvertMenuString( rText ), nBits ) );
    SetItemImage(nItemId, rImage);
    mpData->ImplClearLayoutData();

    ImplInvalidate( true );

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::InsertItem( sal_uInt16 nItemId, const OUString& rText,
                          ToolBoxItemBits nBits, sal_uInt16 nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertItem(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertItem(): ItemId already exists" );

    // create item and add to list
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), ImplToolItem( nItemId, ImplConvertMenuString( rText ), nBits ) );
    mpData->ImplClearLayoutData();

    ImplInvalidate( true );

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos ) );
}

// Get label of the command (like of .uno:Save) from the description service
static OUString getCommandLabel(const OUString& rCommand, const uno::Reference<uno::XComponentContext>& rContext, const OUString& rModuleId)
{
    if (rCommand.isEmpty())
        return OUString();

    try
    {
        uno::Reference<container::XNameAccess> xUICommandLabels;
        uno::Reference<container::XNameAccess> xUICommandDescription(frame::theUICommandDescription::get(rContext));

        if ((xUICommandDescription->getByName(rModuleId) >>= xUICommandLabels) && xUICommandLabels.is())
        {
            uno::Sequence<beans::PropertyValue> aProperties;
            if (xUICommandLabels->getByName(rCommand) >>= aProperties)
            {
                for ( sal_Int32 i = 0; i < aProperties.getLength(); i++ )
                {
                    if (aProperties[i].Name == "Label")
                    {
                        OUString aLabel;
                        if (aProperties[i].Value >>= aLabel)
                            return aLabel;
                    }
                }
            }
        }
    }
    catch (uno::Exception&)
    {
    }

    return OUString();
}


// Get label of the command (like of .uno:Save) from the description service
static Image getCommandImage(const OUString& rCommand, bool bLarge,
        const uno::Reference<uno::XComponentContext>& rContext, const uno::Reference<frame::XFrame>& rFrame,
        const OUString& rModuleId)
{
    if (rCommand.isEmpty())
        return Image();

    sal_Int16 nImageType(ui::ImageType::COLOR_NORMAL | ui::ImageType::SIZE_DEFAULT);
    if (bLarge)
        nImageType |= ui::ImageType::SIZE_LARGE;

    try
    {
        uno::Reference<frame::XController> xController(rFrame->getController());
        uno::Reference<frame::XModel> xModel(xController->getModel());

        uno::Reference<ui::XUIConfigurationManagerSupplier> xSupplier(xModel, uno::UNO_QUERY);
        uno::Reference<ui::XUIConfigurationManager> xDocUICfgMgr(xSupplier->getUIConfigurationManager(), uno::UNO_QUERY);
        uno::Reference<ui::XImageManager> xDocImgMgr(xDocUICfgMgr->getImageManager(), uno::UNO_QUERY);

        uno::Sequence< uno::Reference<graphic::XGraphic> > aGraphicSeq;
        uno::Sequence<OUString> aImageCmdSeq(1);
        aImageCmdSeq[0] = rCommand;

        aGraphicSeq = xDocImgMgr->getImages( nImageType, aImageCmdSeq );
        uno::Reference<graphic::XGraphic> xGraphic = aGraphicSeq[0];
        Image aImage(xGraphic);

        if (!!aImage)
            return aImage;
    }
    catch (uno::Exception&)
    {
    }

    try {
        uno::Reference<ui::XModuleUIConfigurationManagerSupplier> xModuleCfgMgrSupplier(ui::theModuleUIConfigurationManagerSupplier::get(rContext));
        uno::Reference<ui::XUIConfigurationManager> xUICfgMgr(xModuleCfgMgrSupplier->getUIConfigurationManager(rModuleId));

        uno::Sequence< uno::Reference<graphic::XGraphic> > aGraphicSeq;
        uno::Reference<ui::XImageManager> xModuleImageManager(xUICfgMgr->getImageManager(), uno::UNO_QUERY);

        uno::Sequence<OUString> aImageCmdSeq(1);
        aImageCmdSeq[0] = rCommand;

        aGraphicSeq = xModuleImageManager->getImages(nImageType, aImageCmdSeq);

        uno::Reference<graphic::XGraphic> xGraphic(aGraphicSeq[0]);

        return Image(xGraphic);
    }
    catch (uno::Exception&)
    {
    }

    return Image();
}

void ToolBox::InsertItem(const OUString& rCommand, const uno::Reference<frame::XFrame>& rFrame, ToolBoxItemBits nBits, const Size& rRequestedSize, sal_uInt16 nPos)
{
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    uno::Reference<frame::XModuleManager2> xModuleManager(frame::ModuleManager::create(xContext));
    OUString aModuleId(xModuleManager->identify(rFrame));

    OUString aLabel(getCommandLabel(rCommand, xContext, aModuleId));
    Image aImage(getCommandImage(rCommand, (GetToolboxButtonSize() == TOOLBOX_BUTTONSIZE_LARGE), xContext, rFrame, aModuleId));

    // let's invent an ItemId
    const sal_uInt16 COMMAND_ITEMID_START = 30000;
    sal_uInt16 nItemId = COMMAND_ITEMID_START + GetItemCount();

    InsertItem(nItemId, aImage, aLabel, nBits, nPos);
    SetItemCommand(nItemId, rCommand);

    // set the minimal size
    ImplToolItem* pItem = ImplGetItem( nItemId );
    if ( pItem )
        pItem->maMinimalItemSize = rRequestedSize;
}

void ToolBox::InsertWindow( sal_uInt16 nItemId, Window* pWindow,
                            ToolBoxItemBits nBits, sal_uInt16 nPos )
{
    DBG_ASSERT( nItemId, "ToolBox::InsertWindow(): ItemId == 0" );
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::InsertWindow(): ItemId already exists" );

    // create item and add to list
    ImplToolItem aItem;
    aItem.mnId       = nItemId;
    aItem.meType     = TOOLBOXITEM_BUTTON;
    aItem.mnBits     = nBits;
    aItem.mpWindow   = pWindow;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    if ( pWindow )
        pWindow->Hide();

    ImplInvalidate( true );

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::InsertSpace( sal_uInt16 nPos )
{
    // create item and add to list
    ImplToolItem aItem;
    aItem.meType     = TOOLBOXITEM_SPACE;
    aItem.mbEnabled  = false;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    ImplInvalidate( false );

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::InsertSeparator( sal_uInt16 nPos, sal_uInt16 nPixSize )
{
    // create item and add to list
    ImplToolItem aItem;
    aItem.meType     = TOOLBOXITEM_SEPARATOR;
    aItem.mbEnabled  = false;
    if ( nPixSize )
        aItem.mnSepSize = nPixSize;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    ImplInvalidate( false );

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::InsertBreak( sal_uInt16 nPos )
{
    // create item and add to list
    ImplToolItem aItem;
    aItem.meType     = TOOLBOXITEM_BREAK;
    aItem.mbEnabled  = false;
    mpData->m_aItems.insert( (nPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nPos : mpData->m_aItems.end(), aItem );
    mpData->ImplClearLayoutData();

    ImplInvalidate( false );

    // Notify
    sal_uInt16 nNewPos = sal::static_int_cast<sal_uInt16>(( nPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nPos);
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos ) );
}

void ToolBox::RemoveItem( sal_uInt16 nPos )
{
    if( nPos < mpData->m_aItems.size() )
    {
        bool bMustCalc;
        if ( mpData->m_aItems[nPos].meType == TOOLBOXITEM_BUTTON )
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
        ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMREMOVED, reinterpret_cast< void* >( nPos ) );
    }
}

void ToolBox::CopyItem( const ToolBox& rToolBox, sal_uInt16 nItemId,
                        sal_uInt16 nNewPos )
{
    DBG_ASSERT( GetItemPos( nItemId ) == TOOLBOX_ITEM_NOTFOUND,
                "ToolBox::CopyItem(): ItemId already exists" );

    sal_uInt16 nPos = rToolBox.GetItemPos( nItemId );

    // found item
    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        // push ToolBox item onto the list
        ImplToolItem aNewItem = rToolBox.mpData->m_aItems[nPos];
        // reset state
        aNewItem.mpWindow      = NULL;
        aNewItem.mbShowWindow = false;

        mpData->m_aItems.insert( (nNewPos < mpData->m_aItems.size()) ? mpData->m_aItems.begin()+nNewPos : mpData->m_aItems.end(), aNewItem );
        mpData->ImplClearLayoutData();
        // redraw ToolBox
        ImplInvalidate( false );

        // Notify
        sal_uInt16 nNewPos2 = sal::static_int_cast<sal_uInt16>(( nNewPos == TOOLBOX_APPEND ) ? ( mpData->m_aItems.size() - 1 ) : nNewPos);
        ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMADDED, reinterpret_cast< void* >( nNewPos2 ) );
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
    ImplCallEventListeners( VCLEVENT_TOOLBOX_ALLITEMSCHANGED );
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

/*static*/ Size
ToolBox::GetDefaultImageSize(bool bLarge)
{
    const long TB_SMALLIMAGESIZE = 16;
    if (!bLarge) {
        return Size(TB_SMALLIMAGESIZE, TB_SMALLIMAGESIZE);
    }

    OUString iconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
    return vcl::IconThemeInfo::SizeByThemeName(iconTheme);
}

Size ToolBox::GetDefaultImageSize() const
{
    return GetDefaultImageSize( GetToolboxButtonSize() == TOOLBOX_BUTTONSIZE_LARGE );
}

void ToolBox::SetAlign( WindowAlign eNewAlign )
{
    if ( meAlign != eNewAlign )
    {
        meAlign = eNewAlign;

        if ( !ImplIsFloatingMode() )
        {
            // set horizontal/vertical alignment
            if ( (eNewAlign == WINDOWALIGN_LEFT) || (eNewAlign == WINDOWALIGN_RIGHT) )
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
        ImplInvalidate( false );
    }
}

void ToolBox::SetPageScroll( bool b )
{
    mpData->mbPageScroll = b;
}

sal_uInt16 ToolBox::GetItemCount() const
{
    return (sal_uInt16)mpData->m_aItems.size();
}

ToolBoxItemType ToolBox::GetItemType( sal_uInt16 nPos ) const
{
    return (nPos < mpData->m_aItems.size()) ? mpData->m_aItems[nPos].meType : TOOLBOXITEM_DONTKNOW;
}

sal_uInt16 ToolBox::GetItemPos( sal_uInt16 nItemId ) const
{
    int nCount = mpData->m_aItems.size();
    for( int nPos = 0; nPos < nCount; nPos++ )
        if( mpData->m_aItems[nPos].mnId == nItemId )
            return (sal_uInt16)nPos;

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
            if ( it->meType == TOOLBOXITEM_BUTTON )
                return it->mnId;
            else
                return 0;
        }

        ++it;
    }

    return 0;
}

sal_uInt16 ToolBox::GetItemId(const OUString &rCommand) const
{
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


Point ToolBox::GetItemPopupPosition( sal_uInt16 nItemId, const Size& rSize ) const
{
    return ImplGetPopupPosition( GetItemRect( nItemId ), rSize );
}

Rectangle ToolBox::GetItemRect( sal_uInt16 nItemId ) const
{
    if ( mbCalc || mbFormat )
        ((ToolBox*)this)->ImplFormat();

    sal_uInt16 nPos = GetItemPos( nItemId );
    return GetItemPosRect( nPos );
}

Rectangle ToolBox::GetItemPosRect( sal_uInt16 nPos ) const
{
    if ( mbCalc || mbFormat )
        ((ToolBox*)this)->ImplFormat();

    if ( nPos < mpData->m_aItems.size() )
        return mpData->m_aItems[nPos].maRect;
    else
        return Rectangle();
}

Size ToolBox::GetItemContentSize( sal_uInt16 nItemId ) const
{
    if ( mbCalc || mbFormat )
        ((ToolBox*)this)->ImplFormat();

    sal_uInt16 nPos = GetItemPos( nItemId );
    if ( nPos < mpData->m_aItems.size() )
        return mpData->m_aItems[nPos].maContentSize;
    else
        return Size();
}

bool ToolBox::ImplHasExternalMenubutton()
{
    // check if the borderwindow (i.e. the decoration) provides the menu button
    bool bRet = false;
    if( ImplIsFloatingMode() )
    {
        // custom menu is placed in the decoration
        ImplBorderWindow *pBorderWin = dynamic_cast<ImplBorderWindow*>( GetWindow( WINDOW_BORDER ) );
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
        nBits &= TIB_LEFT | TIB_AUTOSIZE | TIB_DROPDOWN;
        nOldBits &= TIB_LEFT | TIB_AUTOSIZE | TIB_DROPDOWN;
        // trigger reformat when the item width has changed (dropdown arrow)
        bool bFormat = (nBits & TIB_DROPDOWN) != (nOldBits & TIB_DROPDOWN);
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
        return 0;
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

bool ToolBox::GetItemExpand( sal_uInt16 nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );
    if (!pItem)
        return false;
    return pItem->mbExpand;
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
        return NULL;
}

void ToolBox::SetItemImage( sal_uInt16 nItemId, const Image& rImage )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        Image aImage(rImage);

#ifndef MACOSX
        if ( GetDPIScaleFactor() > 1)
        {
            BitmapEx aBitmap(aImage.GetBitmapEx());

            // Some code calls this twice, so add a sanity check
            // FIXME find out what that code is & fix accordingly
            if (aBitmap.GetSizePixel().Width() < 32)
            {
                aBitmap.Scale(GetDPIScaleFactor(), GetDPIScaleFactor(), BMP_SCALE_FAST);
                aImage = Image(aBitmap);
            }
        }
#endif
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        // only once all is calculated, do extra work
        if ( !mbCalc )
        {
            Size aOldSize = pItem->maImage.GetSizePixel();
            pItem->maImage = aImage;
            if ( aOldSize != pItem->maImage.GetSizePixel() )
                ImplInvalidate( true );
            else
                ImplUpdateItem( nPos );
        }
        else
            pItem->maImage = aImage;
    }
}

void ToolBox::SetImageList( const ImageList& rImageList )
{
    maImageList = rImageList;

    sal_uInt16 nCount = (sal_uInt16)mpData->m_aItems.size();
    for( sal_uInt16 i = 0; i < nCount; i++ )
    {
        Image aImage;
        if ( mpData->m_aItems[i].mnId )
            aImage = maImageList.GetImage( mpData->m_aItems[i].mnId );
        if( !!aImage )
            SetItemImage( mpData->m_aItems[i].mnId, aImage );
    }
}

static Image ImplRotImage( const Image& rImage, long nAngle10 )
{
    Image       aRet;
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
            if( !!pItem->maHighImage )
                pItem->maHighImage = ImplRotImage( pItem->maHighImage, nDeltaAngle );
        }

        if ( !mbCalc )
        {
            if ( aOldSize != pItem->maImage.GetSizePixel() )
                ImplInvalidate( true );
            else
                ImplUpdateItem( nPos );
        }
    }
}

static Image ImplMirrorImage( const Image& rImage )
{
    Image       aRet;
    BitmapEx    aMirrBitmapEx( rImage.GetBitmapEx() );

    aMirrBitmapEx.Mirror( BMP_MIRROR_HORZ );

    return Image( aMirrBitmapEx );
}

void ToolBox::SetItemImageMirrorMode( sal_uInt16 nItemId, bool bMirror )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

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

Image ToolBox::GetItemImage( sal_uInt16 nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maImage;
    else
        return Image();
}

void ToolBox::SetItemText( sal_uInt16 nItemId, const OUString& rText )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        // only once all is calculated, do extra work
        if ( !mbCalc &&
             ((meButtonType != BUTTON_SYMBOL) || !pItem->maImage) )
        {
            long nOldWidth = GetCtrlTextWidth( pItem->maText );
            pItem->maText = ImplConvertMenuString( rText );
            mpData->ImplClearLayoutData();
            if ( nOldWidth != GetCtrlTextWidth( pItem->maText ) )
                ImplInvalidate( true );
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

const OUString& ToolBox::GetItemText( sal_uInt16 nItemId ) const
{

    ImplToolItem* pItem = ImplGetItem( nItemId );

    assert( pItem );

    return pItem->maText;
}

void ToolBox::SetItemWindow( sal_uInt16 nItemId, Window* pNewWindow )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        ImplToolItem* pItem = &mpData->m_aItems[nPos];
        pItem->mpWindow = pNewWindow;
        if ( pNewWindow )
            pNewWindow->Hide();
        ImplInvalidate( true );
        ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMWINDOWCHANGED, reinterpret_cast< void* >( nPos ) );
    }
}

Window* ToolBox::GetItemWindow( sal_uInt16 nItemId ) const
{
    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->mpWindow;
    else
        return NULL;
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

void ToolBox::SetItemDown( sal_uInt16 nItemId, bool bDown, bool bRelease )
{
    sal_uInt16 nPos = GetItemPos( nItemId );

    if ( nPos != TOOLBOX_ITEM_NOTFOUND )
    {
        if ( bDown )
        {
            if ( nPos != mnCurPos )
            {
                mnCurPos = nPos;
                ImplDrawItem( mnCurPos, true );
                Flush();
            }
        }
        else
        {
            if ( nPos == mnCurPos )
            {
                ImplDrawItem( mnCurPos, false );
                Flush();
                mnCurPos = TOOLBOX_ITEM_NOTFOUND;
            }
        }

        if ( bRelease )
        {
            if ( mbDrag || mbSelection )
            {
                mbDrag = false;
                mbSelection = false;
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
            if ( (eState == TRISTATE_TRUE) && (pItem->mnBits & TIB_AUTOCHECK) &&
                 (pItem->mnBits & TIB_RADIOCHECK) )
            {
                ImplToolItem*    pGroupItem;
                sal_uInt16          nGroupPos;
                sal_uInt16          nItemCount = GetItemCount();

                nGroupPos = nPos;
                while ( nGroupPos )
                {
                    pGroupItem = &mpData->m_aItems[nGroupPos-1];
                    if ( pGroupItem->mnBits & TIB_RADIOCHECK )
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
                    if ( pGroupItem->mnBits & TIB_RADIOCHECK )
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
            ImplCallEventListeners( VCLEVENT_TOOLBOX_BUTTONSTATECHANGED, reinterpret_cast< void* >( nPos ) );

            // Notify
            //Solution:Call accessible listener to notify state_changed event
            ImplCallEventListeners( VCLEVENT_TOOLBOX_ITEMUPDATED, reinterpret_cast< void* >(nPos) );
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
            ImplCallEventListeners( VCLEVENT_TOOLBOX_BUTTONSTATECHANGED, reinterpret_cast< void* >( nPos ) );

            ImplCallEventListeners( bEnable ? VCLEVENT_TOOLBOX_ITEMENABLED : VCLEVENT_TOOLBOX_ITEMDISABLED, reinterpret_cast< void* >( nPos ) );
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
            ImplInvalidate( false );
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

const OUString& ToolBox::GetQuickHelpText( sal_uInt16 nItemId ) const
{
    static const OUString sEmpty;

    ImplToolItem* pItem = ImplGetItem( nItemId );

    if ( pItem )
        return pItem->maQuickHelpText;
    else
        return sEmpty;
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
        if ( !pItem->maRect.IsEmpty() )
            const_cast<ToolBox*>(this)->ImplDrawItem( i, false, false, true );
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

void ToolBox::SetDropdownClickHdl( const Link& rLink )
{
    mpData->maDropdownClickHdl = rLink;
}

const Link& ToolBox::GetDropdownClickHdl() const
{
    return mpData->maDropdownClickHdl;
}

void ToolBox::SetMenuType( sal_uInt16 aType )
{
    if( aType != mpData->maMenuType )
    {
        mpData->maMenuType = aType;
        if( IsFloatingMode() )
        {
            // the menu button may have to be moved into the decoration which changes the layout
            ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
            if( pWrapper )
                pWrapper->ShowTitleButton( TITLE_BUTTON_MENU, ( aType & TOOLBOX_MENUTYPE_CUSTOMIZE) ? true : false );

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

sal_uInt16 ToolBox::GetMenuType() const
{
    return mpData->maMenuType;
}

bool ToolBox::IsMenuEnabled() const
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

    // add menu items, starting from the end and inserting at pos 0
    if ( !mpData->m_aItems.empty() )
    {
        // nStartPos will hold the number of clipped items appended from first loop
        sal_uInt16 nSepPos = 0;
        for ( std::vector< ImplToolItem >::reverse_iterator it(mpData->m_aItems.rbegin());
                it != mpData->m_aItems.rend(); ++it)
        {
            if( it->IsClipped() )
            {
                sal_uInt16 id = it->mnId + TOOLBOX_MENUITEM_START;
                pMenu->InsertItem( id, it->maText, it->maImage, 0, OString(), 0 );
                pMenu->EnableItem( id, it->mbEnabled );
                pMenu->CheckItem ( id, it->meState == TRISTATE_TRUE );
                nSepPos++;
            }
        }

        // add a separator below the inserted clipped-items
        pMenu->InsertSeparator( OString(), nSepPos );

        // now append the items that are explicitly disabled
        for ( std::vector< ImplToolItem >::reverse_iterator it(mpData->m_aItems.rbegin());
                it != mpData->m_aItems.rend(); ++it)
        {
            if( it->IsItemHidden() )
            {
                sal_uInt16 id = it->mnId + TOOLBOX_MENUITEM_START;
                pMenu->InsertItem( id, it->maText, it->maImage, 0, OString(), nSepPos+1 );
                pMenu->EnableItem( id, it->mbEnabled );
                pMenu->CheckItem( id, it->meState == TRISTATE_TRUE );
            }
        }
    }
}

IMPL_LINK( ToolBox, ImplCustomMenuListener, VclMenuEvent*, pEvent )
{
    if( pEvent->GetMenu() == GetMenu() && pEvent->GetId() == VCLEVENT_MENU_SELECT )
    {
        sal_uInt16 id = GetMenu()->GetItemId( pEvent->GetItemPos() );
        if( id >= TOOLBOX_MENUITEM_START )
            TriggerItem( id - TOOLBOX_MENUITEM_START, false, false );
    }
    return 0;
}

IMPL_LINK_NOARG(ToolBox, ImplCallExecuteCustomMenu)
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

        sal_uInt16 uId = GetMenu()->Execute( pWin, Rectangle( ImplGetPopupPosition( aMenuRect, Size() ), Size() ),
                                POPUPMENU_EXECUTE_DOWN | POPUPMENU_NOMOUSEUPCLOSE );

        if ( aDelData.IsDead() )
            return;
        ImplRemoveDel( &aDelData );

        if( GetMenu() )
            GetMenu()->RemoveEventListener( LINK( this, ToolBox, ImplCustomMenuListener ) );
        if( bBorderDel )
        {
            if( aBorderDel.IsDead() )
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
        UpdateCustomMenu();
        Application::PostUserEvent( mpData->mnEventId, LINK( this, ToolBox, ImplCallExecuteCustomMenu ) );
    }
}

// checks override first, useful during calculation of sizes
bool ToolBox::ImplIsFloatingMode() const
{
    DBG_ASSERT( !(mpData->mbAssumeDocked && mpData->mbAssumeFloating),
        "ToolBox::ImplIsFloatingMode(): cannot assume docked and floating" );

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
            OUString("/org.openoffice.Office.UI.GlobalSettings/Toolbars") );    // note: case sensitive !
        if ( aNode.isValid() )
        {
            // feature enabled ?
            bool bStatesEnabled = bool();
            ::com::sun::star::uno::Any aValue = aNode.getNodeValue( OUString("StatesEnabled") );
            if( aValue >>= bStatesEnabled )
            {
                if( bStatesEnabled )
                {
                    // now read the locking state
                    utl::OConfigurationNode aNode2 = utl::OConfigurationTreeRoot::tryCreateWithComponentContext(
                        comphelper::getProcessComponentContext(),
                        OUString("/org.openoffice.Office.UI.GlobalSettings/Toolbars/States") );    // note: case sensitive !

                    bool bLocked = bool();
                    ::com::sun::star::uno::Any aValue2 = aNode2.getNodeValue( OUString("Locked") );
                    if( aValue2 >>= bLocked )
                        nAlwaysLocked = bLocked ? 1 : 0;
                }
            }
        }
    }

    return nAlwaysLocked == 1 ? true : false;
}

bool ToolBox::WillUsePopupMode() const
{
    return mpData->mbWillUsePopupMode;
}

void ToolBox::WillUsePopupMode( bool b )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
