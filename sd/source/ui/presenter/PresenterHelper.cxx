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

#include <cstddef>

#include <PresenterHelper.hxx>
#include "PresenterCanvas.hxx"
#include <cppcanvas/vclfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>


#include <bitmaps.hlst>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd::presenter {

//===== PresenterHelper =======================================================

PresenterHelper::PresenterHelper (
    const Reference<XComponentContext>& rxContext)
    : mxComponentContext(rxContext)
{
}

PresenterHelper::~PresenterHelper()
{
}

//----- XInitialize -----------------------------------------------------------

void SAL_CALL PresenterHelper::initialize (const Sequence<Any>&) {}


Reference<awt::XWindow> PresenterHelper::createWindow (
    const Reference<awt::XWindow>& rxParentWindow,
    bool bCreateSystemChildWindow,
    bool bInitiallyVisible,
    bool bEnableChildTransparentMode,
    bool bEnableParentClip)
{
    VclPtr<vcl::Window> pParentWindow(VCLUnoHelper::GetWindow(rxParentWindow));

    // Create a new window.
    VclPtr<vcl::Window> pWindow;
    if (bCreateSystemChildWindow)
    {
        pWindow = VclPtr<WorkWindow>::Create(pParentWindow, WB_SYSTEMCHILDWINDOW);
    }
    else
    {
        pWindow = VclPtr<vcl::Window>::Create(pParentWindow);
    }
    Reference<awt::XWindow> xWindow (pWindow->GetComponentInterface(), UNO_QUERY);

    if (bEnableChildTransparentMode)
    {
        // Make the frame window transparent and make the parent able to
        // draw behind it.
        if (pParentWindow)
            pParentWindow->EnableChildTransparentMode();
    }

    pWindow->Show(bInitiallyVisible);

    pWindow->SetMapMode(MapMode(MapUnit::MapPixel));
    pWindow->SetBackground();
    if ( ! bEnableParentClip)
    {
        pWindow->SetParentClipMode(ParentClipMode::NoClip);
        pWindow->SetPaintTransparent(true);
    }
    else
    {
        pWindow->SetParentClipMode(ParentClipMode::Clip);
        pWindow->SetPaintTransparent(false);
    }

    return xWindow;
}

Reference<rendering::XCanvas> PresenterHelper::createSharedCanvas (
    const Reference<rendering::XSpriteCanvas>& rxUpdateCanvas,
    const Reference<awt::XWindow>& rxUpdateWindow,
    const Reference<rendering::XCanvas>& rxSharedCanvas,
    const Reference<awt::XWindow>& rxSharedWindow,
    const Reference<awt::XWindow>& rxWindow)
{
    if ( ! rxSharedCanvas.is()
        || ! rxSharedWindow.is()
        || ! rxWindow.is())
    {
        throw RuntimeException(u"illegal argument"_ustr);
    }

    if (rxWindow == rxSharedWindow)
        return rxSharedCanvas;
    else
        return new PresenterCanvas(
            rxUpdateCanvas,
            rxUpdateWindow,
            rxSharedCanvas,
            rxSharedWindow,
            rxWindow);
}

Reference<rendering::XCanvas> PresenterHelper::createCanvas (
    const Reference<awt::XWindow>& rxWindow,
    sal_Int16,
    const OUString& rsOptionalCanvasServiceName)
{
    // No shared window is given or an explicit canvas service name is
    // specified.  Create a new canvas.
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(rxWindow);
    if (!pWindow)
        throw RuntimeException();

    Sequence<Any> aArg{ // common: first any is VCL pointer to window (for VCL canvas)
                        Any(reinterpret_cast<sal_Int64>(pWindow.get())),
                        Any(css::awt::Rectangle()),
                        Any(false),
                        Any(rxWindow)
    };

    Reference<lang::XMultiServiceFactory> xFactory (
        mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
    return Reference<rendering::XCanvas>(
        xFactory->createInstanceWithArguments(
            !rsOptionalCanvasServiceName.isEmpty()
                ? rsOptionalCanvasServiceName
                : u"com.sun.star.rendering.Canvas.VCL"_ustr,
            aArg),
        UNO_QUERY);
}

void PresenterHelper::toTop(const Reference<awt::XWindow>& rxWindow)
{
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(rxWindow);
    if (pWindow)
    {
        pWindow->ToTop();
        pWindow->SetZOrder(nullptr, ZOrderFlags::Last);
    }
}

namespace {

struct IdMapEntry {
    OUString sid;
    OUString bmpid;
};

}

Reference<rendering::XBitmap> PresenterHelper::loadBitmap (
    std::u16string_view sId,
    const Reference<rendering::XCanvas>& rxCanvas)
{
    if ( ! rxCanvas.is())
        return nullptr;

    static IdMapEntry constexpr map[] = {
        { u"bitmaps/Background.png"_ustr, BMP_PRESENTERSCREEN_BACKGROUND },
        { u"bitmaps/Animation.png"_ustr,
          BMP_PRESENTERSCREEN_ANIMATION },
        { u"bitmaps/Transition.png"_ustr,
          BMP_PRESENTERSCREEN_TRANSITION },
        { u"bitmaps/BorderActiveBottom.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_BOTTOM },
        { u"bitmaps/BorderActiveBottomCallout.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_BOTTOM_CALLOUT },
        { u"bitmaps/BorderActiveBottomLeft.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_BOTTOM_LEFT },
        { u"bitmaps/BorderActiveBottomRight.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_BOTTOM_RIGHT },
        { u"bitmaps/BorderActiveLeft.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_LEFT },
        { u"bitmaps/BorderActiveRight.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_RIGHT },
        { u"bitmaps/BorderActiveTop.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_TOP },
        { u"bitmaps/BorderActiveTopLeft.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_TOP_LEFT },
        { u"bitmaps/BorderActiveTopRight.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_TOP_RIGHT },
        { u"bitmaps/BorderBottom.png"_ustr, BMP_PRESENTERSCREEN_BORDER_BOTTOM },
        { u"bitmaps/BorderBottomLeft.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_BOTTOM_LEFT },
        { u"bitmaps/BorderBottomRight.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_BOTTOM_RIGHT },
        { u"bitmaps/BorderCurrentSlideBottom.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_BOTTOM },
        { u"bitmaps/BorderCurrentSlideBottomLeft.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_BOTTOM_LEFT },
        { u"bitmaps/BorderCurrentSlideBottomRight.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_BOTTOM_RIGHT },
        { u"bitmaps/BorderCurrentSlideLeft.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_LEFT },
        { u"bitmaps/BorderCurrentSlideRight.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_RIGHT },
        { u"bitmaps/BorderCurrentSlideTop.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_TOP },
        { u"bitmaps/BorderCurrentSlideTopLeft.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_TOP_LEFT },
        { u"bitmaps/BorderCurrentSlideTopRight.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_TOP_RIGHT },
        { u"bitmaps/BorderLeft.png"_ustr, BMP_PRESENTERSCREEN_BORDER_LEFT },
        { u"bitmaps/BorderRight.png"_ustr, BMP_PRESENTERSCREEN_BORDER_RIGHT },
        { u"bitmaps/BorderToolbarBottom.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_TOOLBAR_BOTTOM },
        { u"bitmaps/BorderToolbarLeft.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_TOOLBAR_LEFT },
        { u"bitmaps/BorderToolbarRight.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_TOOLBAR_RIGHT },
        { u"bitmaps/BorderToolbarTop.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_TOOLBAR_TOP },
        { u"bitmaps/BorderToolbarTopLeft.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_TOOLBAR_TOP_LEFT },
        { u"bitmaps/BorderToolbarTopRight.png"_ustr,
          BMP_PRESENTERSCREEN_BORDER_TOOLBAR_TOP_RIGHT },
        { u"bitmaps/BorderTop.png"_ustr, BMP_PRESENTERSCREEN_BORDER_TOP },
        { u"bitmaps/BorderTopLeft.png"_ustr, BMP_PRESENTERSCREEN_BORDER_TOP_LEFT },
        { u"bitmaps/BorderTopRight.png"_ustr, BMP_PRESENTERSCREEN_BORDER_TOP_RIGHT },
        { u"bitmaps/ButtonEffectNextDisabled.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_EFFECT_NEXT_DISABLED },
        { u"bitmaps/ButtonEffectNextMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_EFFECT_NEXT_MOUSE_OVER },
        { u"bitmaps/ButtonEffectNextNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_EFFECT_NEXT_NORMAL },
        { u"bitmaps/ButtonEffectNextSelected.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_EFFECT_NEXT_SELECTED },
        { u"bitmaps/ButtonFrameCenterMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_FRAME_CENTER_MOUSE_OVER },
        { u"bitmaps/ButtonFrameCenterNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_FRAME_CENTER_NORMAL },
        { u"bitmaps/ButtonFrameLeftMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_FRAME_LEFT_MOUSE_OVER },
        { u"bitmaps/ButtonFrameLeftNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_FRAME_LEFT_NORMAL },
        { u"bitmaps/ButtonFrameRightMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_FRAME_RIGHT_MOUSE_OVER },
        { u"bitmaps/ButtonFrameRightNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_FRAME_RIGHT_NORMAL },
        { u"bitmaps/ButtonHelpDisabled.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_HELP_DISABLED },
        { u"bitmaps/ButtonHelpMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_HELP_MOUSE_OVER },
        { u"bitmaps/ButtonHelpNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_HELP_NORMAL },
        { u"bitmaps/ButtonHelpSelected.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_HELP_SELECTED },
        { u"bitmaps/ButtonExitPresenterMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_EXIT_PRESENTER_MOUSE_OVER },
        { u"bitmaps/ButtonExitPresenterNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_EXIT_PRESENTER_NORMAL },
        { u"bitmaps/ButtonMinusDisabled.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_MINUS_DISABLED },
        { u"bitmaps/ButtonMinusMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_MINUS_MOUSE_OVER },
        { u"bitmaps/ButtonMinusNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_MINUS_NORMAL },
        { u"bitmaps/ButtonMinusSelected.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_MINUS_SELECTED },
        { u"bitmaps/ButtonNotesDisabled.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_NOTES_DISABLED },
        { u"bitmaps/ButtonNotesMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_NOTES_MOUSE_OVER },
        { u"bitmaps/ButtonNotesNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_NOTES_NORMAL },
        { u"bitmaps/ButtonNotesSelected.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_NOTES_SELECTED },
        { u"bitmaps/ButtonPlusDisabled.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_PLUS_DISABLED },
        { u"bitmaps/ButtonPlusMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_PLUS_MOUSE_OVER },
        { u"bitmaps/ButtonPlusNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_PLUS_NORMAL },
        { u"bitmaps/ButtonPlusSelected.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_PLUS_SELECTED },
        { u"bitmaps/ButtonSlideNextDisabled.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_NEXT_DISABLED },
        { u"bitmaps/ButtonSlideNextMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_NEXT_MOUSE_OVER },
        { u"bitmaps/ButtonSlideNextNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_NEXT_NORMAL },
        { u"bitmaps/ButtonSlidePreviousDisabled.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_PREVIOUS_DISABLED },
        { u"bitmaps/ButtonSlidePreviousMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_PREVIOUS_MOUSE_OVER },
        { u"bitmaps/ButtonSlidePreviousNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_PREVIOUS_NORMAL },
        { u"bitmaps/ButtonSlidePreviousSelected.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_PREVIOUS_SELECTED },
        { u"bitmaps/ButtonSlideSorterDisabled.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_SORTER_DISABLED },
        { u"bitmaps/ButtonSlideSorterMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_SORTER_MOUSE_OVER },
        { u"bitmaps/ButtonSlideSorterNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_SORTER_NORMAL },
        { u"bitmaps/ButtonSlideSorterSelected.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_SORTER_SELECTED },
        { u"bitmaps/ButtonSwitchMonitorMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_SWITCH_MONITOR_MOUSE_OVER },
        { u"bitmaps/ButtonSwitchMonitorNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_SWITCH_MONITOR_NORMAL },
        { u"bitmaps/ButtonRestartTimerMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_RESTART_TIMER_MOUSE_OVER },
        { u"bitmaps/ButtonRestartTimerNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_RESTART_TIMER_NORMAL },
        { u"bitmaps/ButtonPauseTimerMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_PAUSE_TIMER_MOUSE_OVER },
        { u"bitmaps/ButtonPauseTimerNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_PAUSE_TIMER_NORMAL },
        { u"bitmaps/ButtonResumeTimerMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_RESUME_TIMER_MOUSE_OVER },
        { u"bitmaps/ButtonResumeTimerNormal.png"_ustr,
          BMP_PRESENTERSCREEN_BUTTON_RESUME_TIMER_NORMAL },
        { u"bitmaps/LabelMouseOverCenter.png"_ustr,
          BMP_PRESENTERSCREEN_LABEL_MOUSE_OVER_CENTER },
        { u"bitmaps/LabelMouseOverLeft.png"_ustr,
          BMP_PRESENTERSCREEN_LABEL_MOUSE_OVER_LEFT },
        { u"bitmaps/LabelMouseOverRight.png"_ustr,
          BMP_PRESENTERSCREEN_LABEL_MOUSE_OVER_RIGHT },
        { u"bitmaps/ScrollbarArrowDownDisabled.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_DOWN_DISABLED },
        { u"bitmaps/ScrollbarArrowDownMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_DOWN_MOUSE_OVER },
        { u"bitmaps/ScrollbarArrowDownNormal.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_DOWN_NORMAL },
        { u"bitmaps/ScrollbarArrowDownSelected.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_DOWN_SELECTED },
        { u"bitmaps/ScrollbarArrowUpDisabled.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_UP_DISABLED },
        { u"bitmaps/ScrollbarArrowUpMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_UP_MOUSE_OVER },
        { u"bitmaps/ScrollbarArrowUpNormal.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_UP_NORMAL },
        { u"bitmaps/ScrollbarArrowUpSelected.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_UP_SELECTED },
        { u"bitmaps/ScrollbarPagerMiddleMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_PAGER_MIDDLE_MOUSE_OVER },
        { u"bitmaps/ScrollbarPagerMiddleNormal.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_PAGER_MIDDLE_NORMAL },
        { u"bitmaps/ScrollbarThumbBottomMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_THUMB_BOTTOM_MOUSE_OVER },
        { u"bitmaps/ScrollbarThumbBottomNormal.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_THUMB_BOTTOM_NORMAL },
        { u"bitmaps/ScrollbarThumbMiddleMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_THUMB_MIDDLE_MOUSE_OVER },
        { u"bitmaps/ScrollbarThumbMiddleNormal.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_THUMB_MIDDLE_NORMAL },
        { u"bitmaps/ScrollbarThumbTopMouseOver.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_THUMB_TOP_MOUSE_OVER },
        { u"bitmaps/ScrollbarThumbTopNormal.png"_ustr,
          BMP_PRESENTERSCREEN_SCROLLBAR_THUMB_TOP_NORMAL },
        { u"bitmaps/ViewBackground.png"_ustr, BMP_PRESENTERSCREEN_VIEW_BACKGROUND },
        { u"bitmaps/Separator.png"_ustr,
          BMP_PRESENTERSCREEN_SEPARATOR }
    };
    OUString bmpid;
    for (std::size_t i = 0; i != SAL_N_ELEMENTS(map); ++i) {
        if (sId == map[i].sid) {
            bmpid = map[i].bmpid;
            break;
        }
    }
    if (bmpid.isEmpty()) {
        return nullptr;
    }

    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    const cppcanvas::CanvasSharedPtr pCanvas (
        cppcanvas::VCLFactory::createCanvas(rxCanvas));

    if (pCanvas)
    {
        BitmapEx aBitmapEx(bmpid);
        cppcanvas::BitmapSharedPtr xBitmap(
            cppcanvas::VCLFactory::createBitmap(pCanvas,
                aBitmapEx));
        if (!xBitmap)
            return nullptr;
        return xBitmap->getUNOBitmap();
    }

    return nullptr;
}

void PresenterHelper::captureMouse (
    const Reference<awt::XWindow>& rxWindow)
{
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    // Capture the mouse (if not already done.)
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(rxWindow);
    if (pWindow && ! pWindow->IsMouseCaptured())
    {
        pWindow->CaptureMouse();
    }
}

void PresenterHelper::releaseMouse (const Reference<awt::XWindow>& rxWindow)
{
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    // Release the mouse (if not already done.)
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(rxWindow);
    if (pWindow && pWindow->IsMouseCaptured())
    {
        pWindow->ReleaseMouse();
    }
}

awt::Rectangle PresenterHelper::getWindowExtentsRelative (
    const Reference<awt::XWindow>& rxChildWindow,
    const Reference<awt::XWindow>& rxParentWindow)
{
    VclPtr<vcl::Window> pChildWindow = VCLUnoHelper::GetWindow(rxChildWindow);
    VclPtr<vcl::Window> pParentWindow = VCLUnoHelper::GetWindow(rxParentWindow);
    if (pChildWindow && pParentWindow)
    {
        ::tools::Rectangle aBox (pChildWindow->GetWindowExtentsRelative(*pParentWindow));
        return awt::Rectangle(aBox.Left(),aBox.Top(),aBox.GetWidth(),aBox.GetHeight());
    }
    else
        return awt::Rectangle();
}

} // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
