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

#include <cstddef>

#include "PresenterHelper.hxx"
#include "CanvasUpdateRequester.hxx"
#include "PresenterCanvas.hxx"
#include "facreg.hxx"
#include <cppcanvas/vclfactory.hxx>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/WindowClass.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>

#include "res_bmp.hrc"
#include "sdresid.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd { namespace presenter {

//===== PresenterHelper =======================================================

PresenterHelper::PresenterHelper (
    const Reference<XComponentContext>& rxContext)
    : PresenterHelperInterfaceBase(m_aMutex),
      mxComponentContext(rxContext)
{
}

PresenterHelper::~PresenterHelper()
{
}

//----- XInitialize -----------------------------------------------------------

void SAL_CALL PresenterHelper::initialize (const Sequence<Any>& rArguments)
    throw(Exception,RuntimeException, std::exception)
{
    (void)rArguments;
}

//----- XPaneHelper ----------------------------------------------------

Reference<awt::XWindow> SAL_CALL PresenterHelper::createWindow (
    const Reference<awt::XWindow>& rxParentWindow,
    sal_Bool bCreateSystemChildWindow,
    sal_Bool bInitiallyVisible,
    sal_Bool bEnableChildTransparentMode,
    sal_Bool bEnableParentClip)
    throw (css::uno::RuntimeException, std::exception)
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
        if (pParentWindow.get() != nullptr)
            pParentWindow->EnableChildTransparentMode();
    }

    pWindow->Show(bInitiallyVisible);

    pWindow->SetMapMode(MAP_PIXEL);
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

Reference<rendering::XCanvas> SAL_CALL PresenterHelper::createSharedCanvas (
    const Reference<rendering::XSpriteCanvas>& rxUpdateCanvas,
    const Reference<awt::XWindow>& rxUpdateWindow,
    const Reference<rendering::XCanvas>& rxSharedCanvas,
    const Reference<awt::XWindow>& rxSharedWindow,
    const Reference<awt::XWindow>& rxWindow)
    throw (css::uno::RuntimeException, std::exception)
{
    if ( ! rxSharedCanvas.is()
        || ! rxSharedWindow.is()
        || ! rxWindow.is())
    {
        throw RuntimeException("illegal argument", static_cast<XWeak*>(this));
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

Reference<rendering::XCanvas> SAL_CALL PresenterHelper::createCanvas (
    const Reference<awt::XWindow>& rxWindow,
    sal_Int16 nRequestedCanvasFeatures,
    const OUString& rsOptionalCanvasServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    (void)nRequestedCanvasFeatures;

    // No shared window is given or an explicit canvas service name is
    // specified.  Create a new canvas.
    vcl::Window* pWindow = VCLUnoHelper::GetWindow(rxWindow);
    if (pWindow != nullptr)
    {
        Sequence<Any> aArg (5);

        // common: first any is VCL pointer to window (for VCL canvas)
        aArg[0] = makeAny(reinterpret_cast<sal_Int64>(pWindow));
        aArg[1] = Any();
        aArg[2] = makeAny(css::awt::Rectangle());
        aArg[3] = makeAny(false);
        aArg[4] = makeAny(rxWindow);

        Reference<lang::XMultiServiceFactory> xFactory (
            mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
        return Reference<rendering::XCanvas>(
            xFactory->createInstanceWithArguments(
                !rsOptionalCanvasServiceName.isEmpty()
                    ? rsOptionalCanvasServiceName
                    : OUString("com.sun.star.rendering.Canvas.VCL"),
                aArg),
            UNO_QUERY);
    }
    else
        throw RuntimeException();
}

void SAL_CALL PresenterHelper::toTop (
    const Reference<awt::XWindow>& rxWindow)
    throw (css::uno::RuntimeException, std::exception)
{
    vcl::Window* pWindow = VCLUnoHelper::GetWindow(rxWindow);
    if (pWindow != nullptr)
    {
        pWindow->ToTop();
        pWindow->SetZOrder(nullptr, ZOrderFlags::Last);
    }
}

namespace {

struct IdMapEntry {
    char const * sid;
    sal_uInt32 nid;
};

}

Reference<rendering::XBitmap> SAL_CALL PresenterHelper::loadBitmap (
    const OUString& id,
    const Reference<rendering::XCanvas>& rxCanvas)
    throw (RuntimeException, std::exception)
{
    if ( ! rxCanvas.is())
        return nullptr;

    static IdMapEntry const map[] = {
        { "bitmaps/Background.png", BMP_PRESENTERSCREEN_BACKGROUND },
        { "bitmaps/Animation.png",
          BMP_PRESENTERSCREEN_ANIMATION },
        { "bitmaps/Transition.png",
          BMP_PRESENTERSCREEN_TRANSITION },
        { "bitmaps/BorderActiveBottom.png",
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_BOTTOM },
        { "bitmaps/BorderActiveBottomCallout.png",
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_BOTTOM_CALLOUT },
        { "bitmaps/BorderActiveBottomLeft.png",
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_BOTTOM_LEFT },
        { "bitmaps/BorderActiveBottomRight.png",
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_BOTTOM_RIGHT },
        { "bitmaps/BorderActiveLeft.png",
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_LEFT },
        { "bitmaps/BorderActiveRight.png",
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_RIGHT },
        { "bitmaps/BorderActiveTop.png",
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_TOP },
        { "bitmaps/BorderActiveTopLeft.png",
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_TOP_LEFT },
        { "bitmaps/BorderActiveTopRight.png",
          BMP_PRESENTERSCREEN_BORDER_ACTIVE_TOP_RIGHT },
        { "bitmaps/BorderBottom.png", BMP_PRESENTERSCREEN_BORDER_BOTTOM },
        { "bitmaps/BorderBottomLeft.png",
          BMP_PRESENTERSCREEN_BORDER_BOTTOM_LEFT },
        { "bitmaps/BorderBottomRight.png",
          BMP_PRESENTERSCREEN_BORDER_BOTTOM_RIGHT },
        { "bitmaps/BorderCurrentSlideBottom.png",
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_BOTTOM },
        { "bitmaps/BorderCurrentSlideBottomLeft.png",
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_BOTTOM_LEFT },
        { "bitmaps/BorderCurrentSlideBottomRight.png",
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_BOTTOM_RIGHT },
        { "bitmaps/BorderCurrentSlideLeft.png",
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_LEFT },
        { "bitmaps/BorderCurrentSlideRight.png",
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_RIGHT },
        { "bitmaps/BorderCurrentSlideTop.png",
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_TOP },
        { "bitmaps/BorderCurrentSlideTopLeft.png",
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_TOP_LEFT },
        { "bitmaps/BorderCurrentSlideTopRight.png",
          BMP_PRESENTERSCREEN_BORDER_CURRENT_SLIDE_TOP_RIGHT },
        { "bitmaps/BorderLeft.png", BMP_PRESENTERSCREEN_BORDER_LEFT },
        { "bitmaps/BorderRight.png", BMP_PRESENTERSCREEN_BORDER_RIGHT },
        { "bitmaps/BorderToolbarBottom.png",
          BMP_PRESENTERSCREEN_BORDER_TOOLBAR_BOTTOM },
        { "bitmaps/BorderToolbarLeft.png",
          BMP_PRESENTERSCREEN_BORDER_TOOLBAR_LEFT },
        { "bitmaps/BorderToolbarRight.png",
          BMP_PRESENTERSCREEN_BORDER_TOOLBAR_RIGHT },
        { "bitmaps/BorderToolbarTop.png",
          BMP_PRESENTERSCREEN_BORDER_TOOLBAR_TOP },
        { "bitmaps/BorderToolbarTopLeft.png",
          BMP_PRESENTERSCREEN_BORDER_TOOLBAR_TOP_LEFT },
        { "bitmaps/BorderToolbarTopRight.png",
          BMP_PRESENTERSCREEN_BORDER_TOOLBAR_TOP_RIGHT },
        { "bitmaps/BorderTop.png", BMP_PRESENTERSCREEN_BORDER_TOP },
        { "bitmaps/BorderTopLeft.png", BMP_PRESENTERSCREEN_BORDER_TOP_LEFT },
        { "bitmaps/BorderTopRight.png", BMP_PRESENTERSCREEN_BORDER_TOP_RIGHT },
        { "bitmaps/ButtonEffectNextDisabled.png",
          BMP_PRESENTERSCREEN_BUTTON_EFFECT_NEXT_DISABLED },
        { "bitmaps/ButtonEffectNextMouseOver.png",
          BMP_PRESENTERSCREEN_BUTTON_EFFECT_NEXT_MOUSE_OVER },
        { "bitmaps/ButtonEffectNextNormal.png",
          BMP_PRESENTERSCREEN_BUTTON_EFFECT_NEXT_NORMAL },
        { "bitmaps/ButtonEffectNextSelected.png",
          BMP_PRESENTERSCREEN_BUTTON_EFFECT_NEXT_SELECTED },
        { "bitmaps/ButtonFrameCenterMouseOver.png",
          BMP_PRESENTERSCREEN_BUTTON_FRAME_CENTER_MOUSE_OVER },
        { "bitmaps/ButtonFrameCenterNormal.png",
          BMP_PRESENTERSCREEN_BUTTON_FRAME_CENTER_NORMAL },
        { "bitmaps/ButtonFrameLeftMouseOver.png",
          BMP_PRESENTERSCREEN_BUTTON_FRAME_LEFT_MOUSE_OVER },
        { "bitmaps/ButtonFrameLeftNormal.png",
          BMP_PRESENTERSCREEN_BUTTON_FRAME_LEFT_NORMAL },
        { "bitmaps/ButtonFrameRightMouseOver.png",
          BMP_PRESENTERSCREEN_BUTTON_FRAME_RIGHT_MOUSE_OVER },
        { "bitmaps/ButtonFrameRightNormal.png",
          BMP_PRESENTERSCREEN_BUTTON_FRAME_RIGHT_NORMAL },
        { "bitmaps/ButtonHelpDisabled.png",
          BMP_PRESENTERSCREEN_BUTTON_HELP_DISABLED },
        { "bitmaps/ButtonHelpMouseOver.png",
          BMP_PRESENTERSCREEN_BUTTON_HELP_MOUSE_OVER },
        { "bitmaps/ButtonHelpNormal.png",
          BMP_PRESENTERSCREEN_BUTTON_HELP_NORMAL },
        { "bitmaps/ButtonHelpSelected.png",
          BMP_PRESENTERSCREEN_BUTTON_HELP_SELECTED },
        { "bitmaps/ButtonMinusDisabled.png",
          BMP_PRESENTERSCREEN_BUTTON_MINUS_DISABLED },
        { "bitmaps/ButtonMinusMouseOver.png",
          BMP_PRESENTERSCREEN_BUTTON_MINUS_MOUSE_OVER },
        { "bitmaps/ButtonMinusNormal.png",
          BMP_PRESENTERSCREEN_BUTTON_MINUS_NORMAL },
        { "bitmaps/ButtonMinusSelected.png",
          BMP_PRESENTERSCREEN_BUTTON_MINUS_SELECTED },
        { "bitmaps/ButtonNotesDisabled.png",
          BMP_PRESENTERSCREEN_BUTTON_NOTES_DISABLED },
        { "bitmaps/ButtonNotesMouseOver.png",
          BMP_PRESENTERSCREEN_BUTTON_NOTES_MOUSE_OVER },
        { "bitmaps/ButtonNotesNormal.png",
          BMP_PRESENTERSCREEN_BUTTON_NOTES_NORMAL },
        { "bitmaps/ButtonNotesSelected.png",
          BMP_PRESENTERSCREEN_BUTTON_NOTES_SELECTED },
        { "bitmaps/ButtonPlusDisabled.png",
          BMP_PRESENTERSCREEN_BUTTON_PLUS_DISABLED },
        { "bitmaps/ButtonPlusMouseOver.png",
          BMP_PRESENTERSCREEN_BUTTON_PLUS_MOUSE_OVER },
        { "bitmaps/ButtonPlusNormal.png",
          BMP_PRESENTERSCREEN_BUTTON_PLUS_NORMAL },
        { "bitmaps/ButtonPlusSelected.png",
          BMP_PRESENTERSCREEN_BUTTON_PLUS_SELECTED },
        { "bitmaps/ButtonSlideNextDisabled.png",
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_NEXT_DISABLED },
        { "bitmaps/ButtonSlideNextMouseOver.png",
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_NEXT_MOUSE_OVER },
        { "bitmaps/ButtonSlideNextNormal.png",
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_NEXT_NORMAL },
        { "bitmaps/ButtonSlidePreviousDisabled.png",
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_PREVIOUS_DISABLED },
        { "bitmaps/ButtonSlidePreviousMouseOver.png",
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_PREVIOUS_MOUSE_OVER },
        { "bitmaps/ButtonSlidePreviousNormal.png",
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_PREVIOUS_NORMAL },
        { "bitmaps/ButtonSlidePreviousSelected.png",
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_PREVIOUS_SELECTED },
        { "bitmaps/ButtonSlideSorterDisabled.png",
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_SORTER_DISABLED },
        { "bitmaps/ButtonSlideSorterMouseOver.png",
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_SORTER_MOUSE_OVER },
        { "bitmaps/ButtonSlideSorterNormal.png",
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_SORTER_NORMAL },
        { "bitmaps/ButtonSlideSorterSelected.png",
          BMP_PRESENTERSCREEN_BUTTON_SLIDE_SORTER_SELECTED },
        { "bitmaps/ButtonSwitchMonitorMouseOver.png",
          BMP_PRESENTERSCREEN_BUTTON_SWITCH_MONITOR_MOUSE_OVER },
        { "bitmaps/ButtonSwitchMonitorNormal.png",
          BMP_PRESENTERSCREEN_BUTTON_SWITCH_MONITOR_NORMAL },
        { "bitmaps/ButtonRestartTimerMouseOver.png",
          BMP_PRESENTERSCREEN_BUTTON_RESTART_TIMER_MOUSE_OVER },
        { "bitmaps/ButtonRestartTimerNormal.png",
          BMP_PRESENTERSCREEN_BUTTON_RESTART_TIMER_NORMAL },
        { "bitmaps/LabelMouseOverCenter.png",
          BMP_PRESENTERSCREEN_LABEL_MOUSE_OVER_CENTER },
        { "bitmaps/LabelMouseOverLeft.png",
          BMP_PRESENTERSCREEN_LABEL_MOUSE_OVER_LEFT },
        { "bitmaps/LabelMouseOverRight.png",
          BMP_PRESENTERSCREEN_LABEL_MOUSE_OVER_RIGHT },
        { "bitmaps/ScrollbarArrowDownDisabled.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_DOWN_DISABLED },
        { "bitmaps/ScrollbarArrowDownMouseOver.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_DOWN_MOUSE_OVER },
        { "bitmaps/ScrollbarArrowDownNormal.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_DOWN_NORMAL },
        { "bitmaps/ScrollbarArrowDownSelected.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_DOWN_SELECTED },
        { "bitmaps/ScrollbarArrowUpDisabled.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_UP_DISABLED },
        { "bitmaps/ScrollbarArrowUpMouseOver.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_UP_MOUSE_OVER },
        { "bitmaps/ScrollbarArrowUpNormal.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_UP_NORMAL },
        { "bitmaps/ScrollbarArrowUpSelected.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_ARROW_UP_SELECTED },
        { "bitmaps/ScrollbarPagerMiddleMouseOver.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_PAGER_MIDDLE_MOUSE_OVER },
        { "bitmaps/ScrollbarPagerMiddleNormal.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_PAGER_MIDDLE_NORMAL },
        { "bitmaps/ScrollbarThumbBottomMouseOver.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_THUMB_BOTTOM_MOUSE_OVER },
        { "bitmaps/ScrollbarThumbBottomNormal.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_THUMB_BOTTOM_NORMAL },
        { "bitmaps/ScrollbarThumbMiddleMouseOver.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_THUMB_MIDDLE_MOUSE_OVER },
        { "bitmaps/ScrollbarThumbMiddleNormal.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_THUMB_MIDDLE_NORMAL },
        { "bitmaps/ScrollbarThumbTopMouseOver.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_THUMB_TOP_MOUSE_OVER },
        { "bitmaps/ScrollbarThumbTopNormal.png",
          BMP_PRESENTERSCREEN_SCROLLBAR_THUMB_TOP_NORMAL },
        { "bitmaps/ViewBackground.png", BMP_PRESENTERSCREEN_VIEW_BACKGROUND }
    };
    sal_uInt32 nid = 0;
    for (std::size_t i = 0; i != SAL_N_ELEMENTS(map); ++i) {
        if (id.equalsAscii(map[i].sid)) {
            nid = map[i].nid;
            break;
        }
    }
    if (nid == 0) {
        return nullptr;
    }

    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    const cppcanvas::CanvasSharedPtr pCanvas (
        cppcanvas::VCLFactory::createCanvas(
            Reference<css::rendering::XCanvas>(rxCanvas,UNO_QUERY)));

    if (pCanvas.get() != nullptr)
    {
        BitmapEx aBitmapEx = SdResId(nid);
        cppcanvas::BitmapSharedPtr xBitmap(
            cppcanvas::VCLFactory::createBitmap(pCanvas,
                aBitmapEx));
        if (xBitmap.get() == nullptr)
            return nullptr;
        return xBitmap->getUNOBitmap();
    }

    return nullptr;
}

void SAL_CALL PresenterHelper::captureMouse (
    const Reference<awt::XWindow>& rxWindow)
    throw (RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    // Capture the mouse (if not already done.)
    vcl::Window* pWindow = VCLUnoHelper::GetWindow(rxWindow);
    if (pWindow != nullptr && ! pWindow->IsMouseCaptured())
    {
        pWindow->CaptureMouse();
    }
}

void SAL_CALL PresenterHelper::releaseMouse (const Reference<awt::XWindow>& rxWindow)
    throw (RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    // Release the mouse (if not already done.)
    vcl::Window* pWindow = VCLUnoHelper::GetWindow(rxWindow);
    if (pWindow != nullptr && pWindow->IsMouseCaptured())
    {
        pWindow->ReleaseMouse();
    }
}

awt::Rectangle PresenterHelper::getWindowExtentsRelative (
    const Reference<awt::XWindow>& rxChildWindow,
    const Reference<awt::XWindow>& rxParentWindow)
    throw (RuntimeException, std::exception)
{
    vcl::Window* pChildWindow = VCLUnoHelper::GetWindow(rxChildWindow);
    vcl::Window* pParentWindow = VCLUnoHelper::GetWindow(rxParentWindow);
    if (pChildWindow!=nullptr && pParentWindow!=nullptr)
    {
        Rectangle aBox (pChildWindow->GetWindowExtentsRelative(pParentWindow));
        return awt::Rectangle(aBox.Left(),aBox.Top(),aBox.GetWidth(),aBox.GetHeight());
    }
    else
        return awt::Rectangle();
}

} } // end of namespace ::sd::presenter


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_Draw_PresenterHelper_get_implementation(css::uno::XComponentContext* context,
                                                          css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::presenter::PresenterHelper(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
