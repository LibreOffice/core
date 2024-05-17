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

#include "FullScreenPane.hxx"
#include <vcl/vclevent.hxx>
#include <vcl/wrkwin.hxx>
#include <o3tl/string_view.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <strings.hrc>
#include <sdresid.hxx>
#include <DrawDocShell.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd::framework {

FullScreenPane::FullScreenPane (
    const Reference<XComponentContext>& rxComponentContext,
    const Reference<XResourceId>& rxPaneId,
    const vcl::Window* pViewShellWindow,
    const DrawDocShell* pDrawDocShell)
    : FrameWindowPane(rxPaneId,nullptr),
      mxComponentContext(rxComponentContext)
{
    sal_Int32 nScreenNumber = 1;
    bool bFullScreen = true;
    ExtractArguments(rxPaneId, nScreenNumber, bFullScreen);

    vcl::Window* pParent = nullptr;
    WinBits nStyle = bFullScreen ? 0 : (WB_BORDER | WB_MOVEABLE | WB_SIZEABLE);

    mpWorkWindow.reset(VclPtr<WorkWindow>::Create(
        pParent,
        nStyle));  // For debugging (non-fullscreen) use WB_BORDER | WB_MOVEABLE | WB_SIZEABLE));

    if ( ! rxPaneId.is())
        throw lang::IllegalArgumentException();

    if (!mpWorkWindow)
        return;

    // Create a new top-level window that is displayed full screen.
    if (bFullScreen)
        mpWorkWindow->ShowFullScreenMode(bFullScreen, nScreenNumber);

    // For debugging (non-fullscreen) use mpWorkWindow->SetScreenNumber(nScreenNumber);
    mpWorkWindow->SetMenuBarMode(MenuBarMode::Hide);
    mpWorkWindow->SetBorderStyle(WindowBorderStyle::REMOVEBORDER);
    mpWorkWindow->SetBackground(Wallpaper());
    // Don't show the window right now in order to allow the setting of an
    // accessibility object: accessibility objects are typically
    // requested by AT-tools when the window is shown. Chaining it
    // afterwards may or may not work.

    // Add resize listener at the work window.
    Link<VclWindowEvent&,void> aWindowEventHandler (LINK(this, FullScreenPane, WindowEventHandler));
    mpWorkWindow->AddEventListener(aWindowEventHandler);

    // Set title and icon of the new window to those of the current window
    // of the view shell.
    if (pViewShellWindow != nullptr && pDrawDocShell != nullptr)
    {
        SystemWindow* pSystemWindow = pViewShellWindow->GetSystemWindow();
        OUString Title(SdResId(STR_FULLSCREEN_CONSOLE));
        Title = Title.replaceFirst("%s", pDrawDocShell->GetTitle(SFX_TITLE_DETECT));
        mpWorkWindow->SetText(Title);
        mpWorkWindow->SetIcon(pSystemWindow->GetIcon());
    }

    // For some reason the VCL canvas can not paint into a WorkWindow.
    // Therefore a child window is created that covers the WorkWindow
    // completely.
    mpWindow = VclPtr<vcl::Window>::Create(mpWorkWindow.get());
    mpWindow->SetPosSizePixel(Point(0,0), mpWorkWindow->GetSizePixel());
    mpWindow->SetBackground(Wallpaper());
    mxWindow = VCLUnoHelper::GetInterface(mpWindow);

    // Create the canvas.
    mxCanvas = CreateCanvas();

    mpWindow->GrabFocus();
}

FullScreenPane::~FullScreenPane() noexcept
{
}

void SAL_CALL FullScreenPane::disposing()
{
    mpWindow.disposeAndClear();

    if (mpWorkWindow)
    {
        Link<VclWindowEvent&,void> aWindowEventHandler (LINK(this, FullScreenPane, WindowEventHandler));
        mpWorkWindow->RemoveEventListener(aWindowEventHandler);
        mpWorkWindow.disposeAndClear();
    }

    FrameWindowPane::disposing();
}

//----- XPane -----------------------------------------------------------------

sal_Bool SAL_CALL FullScreenPane::isVisible()
{
    ThrowIfDisposed();

    if (mpWindow != nullptr)
        return mpWindow->IsReallyVisible();
    else
        return false;
}

void SAL_CALL FullScreenPane::setVisible (const sal_Bool bIsVisible)
{
    ThrowIfDisposed();

    if (mpWindow != nullptr)
        mpWindow->Show(bIsVisible);
    if (mpWorkWindow != nullptr)
        mpWorkWindow->Show(bIsVisible);
}

Reference<css::accessibility::XAccessible> SAL_CALL FullScreenPane::getAccessible()
{
    ThrowIfDisposed();

    if (mpWorkWindow != nullptr)
        return mpWorkWindow->GetAccessible(false);
    else
        return nullptr;
}

void SAL_CALL FullScreenPane::setAccessible (
    const Reference<css::accessibility::XAccessible>& rxAccessible)
{
    ThrowIfDisposed();

    if (mpWindow == nullptr)
        return;

    Reference<lang::XInitialization> xInitializable (rxAccessible, UNO_QUERY);
    if (xInitializable.is())
    {
        vcl::Window* pParentWindow = mpWindow->GetParent();
        Reference<css::accessibility::XAccessible> xAccessibleParent;
        if (pParentWindow != nullptr)
            xAccessibleParent = pParentWindow->GetAccessible();
        Sequence<Any> aArguments{ Any(xAccessibleParent) };
        xInitializable->initialize(aArguments);
    }
    GetWindow()->SetAccessible(rxAccessible);
}

IMPL_LINK(FullScreenPane, WindowEventHandler, VclWindowEvent&, rEvent, void)
{
    switch (rEvent.GetId())
    {
        case VclEventId::WindowResize:
            GetWindow()->SetPosPixel(Point(0,0));
            GetWindow()->SetSizePixel(Size(
                mpWorkWindow->GetSizePixel().Width(),
                mpWorkWindow->GetSizePixel().Height()));
            break;

        case VclEventId::ObjectDying:
            mpWorkWindow.disposeAndClear();
            break;

        default: break;
    }
}

Reference<rendering::XCanvas> FullScreenPane::CreateCanvas()
{
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(mxWindow);
    if (!pWindow)
        throw RuntimeException();

    Sequence<Any> aArg{ // common: first any is VCL pointer to window (for VCL canvas)
                        Any(reinterpret_cast<sal_Int64>(pWindow.get())),
                        Any(css::awt::Rectangle()),
                        Any(false),
                        Any(mxWindow)
    };

    Reference<lang::XMultiServiceFactory> xFactory (
        mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
    return Reference<rendering::XCanvas>(
        xFactory->createInstanceWithArguments(u"com.sun.star.rendering.SpriteCanvas.VCL"_ustr,
            aArg),
        UNO_QUERY);
}

void FullScreenPane::ExtractArguments (
    const Reference<XResourceId>& rxPaneId,
    sal_Int32& rnScreenNumberReturnValue,
    bool& rbFullScreen)
{
    // Extract arguments from the resource URL.
    const util::URL aURL = rxPaneId->getFullResourceURL();
    for (sal_Int32 nIndex{ 0 }; nIndex >= 0; )
    {
        const std::u16string_view aToken = o3tl::getToken(aURL.Arguments, 0, '&', nIndex);
        std::u16string_view sValue;
        if (o3tl::starts_with(aToken, u"ScreenNumber=", &sValue))
        {
            rnScreenNumberReturnValue = o3tl::toInt32(sValue);
        }
        if (o3tl::starts_with(aToken, u"FullScreen=", &sValue))
        {
            rbFullScreen = o3tl::equalsAscii(sValue, "true");
        }
    }
}

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
