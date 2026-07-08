/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <vcl/cvtgrf.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/event.hxx>
#include <vcl/graph.hxx>
#include <vcl/image.hxx>
#include <vcl/toolkit/floatwin.hxx>
#include <vcl/menu.hxx>
#include <vcl/timer.hxx>
#include <vcl/toolkit/MenuButton.hxx>
#include <vcl/settings.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>
#include <comphelper/base64.hxx>
#include <cpo/uno/Sequence.hxx>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <menutogglebutton.hxx>
#include <tools/json_writer.hxx>
#include <tools/stream.hxx>
#include <algorithm>

namespace
{
void collectUIInformation( const OUString& aID, const OUString& aevent , const OUString& akey , const OUString& avalue)
{
    EventDescription aDescription;
    aDescription.aID = aID;
    aDescription.aParameters = {{ akey ,  avalue}};
    aDescription.aAction = aevent;
    aDescription.aParent = u"MainWindow"_ustr;
    aDescription.aKeyWord = u"MenuButton"_ustr;
    UITestLogger::getInstance().logEvent(aDescription);
}

// Encode a menu item image as a PNG data URL so the client can show it
// directly as the source of an image, rather than resolving an icon name.
// The bitmap is shrunk to fit within 16 by 16 pixels, keeping its aspect
// ratio, so the transported data stays small and the icons share one size.
OString lclImageToDataUrl(const Image& rImage)
{
    Bitmap aBitmap = rImage.GetBitmap();
    Size aSize = aBitmap.GetSizePixel();
    constexpr double fImgSize = 16.0;
    if (aSize.Width() > 0 && aSize.Height() > 0)
    {
        const double fScale = std::min(fImgSize / aSize.Width(), fImgSize / aSize.Height());
        aBitmap.Scale(fScale, fScale, BmpScaleFlag::BestQuality);
    }

    SvMemoryStream aStream;
    if (GraphicConverter::Export(aStream, aBitmap, ConvertDataFormat::PNG) != ERRCODE_NONE)
        return OString();

    cpo::uno::Sequence<sal_Int8> aData(static_cast<sal_Int8 const*>(aStream.GetData()),
                                       aStream.Tell());
    OStringBuffer aBuffer("data:image/png;base64,");
    comphelper::Base64::encode(aBuffer, aData);
    return aBuffer.makeStringAndClear();
}

// Encode a menu item's vector graphic as an SVG data URL. A graphic that is
// already an SVG, such as a gallery bullet icon, has its original bytes written
// unchanged; a drawn graphic such as a metafile symbol is exported to SVG.
// The client renders the result crisply at any size instead of a fixed bitmap.
OString lclGraphicToSvgDataUrl(const css::uno::Reference<css::graphic::XGraphic>& rGraphic)
{
    if (!rGraphic.is())
        return OString();

    Graphic aGraphic(rGraphic);
    SvMemoryStream aStream;
    if (GraphicConverter::Export(aStream, aGraphic, ConvertDataFormat::SVG) != ERRCODE_NONE)
        return OString();

    cpo::uno::Sequence<sal_Int8> aData(static_cast<sal_Int8 const*>(aStream.GetData()),
                                       aStream.Tell());
    OStringBuffer aBuffer("data:image/svg+xml;base64,");
    comphelper::Base64::encode(aBuffer, aData);
    return aBuffer.makeStringAndClear();
}

// Write the items of rMenu into the JSON array that is currently open.
// An item with an image gets an "img" data URL so the client shows it. When the
// item carries a vector graphic the URL is SVG; otherwise the item's bitmap is
// encoded as PNG. An item that has a submenu gets an "items" array holding that
// submenu's entries, so nested menus reach the client as nested arrays.
void lclDumpMenuEntries(tools::JsonWriter& rJsonWriter, const Menu& rMenu)
{
    for (sal_uInt16 i = 0; i < rMenu.GetItemCount(); ++i)
    {
        auto aEntry = rJsonWriter.startStruct();
        sal_uInt16 nId = rMenu.GetItemId(i);
        rJsonWriter.put("id", rMenu.GetItemIdent(nId));
        rJsonWriter.put("text", rMenu.GetItemText(nId));
        OString aDataUrl = lclGraphicToSvgDataUrl(rMenu.GetItemImageGraphic(nId));
        if (aDataUrl.isEmpty())
        {
            Image aImage = rMenu.GetItemImage(nId);
            if (!!aImage)
                aDataUrl = lclImageToDataUrl(aImage);
        }
        if (!aDataUrl.isEmpty())
            rJsonWriter.put("img", aDataUrl);
        if (PopupMenu* pSubMenu = rMenu.GetPopupMenu(nId))
        {
            auto aItems = rJsonWriter.startArray("items");
            lclDumpMenuEntries(rJsonWriter, *pSubMenu);
        }
    }
}
}

void MenuButton::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;

    PushButton::ImplInit( pParent, nStyle );
    EnableRTL( AllSettings::GetLayoutRTL() );
}

void MenuButton::ExecuteMenu()
{
    mbStartingMenu = true;

    PrepareExecute();

    if (!mpMenu && !mpFloatingWindow)
    {
        mbStartingMenu = false;
        return;
    }

    Size aSize = GetSizePixel();
    SetPressed( true );
    EndSelection();
    if (mpMenu)
    {
        Point aPos(0, 1);
        tools::Rectangle aRect(aPos, aSize );
        mpMenu->Execute(this, aRect, PopupMenuFlags::ExecuteDown);

        if (isDisposed())
            return;

        mnCurItemId = mpMenu->GetCurItemId();
        msCurItemIdent = mpMenu->GetCurItemIdent();
    }
    else
    {
        Point aPos(GetParent()->OutputToScreenPixel(GetPosPixel()));
        tools::Rectangle aRect(aPos, aSize );
        FloatWinPopupFlags nFlags = FloatWinPopupFlags::Down | FloatWinPopupFlags::GrabFocus;
        if (mpFloatingWindow->GetType() == WindowType::FLOATINGWINDOW)
            static_cast<FloatingWindow*>(mpFloatingWindow.get())->StartPopupMode(aRect, nFlags);
        else
        {
            mpFloatingWindow->EnableDocking();
            vcl::Window::GetDockingManager()->StartPopupMode(mpFloatingWindow, aRect, nFlags);
        }
    }

    Activate();

    mbStartingMenu = false;

    SetPressed(false);
    OUString aID = get_id(); // tdf#136678 take a copy if we are destroyed by Select callback
    if (mnCurItemId)
    {
        Select();
        mnCurItemId = 0;
        msCurItemIdent.clear();
    }
    collectUIInformation(aID,u"OPENLIST"_ustr,u""_ustr,u""_ustr);
}

void MenuButton::CancelMenu()
{
    if (!mpMenu && !mpFloatingWindow)
        return;

    if (mpMenu)
    {
        mpMenu->EndExecute();
    }
    else
    {
        if (mpFloatingWindow->GetType() == WindowType::FLOATINGWINDOW)
            static_cast<FloatingWindow*>(mpFloatingWindow.get())->EndPopupMode();
        else
            vcl::Window::GetDockingManager()->EndPopupMode(mpFloatingWindow);
    }
    collectUIInformation(get_id(),u"CLOSELIST"_ustr,u""_ustr,u""_ustr);
}

bool MenuButton::InPopupMode() const
{
    if (mbStartingMenu)
        return true;

    if (!mpMenu && !mpFloatingWindow)
        return false;

    if (mpMenu)
       return PopupMenu::GetActivePopupMenu() == mpMenu;
    else
    {
        if (mpFloatingWindow->GetType() == WindowType::FLOATINGWINDOW)
            return static_cast<const FloatingWindow*>(mpFloatingWindow.get())->IsInPopupMode();
        else
            return vcl::Window::GetDockingManager()->IsInPopupMode(mpFloatingWindow);
    }
}

MenuButton::MenuButton( vcl::Window* pParent, WinBits nWinBits )
    : PushButton(WindowType::MENUBUTTON)
    , mnCurItemId(0)
    , mbStartingMenu(false)
{
    mnDDStyle = PushButtonDropdownStyle::MenuButton;
    ImplInit(pParent, nWinBits);
}

MenuButton::~MenuButton()
{
    disposeOnce();
}

void MenuButton::dispose()
{
    mpFloatingWindow.reset();
    if (mpMenu && mbOwnPopupMenu)
        mpMenu->dispose();
    mpMenu.reset();
    PushButton::dispose();
}

void MenuButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( PushButton::ImplHitTestPushButton( this, rMEvt.GetPosPixel() ) )
    {
        if ( !(GetStyle() & WB_NOPOINTERFOCUS) )
            GrabFocus();
        ExecuteMenu();
    }
}

void MenuButton::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = aKeyCode.GetCode();
    if ( (nCode == KEY_DOWN) && aKeyCode.IsMod2() )
        ExecuteMenu();
    else if ( !aKeyCode.GetModifier() &&
              ((nCode == KEY_RETURN) || (nCode == KEY_SPACE)) )
        ExecuteMenu();
    else
        PushButton::KeyInput( rKEvt );
}

void MenuButton::Activate()
{
    maActivateHdl.Call( this );
}

void MenuButton::Select()
{
    if (mnCurItemId)
        collectUIInformation(get_id(),u"OPENFROMLIST"_ustr,u"POS"_ustr,OUString::number(mnCurItemId));

    maSelectHdl.Call( this );
}

void MenuButton::SetPopupMenu(PopupMenu* pNewMenu, bool bTakeOwnership)
{
    if (pNewMenu == mpMenu)
        return;

    if (mpMenu && mbOwnPopupMenu)
        mpMenu->dispose();

    mpMenu = pNewMenu;
    mbOwnPopupMenu = bTakeOwnership;
}

void MenuButton::SetPopover(Window* pWindow)
{
    if (pWindow == mpFloatingWindow)
        return;

    mpFloatingWindow = pWindow;
}


FactoryFunction MenuButton::GetUITestFactory() const
{
    return MenuButtonUIObject::create;
}

void MenuButton::SetCurItemId(){
    mnCurItemId = mpMenu->GetCurItemId();
    msCurItemIdent = mpMenu->GetCurItemIdent();
}

void MenuButton::DumpAsPropertyTree(tools::JsonWriter& rJsonWriter)
{
    PushButton::DumpAsPropertyTree(rJsonWriter);

    if (mpMenu)
    {
        auto aMenuNode = rJsonWriter.startArray("menu");
        lclDumpMenuEntries(rJsonWriter, *mpMenu);
    }
}

//class MenuToggleButton ----------------------------------------------------

MenuToggleButton::MenuToggleButton( vcl::Window* pParent, WinBits nWinBits )
    : MenuButton( pParent, nWinBits )
{
}

MenuToggleButton::~MenuToggleButton()
{
    disposeOnce();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
