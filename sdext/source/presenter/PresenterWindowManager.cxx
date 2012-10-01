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

#undef ENABLE_PANE_RESIZING
//#define ENABLE_PANE_RESIZING

#include "PresenterWindowManager.hxx"
#include "PresenterController.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterHelper.hxx"
#include "PresenterPaintManager.hxx"
#include "PresenterPaneBase.hxx"
#include "PresenterPaneBorderManager.hxx"
#include "PresenterPaneBorderPainter.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterPaneFactory.hxx"
#include "PresenterSprite.hxx"
#include "PresenterToolBar.hxx"
#include "PresenterViewFactory.hxx"
#include "PresenterTheme.hxx"
#include <com/sun/star/awt/InvalidateStyle.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/FillRule.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/Texture.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <math.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

#define A2S(pString) (::rtl::OUString(pString))

namespace sdext { namespace presenter {

//===== PresenterWindowManager ================================================

PresenterWindowManager::PresenterWindowManager (
    const Reference<XComponentContext>& rxContext,
    const ::rtl::Reference<PresenterPaneContainer>& rpPaneContainer,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterWindowManagerInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      mpPresenterController(rpPresenterController),
      mxParentWindow(),
      mxParentCanvas(),
      mxPaneBorderManager(),
      mpPaneBorderPainter(),
      mpPaneContainer(rpPaneContainer),
      mbIsLayoutPending(true),
      mbIsLayouting(false),
      mpTheme(),
      mpBackgroundBitmap(),
      mxScaledBackgroundBitmap(),
      maPaneBackgroundColor(),
      mxClipPolygon(),
      meLayoutMode(LM_Generic),
      mbIsSlideSorterActive(false),
      mbIsHelpViewActive(false),
      maLayoutListeners(),
      mbIsMouseClickPending(false)
{
    UpdateWindowList();
}

PresenterWindowManager::~PresenterWindowManager (void)
{
}

void SAL_CALL PresenterWindowManager::disposing (void)
{
    NotifyDisposing();

    SetParentPane(NULL);

    Reference<lang::XComponent> xComponent (mxPaneBorderManager, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
    mxPaneBorderManager = NULL;

    PresenterPaneContainer::PaneList::const_iterator iPane;
    PresenterPaneContainer::PaneList::const_iterator iEnd (mpPaneContainer->maPanes.end());
    for (iPane=mpPaneContainer->maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        if ((*iPane)->mxBorderWindow.is())
        {
            (*iPane)->mxBorderWindow->removeWindowListener(this);
            (*iPane)->mxBorderWindow->removeFocusListener(this);
#ifndef ENABLE_PANE_RESIZING
            (*iPane)->mxBorderWindow->removeMouseListener(this);
#endif
        }
    }
}

void PresenterWindowManager::SetParentPane (
    const Reference<drawing::framework::XPane>& rxPane)
{
    if (mxParentWindow.is())
    {
        mxParentWindow->removeWindowListener(this);
        mxParentWindow->removePaintListener(this);
        mxParentWindow->removeMouseListener(this);
        mxParentWindow->removeFocusListener(this);
    }
    mxParentWindow = NULL;
    mxParentCanvas = NULL;

    if (rxPane.is())
    {
        mxParentWindow = rxPane->getWindow();
        mxParentCanvas = rxPane->getCanvas();
    }
    else
    {
        mxParentWindow = NULL;
    }

    if (mxParentWindow.is())
    {
        mxParentWindow->addWindowListener(this);
        mxParentWindow->addPaintListener(this);
        mxParentWindow->addMouseListener(this);
        mxParentWindow->addFocusListener(this);

        // We paint our own background, make that of the parent window transparent.
        Reference<awt::XWindowPeer> xPeer (mxParentWindow, UNO_QUERY);
        if (xPeer.is())
            xPeer->setBackground(util::Color(0xff000000));
    }
}

void PresenterWindowManager::SetTheme (const ::boost::shared_ptr<PresenterTheme>& rpTheme)
{
    mpTheme = rpTheme;

    // Get background bitmap or background color from the theme.

    if (mpTheme.get() != NULL)
    {
        mpBackgroundBitmap = mpTheme->GetBitmap(OUString(), A2S("Background"));
    }
}

void PresenterWindowManager::NotifyViewCreation (const Reference<XView>& rxView)
{
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindPaneId(rxView->getResourceId()->getAnchor()));
    OSL_ASSERT(pDescriptor.get() != NULL);
    if (pDescriptor.get() != NULL)
    {
        Layout();

        mpPresenterController->GetPaintManager()->Invalidate(
            pDescriptor->mxContentWindow,
            (sal_Int16)(awt::InvalidateStyle::TRANSPARENT
            | awt::InvalidateStyle::CHILDREN));
    }
}

void PresenterWindowManager::SetPanePosSizeAbsolute (
    const OUString& rsPaneURL,
    const double nX,
    const double nY,
    const double nWidth,
    const double nHeight)
{
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindPaneURL(rsPaneURL));
    if (pDescriptor.get() != NULL)
    {
        awt::Rectangle aParentBox = mxParentWindow->getPosSize();
        if (aParentBox.Width > 0 && aParentBox.Height > 0)
        {
            pDescriptor->mnLeft = nX / aParentBox.Width;
            pDescriptor->mnTop = nY / aParentBox.Height;
            pDescriptor->mnRight = (nX + nWidth) / aParentBox.Width;
            pDescriptor->mnBottom = (nY + nHeight) / aParentBox.Height;
        }
        if (pDescriptor->mxBorderWindow.is())
            pDescriptor->mxBorderWindow->setPosSize(
                ::sal::static_int_cast<sal_Int32>(nX),
                ::sal::static_int_cast<sal_Int32>(nY),
                ::sal::static_int_cast<sal_Int32>(nWidth),
                ::sal::static_int_cast<sal_Int32>(nHeight),
                awt::PosSize::POSSIZE);
    }
}

void PresenterWindowManager::SetPaneBorderPainter (
    const ::rtl::Reference<PresenterPaneBorderPainter>& rPainter)
{
    mpPaneBorderPainter = rPainter;
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterWindowManager::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (rEvent.Source == mxParentWindow)
    {
        Layout();
    }
    else
    {
        Reference<awt::XWindow> xWindow (rEvent.Source,UNO_QUERY);
        if (xWindow.is())
        {
            UpdateWindowSize(xWindow);

            // Make sure the background of a transparent window is painted.
            mpPresenterController->GetPaintManager()->Invalidate(mxParentWindow);
        }
    }
}

void SAL_CALL PresenterWindowManager::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (rEvent.Source != mxParentWindow)
    {
        Reference<awt::XWindow> xWindow (rEvent.Source,UNO_QUERY);
        UpdateWindowSize(xWindow);

        // Make sure the background of a transparent window is painted.
        mpPresenterController->GetPaintManager()->Invalidate(xWindow);
    }
}

void SAL_CALL PresenterWindowManager::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}

void SAL_CALL PresenterWindowManager::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}

//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterWindowManager::windowPaint (const awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    if ( ! mxParentWindow.is())
        return;
    if ( ! mxParentCanvas.is())
        return;

    if (mpTheme.get()!=NULL)
    {
        try
        {
            if (mbIsLayoutPending)
                Layout();
            PaintBackground(rEvent.UpdateRect);
            if ( ! PaintChildren(rEvent))
            {
                Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxParentCanvas, UNO_QUERY);
                //                if (xSpriteCanvas.is())
                //                    xSpriteCanvas->updateScreen(sal_False);
            }
        }
        catch (RuntimeException&)
        {
            OSL_FAIL("paint failed!");
        }
    }
}

//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterWindowManager::mousePressed (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
    mbIsMouseClickPending = true;
}

void SAL_CALL PresenterWindowManager::mouseReleased (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
#ifndef ENABLE_PANE_RESIZING
    if (mbIsMouseClickPending)
    {
        mbIsMouseClickPending = false;
        mpPresenterController->HandleMouseClick(rEvent);
    }
#else
    (void)rEvent;
#endif
}

void SAL_CALL PresenterWindowManager::mouseEntered (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
    mbIsMouseClickPending = false;
}

void SAL_CALL PresenterWindowManager::mouseExited (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
    mbIsMouseClickPending = false;
}

//----- XFocusListener --------------------------------------------------------

void SAL_CALL PresenterWindowManager::focusGained (const css::awt::FocusEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    (void)rEvent;
    OSL_TRACE("PresenterWindowManager::focusGained window %x\n",
        rEvent.Source.get());
}

void SAL_CALL PresenterWindowManager::focusLost (const css::awt::FocusEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    (void)rEvent;
}

//----- XEventListener --------------------------------------------------------

void SAL_CALL PresenterWindowManager::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Source == mxParentWindow)
        mxParentWindow = NULL;
    else
    {
        Reference<awt::XWindow> xWindow (rEvent.Source, UNO_QUERY);
    }
}

//-----------------------------------------------------------------------------

bool PresenterWindowManager::PaintChildren (const awt::PaintEvent& rEvent) const
{
    bool bChildInvalidated (false);

    // Call windowPaint on all children that lie in or touch the
    // update rectangle.
    PresenterPaneContainer::PaneList::const_iterator iPane;
    PresenterPaneContainer::PaneList::const_iterator iEnd (mpPaneContainer->maPanes.end());
    for (iPane=mpPaneContainer->maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        try
        {
            // Make sure that the pane shall and can be painted.
            if ( ! (*iPane)->mbIsActive)
                continue;
            if ((*iPane)->mbIsSprite)
                continue;
            if ( ! (*iPane)->mxPane.is())
                continue;
            if ( ! (*iPane)->mxBorderWindow.is())
                continue;
            Reference<awt::XWindow> xBorderWindow ((*iPane)->mxBorderWindow);
            if ( ! xBorderWindow.is())
                continue;

            // Get the area in which the border of the pane has to be painted.
            const awt::Rectangle aBorderBox (xBorderWindow->getPosSize());
            const awt::Rectangle aBorderUpdateBox(
                PresenterGeometryHelper::Intersection(
                    rEvent.UpdateRect,
                    aBorderBox));
            if (aBorderUpdateBox.Width<=0 || aBorderUpdateBox.Height<=0)
                continue;

            const awt::Rectangle aLocalBorderUpdateBox(
                PresenterGeometryHelper::TranslateRectangle(
                    aBorderUpdateBox,
                    -aBorderBox.X,
                    -aBorderBox.Y));

            // Invalidate the area of the content window.
            mpPresenterController->GetPaintManager()->Invalidate(
                xBorderWindow,
                aLocalBorderUpdateBox,
                sal_Int16(awt::InvalidateStyle::CHILDREN
                    | awt::InvalidateStyle::NOTRANSPARENT));
        }
        catch (RuntimeException&)
        {
            OSL_FAIL("paint children failed!");
        }
    }

    return bChildInvalidated;
}

void PresenterWindowManager::SetLayoutMode (const LayoutMode eMode)
{
    OSL_ASSERT(mpPresenterController.get() != NULL);

    if (meLayoutMode != eMode
        || mbIsSlideSorterActive
        || mbIsHelpViewActive)
    {
        meLayoutMode = eMode;
        mbIsSlideSorterActive = false;
        mbIsHelpViewActive = false;

        mpPresenterController->RequestViews(
            mbIsSlideSorterActive,
            meLayoutMode==LM_Notes,
            mbIsHelpViewActive);
        Layout();
        NotifyLayoutModeChange();
    }
}

void PresenterWindowManager::SetSlideSorterState (bool bIsActive)
{
    if (mbIsSlideSorterActive != bIsActive)
    {
        mbIsSlideSorterActive = bIsActive;
        if (mbIsSlideSorterActive)
            mbIsHelpViewActive = false;
        StoreViewMode(GetViewMode());

        mpPresenterController->RequestViews(
            mbIsSlideSorterActive,
            meLayoutMode==LM_Notes,
            mbIsHelpViewActive);
        Layout();
        NotifyLayoutModeChange();
    }
}

void PresenterWindowManager::SetHelpViewState (bool bIsActive)
{
    if (mbIsHelpViewActive != bIsActive)
    {
        mbIsHelpViewActive = bIsActive;
        if (mbIsHelpViewActive)
            mbIsSlideSorterActive = false;
        StoreViewMode(GetViewMode());

        mpPresenterController->RequestViews(
            mbIsSlideSorterActive,
            meLayoutMode==LM_Notes,
            mbIsHelpViewActive);
        Layout();
        NotifyLayoutModeChange();
    }
}

void PresenterWindowManager::SetViewMode (const ViewMode eMode)
{
    switch (eMode)
    {
        case VM_Standard:
            SetSlideSorterState(false);
            SetHelpViewState(false);
            SetLayoutMode(LM_Standard);
            break;

        case VM_Notes:
            SetSlideSorterState(false);
            SetHelpViewState(false);
            SetLayoutMode(LM_Notes);
            break;

        case VM_SlideOverview:
            SetHelpViewState(false);
            SetSlideSorterState(true);
            break;

        case VM_Help:
            SetHelpViewState(true);
            SetSlideSorterState(false);
            break;
    }

    StoreViewMode(eMode);
}

PresenterWindowManager::ViewMode PresenterWindowManager::GetViewMode (void) const
{
    if (mbIsHelpViewActive)
        return VM_Help;
    else if (mbIsSlideSorterActive)
        return VM_SlideOverview;
    else if (meLayoutMode == LM_Notes)
        return VM_Notes;
    else
        return VM_Standard;
}

void PresenterWindowManager::RestoreViewMode (void)
{
    sal_Int32 nMode (0);
    PresenterConfigurationAccess aConfiguration (
        mxComponentContext,
        OUString("/org.openoffice.Office.extension.PresenterScreen/"),
        PresenterConfigurationAccess::READ_ONLY);
    aConfiguration.GetConfigurationNode(A2S("Presenter/InitialViewMode")) >>= nMode;
    switch (nMode)
    {
        default:
        case 0:
            SetViewMode(VM_Standard);
            break;

        case 1:
            SetViewMode(VM_Notes);
            break;

        case 2:
            SetViewMode(VM_SlideOverview);
            break;
    }
}

void PresenterWindowManager::StoreViewMode (const ViewMode eViewMode)
{
    try
    {
        PresenterConfigurationAccess aConfiguration (
            mxComponentContext,
            OUString("/org.openoffice.Office.extension.PresenterScreen/"),
            PresenterConfigurationAccess::READ_WRITE);
        aConfiguration.GoToChild(A2S("Presenter"));
        Any aValue;
        switch (eViewMode)
        {
            default:
            case VM_Standard:
                aValue = Any(sal_Int32(0));
                break;

            case VM_Notes:
                aValue = Any(sal_Int32(1));
                break;

            case VM_SlideOverview:
                aValue = Any(sal_Int32(2));
                break;
        }

        aConfiguration.SetProperty (A2S("InitialViewMode"), aValue);
        aConfiguration.CommitChanges();
    }
    catch (Exception&)
    {
    }
}

void PresenterWindowManager::AddLayoutListener (
    const Reference<document::XEventListener>& rxListener)
{
    maLayoutListeners.push_back(rxListener);
}

void PresenterWindowManager::RemoveLayoutListener (
    const Reference<document::XEventListener>& rxListener)
{
    LayoutListenerContainer::iterator iListener (maLayoutListeners.begin());
    LayoutListenerContainer::iterator iEnd (maLayoutListeners.end());
    for ( ; iListener!=iEnd; ++iListener)
    {
        if (*iListener == rxListener)
        {
            maLayoutListeners.erase(iListener);
            // Assume that there are no multiple entries.
            break;
        }
    }
}

void PresenterWindowManager::Layout (void)
{
    if (mxParentWindow.is() && ! mbIsLayouting)
    {
        mbIsLayoutPending = false;
        mbIsLayouting = true;
        mxScaledBackgroundBitmap = NULL;
        mxClipPolygon = NULL;

        try
        {
            if (mbIsSlideSorterActive)
                LayoutSlideSorterMode();
            else if (mbIsHelpViewActive)
                LayoutHelpMode();
            else
                switch (meLayoutMode)
                {
                    case LM_Standard:
                    default:
                        LayoutStandardMode();
                        break;

                    case LM_Notes:
                        LayoutNotesMode();
                        break;
                }
        }
        catch (Exception&)
        {
            OSL_ASSERT(false);
            throw;
        }

        mbIsLayouting = false;
    }
}

void PresenterWindowManager::LayoutStandardMode (void)
{
    awt::Rectangle aBox = mxParentWindow->getPosSize();

    const double nGoldenRatio ((1 + sqrt(5.0)) / 2);
    const double nGap (20);
    const double nHorizontalSlideDivide (aBox.Width / nGoldenRatio);
    double nSlidePreviewTop (0);

    // For the current slide view calculate the outer height from the outer
    // width.  This takes into acount the slide aspect ratio and thus has to
    // go over the inner pane size.
    PresenterPaneContainer::SharedPaneDescriptor pPane (
        mpPaneContainer->FindPaneURL(PresenterPaneFactory::msCurrentSlidePreviewPaneURL));
    if (pPane.get() != NULL)
    {
        const awt::Size aCurrentSlideOuterBox(CalculatePaneSize(
            nHorizontalSlideDivide - 1.5*nGap,
            PresenterPaneFactory::msCurrentSlidePreviewPaneURL));
        nSlidePreviewTop = (aBox.Height - aCurrentSlideOuterBox.Height) / 2;
        SetPanePosSizeAbsolute (
            PresenterPaneFactory::msCurrentSlidePreviewPaneURL,
            nGap,
            nSlidePreviewTop,
            aCurrentSlideOuterBox.Width,
            aCurrentSlideOuterBox.Height);
    }

    // For the next slide view calculate the outer height from the outer
    // width.  This takes into acount the slide aspect ratio and thus has to
    // go over the inner pane size.
    pPane = mpPaneContainer->FindPaneURL(PresenterPaneFactory::msNextSlidePreviewPaneURL);
    if (pPane.get() != NULL)
    {
        const awt::Size aNextSlideOuterBox (CalculatePaneSize(
            aBox.Width - nHorizontalSlideDivide - 1.5*nGap,
            PresenterPaneFactory::msNextSlidePreviewPaneURL));
        SetPanePosSizeAbsolute (
            PresenterPaneFactory::msNextSlidePreviewPaneURL,
            aBox.Width - aNextSlideOuterBox.Width - nGap,
            nSlidePreviewTop,
            aNextSlideOuterBox.Width,
            aNextSlideOuterBox.Height);
    }

    LayoutToolBar();
}

void PresenterWindowManager::LayoutNotesMode (void)
{
    awt::Rectangle aBox = mxParentWindow->getPosSize();

    const geometry::RealRectangle2D aToolBarBox (LayoutToolBar());

    const double nGoldenRatio ((1 + sqrt(5.0)) / 2);
    const double nGap (20);
    const double nPrimaryWidth (aBox.Width / nGoldenRatio);
    const double nSecondaryWidth (aBox.Width - nPrimaryWidth);
    const double nTertiaryWidth (nSecondaryWidth / nGoldenRatio);
    double nSlidePreviewTop (0);
    double nNotesViewBottom (aToolBarBox.Y1 - nGap);

    // The notes view has no fixed aspect ratio.
    PresenterPaneContainer::SharedPaneDescriptor pPane (
        mpPaneContainer->FindPaneURL(PresenterPaneFactory::msNotesPaneURL));
    if (pPane.get() != NULL)
    {
        const geometry::RealSize2D aNotesViewOuterSize(
            nPrimaryWidth - 1.5*nGap + 0.5,
            nNotesViewBottom);
        nSlidePreviewTop = (aBox.Height
            - aToolBarBox.Y2 + aToolBarBox.Y1 - aNotesViewOuterSize.Height) / 2;
        SetPanePosSizeAbsolute (
            PresenterPaneFactory::msNotesPaneURL,
            aBox.Width - aNotesViewOuterSize.Width - nGap,
            nSlidePreviewTop,
            aNotesViewOuterSize.Width,
            aNotesViewOuterSize.Height);
        nNotesViewBottom = nSlidePreviewTop + aNotesViewOuterSize.Height;
    }

    // For the current slide view calculate the outer height from the outer
    // width.  This takes into acount the slide aspect ratio and thus has to
    // go over the inner pane size.
    pPane = mpPaneContainer->FindPaneURL(PresenterPaneFactory::msCurrentSlidePreviewPaneURL);
    if (pPane.get() != NULL)
    {
        const awt::Size aCurrentSlideOuterBox(CalculatePaneSize(
            nSecondaryWidth - 1.5*nGap,
            PresenterPaneFactory::msCurrentSlidePreviewPaneURL));
        SetPanePosSizeAbsolute (
            PresenterPaneFactory::msCurrentSlidePreviewPaneURL,
            nGap,
            nSlidePreviewTop,
            aCurrentSlideOuterBox.Width,
            aCurrentSlideOuterBox.Height);
    }

    // For the next slide view calculate the outer height from the outer
    // width.  This takes into acount the slide aspect ratio and thus has to
    // go over the inner pane size.
    pPane = mpPaneContainer->FindPaneURL(PresenterPaneFactory::msNextSlidePreviewPaneURL);
    if (pPane.get() != NULL)
    {
        const awt::Size aNextSlideOuterBox (CalculatePaneSize(
            nTertiaryWidth,
            PresenterPaneFactory::msNextSlidePreviewPaneURL));
        SetPanePosSizeAbsolute (
            PresenterPaneFactory::msNextSlidePreviewPaneURL,
            nGap,
            nNotesViewBottom - aNextSlideOuterBox.Height,
            aNextSlideOuterBox.Width,
            aNextSlideOuterBox.Height);
    }
}

void PresenterWindowManager::LayoutSlideSorterMode (void)
{
    const geometry::RealRectangle2D aToolBarBox (LayoutToolBar());

    awt::Rectangle aWindowBox = mxParentWindow->getPosSize();
    const double nGap (20);
    SetPanePosSizeAbsolute(
        mpPaneContainer->GetPaneURLForViewURL(PresenterViewFactory::msSlideSorterURL),
        nGap,
        nGap,
        aWindowBox.Width - 2*nGap,
        aToolBarBox.Y1 - 2*nGap);
}

void PresenterWindowManager::LayoutHelpMode (void)
{
    const geometry::RealRectangle2D aToolBarBox (LayoutToolBar());

    awt::Rectangle aWindowBox = mxParentWindow->getPosSize();
    const double nGap (20);
    const double nGoldenRatio ((1 + sqrt(5.0)) / 2);
    const double nWidth = ::std::min(aWindowBox.Width - 2*nGap, aWindowBox.Width/nGoldenRatio);
    SetPanePosSizeAbsolute(
        mpPaneContainer->GetPaneURLForViewURL(PresenterViewFactory::msHelpViewURL),
        (aWindowBox.Width - nWidth)/2,
        nGap,
        nWidth,
        aToolBarBox.Y1 - 2*nGap);
}

geometry::RealRectangle2D PresenterWindowManager::LayoutToolBar (void)
{
    double nToolBarWidth (400);
    double nToolBarHeight (80);

    // Get access to the tool bar.
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor(
        mpPaneContainer->FindPaneURL(PresenterPaneFactory::msToolBarPaneURL));
    if (pDescriptor.get() != NULL)
    {
        PresenterToolBarView* pToolBarView
            = dynamic_cast<PresenterToolBarView*>(pDescriptor->mxView.get());
        if (pToolBarView != NULL && pToolBarView->GetPresenterToolBar().is())
        {
            geometry::RealSize2D aSize (pToolBarView->GetPresenterToolBar()->GetMinimalSize());

            if (mpPaneBorderPainter.is())
            {
                const awt::Rectangle aBox (mpPaneBorderPainter->addBorder (
                    PresenterPaneFactory::msToolBarPaneURL,
                    awt::Rectangle(
                        0,
                        0,
                        PresenterGeometryHelper::Round(aSize.Width),
                        PresenterGeometryHelper::Round(aSize.Height)),
                    css::drawing::framework::BorderType_TOTAL_BORDER));

                nToolBarWidth = aBox.Width;
                nToolBarHeight = aBox.Height;
            }
            else
            {
                nToolBarWidth = aSize.Width + 20;
                nToolBarHeight = aSize.Height + 10;
            }
        }
    }

    const awt::Rectangle aBox = mxParentWindow->getPosSize();
    const double nToolBarX ((aBox.Width - nToolBarWidth) / 2);
    const double nToolBarY (aBox.Height - nToolBarHeight);
    SetPanePosSizeAbsolute(
        PresenterPaneFactory::msToolBarPaneURL,
        nToolBarX,
        nToolBarY,
        nToolBarWidth,
        nToolBarHeight);

    return geometry::RealRectangle2D(
        nToolBarX,
        nToolBarY,
        nToolBarX + nToolBarWidth - 1,
        nToolBarY + nToolBarHeight - 1);
}

awt::Size PresenterWindowManager::CalculatePaneSize (
    const double nOuterWidth,
    const OUString& rsPaneURL)
{
    // Calculate the inner width by removing the pane border.
    awt::Rectangle aInnerBox (mpPaneBorderPainter->RemoveBorder (
        rsPaneURL,
        awt::Rectangle(0,0,
            sal_Int32(nOuterWidth+0.5),sal_Int32(nOuterWidth)),
        drawing::framework::BorderType_TOTAL_BORDER));

    // Calculate the inner height with the help of the slide aspect ratio.
    const double nCurrentSlideInnerHeight (
        aInnerBox.Width / mpPresenterController->GetSlideAspectRatio());

    // Add the pane border to get the outer box.
    awt::Rectangle aOuterBox (mpPaneBorderPainter->AddBorder (
        rsPaneURL,
        awt::Rectangle(0,0,
            aInnerBox.Width,sal_Int32(nCurrentSlideInnerHeight+0.5)),
        drawing::framework::BorderType_TOTAL_BORDER));

    return awt::Size(aOuterBox.Width, aOuterBox.Height);
}

void PresenterWindowManager::NotifyLayoutModeChange (void)
{
    document::EventObject aEvent;
    aEvent.Source = Reference<XInterface>(static_cast<XWeak*>(this));

    LayoutListenerContainer aContainerCopy (maLayoutListeners);
    LayoutListenerContainer::iterator iListener (aContainerCopy.begin());
    LayoutListenerContainer::iterator iEnd (aContainerCopy.end());
    for ( ; iListener!=iEnd; ++iListener)
    {
        if (iListener->is())
        {
            try
            {
                (*iListener)->notifyEvent(aEvent);
            }
            catch (lang::DisposedException&)
            {
                RemoveLayoutListener(*iListener);
            }
            catch (RuntimeException&)
            {
            }
        }
    }
}

void PresenterWindowManager::NotifyDisposing (void)
{
    lang::EventObject aEvent;
    aEvent.Source = static_cast<XWeak*>(this);

    LayoutListenerContainer aContainer;
    aContainer.swap(maLayoutListeners);
    LayoutListenerContainer::iterator iListener (aContainer.begin());
    LayoutListenerContainer::iterator iEnd (aContainer.end());
    for ( ; iListener!=iEnd; ++iListener)
    {
        if (iListener->is())
        {
            try
            {
                (*iListener)->disposing(aEvent);
            }
            catch (lang::DisposedException&)
            {
            }
            catch (RuntimeException&)
            {
            }
        }
    }
}

void PresenterWindowManager::UpdateWindowSize (const Reference<awt::XWindow>& rxBorderWindow)
{
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindBorderWindow(rxBorderWindow));
    if (pDescriptor.get() != NULL)
    {
        mxClipPolygon = NULL;

        awt::Rectangle aParentBox = mxParentWindow->getPosSize();
        awt::Rectangle aBorderBox (pDescriptor->mxBorderWindow->getPosSize());

        if ( ! mbIsLayouting)
        {
            const double nWidth (aParentBox.Width);
            const double nHeight (aParentBox.Height);
            pDescriptor->mnLeft = double(aBorderBox.X) / nWidth;
            pDescriptor->mnTop = double(aBorderBox.Y) / nHeight;
            pDescriptor->mnRight = double(aBorderBox.X + aBorderBox.Width) / nWidth;
            pDescriptor->mnBottom = double(aBorderBox.Y + aBorderBox.Height) / nHeight;
        }
        else
        {
            // This update of the window size was initiated by
            // Layout(). Therefore the window size does not have to be
            // updated.
        }

        // ToTop is called last because it may invalidate the iterator.
        if ( ! mbIsLayouting)
            mpPaneContainer->ToTop(pDescriptor);
    }
}

void PresenterWindowManager::PaintBackground (const awt::Rectangle& rUpdateBox)
{
    (void)rUpdateBox;
    if ( ! mxParentWindow.is())
        return;

    Reference<rendering::XGraphicDevice> xDevice (mxParentCanvas->getDevice());
    if ( ! xDevice.is())
        return;

    // Create a polygon for the background and for clipping.
    Reference<rendering::XPolyPolygon2D> xBackgroundPolygon (
        PresenterGeometryHelper::CreatePolygon(mxParentWindow->getPosSize(), xDevice));
    if ( ! mxClipPolygon.is())
        mxClipPolygon = CreateClipPolyPolygon();

    // Create View- and RenderState structs.
    const rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        PresenterGeometryHelper::CreatePolygon(rUpdateBox, xDevice));
    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        mxClipPolygon,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    // Paint the background.
    if (mpBackgroundBitmap.get() != NULL)
    {
        ProvideBackgroundBitmap();

        if (mxScaledBackgroundBitmap.is())
        {
            Sequence<rendering::Texture> aTextures (1);
            const geometry::IntegerSize2D aBitmapSize(mxScaledBackgroundBitmap->getSize());
            aTextures[0] = rendering::Texture (
                geometry::AffineMatrix2D(
                    aBitmapSize.Width,0,0,
                    0,aBitmapSize.Height,0),
                1,
                0,
                mxScaledBackgroundBitmap,
                NULL,
                NULL,
                rendering::StrokeAttributes(),
                rendering::TexturingMode::REPEAT,
                rendering::TexturingMode::REPEAT);

            mxParentCanvas->fillTexturedPolyPolygon(
                xBackgroundPolygon,
                aViewState,
                aRenderState,
                aTextures);
        }
        else
        {
            const util::Color aBackgroundColor (mpBackgroundBitmap->maReplacementColor);
            aRenderState.DeviceColor[0] = ((aBackgroundColor >> 16) & 0x0ff) / 255.0;
            aRenderState.DeviceColor[1] = ((aBackgroundColor >> 8) & 0x0ff) / 255.0;
            aRenderState.DeviceColor[2] = ((aBackgroundColor >> 0) & 0x0ff) / 255.0;
            aRenderState.DeviceColor[3] = ((aBackgroundColor >> 24) & 0x0ff) / 255.0;
            mxParentCanvas->fillPolyPolygon(
                xBackgroundPolygon,
                aViewState,
                aRenderState);
        }
    }
}

void PresenterWindowManager::ProvideBackgroundBitmap (void)
{
    if ( ! mxScaledBackgroundBitmap.is())
    {
        Reference<rendering::XBitmap> xBitmap (mpBackgroundBitmap->GetNormalBitmap());
        if (xBitmap.is())
        {
            const bool bStretchVertical (mpBackgroundBitmap->meVerticalTexturingMode
                == PresenterBitmapDescriptor::Stretch);
            const bool bStretchHorizontal (mpBackgroundBitmap->meHorizontalTexturingMode
                == PresenterBitmapDescriptor::Stretch);
            if (bStretchHorizontal || bStretchVertical)
            {
                geometry::RealSize2D aSize;
                if (bStretchVertical)
                    aSize.Height = mxParentWindow->getPosSize().Height;
                else
                    aSize.Height = xBitmap->getSize().Height;
                if (bStretchHorizontal)
                    aSize.Width = mxParentWindow->getPosSize().Width;
                else
                    aSize.Width = xBitmap->getSize().Width;
                mxScaledBackgroundBitmap = xBitmap->getScaledBitmap(aSize, sal_False);
            }
            else
            {
                mxScaledBackgroundBitmap
                    = Reference<rendering::XBitmap>(xBitmap, UNO_QUERY);
            }
        }
    }
}

Reference<rendering::XPolyPolygon2D> PresenterWindowManager::CreateClipPolyPolygon (void) const
{
    // Create a clip polygon that includes the whole update area but has the
    // content windows as holes.
    const sal_Int32 nPaneCount (mpPaneContainer->maPanes.size());
    ::std::vector<awt::Rectangle> aRectangles;
    aRectangles.reserve(1+nPaneCount);
    aRectangles.push_back(mxParentWindow->getPosSize());
    PresenterPaneContainer::PaneList::const_iterator iPane;
    PresenterPaneContainer::PaneList::const_iterator iEnd (mpPaneContainer->maPanes.end());
    for (iPane=mpPaneContainer->maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        PresenterPaneContainer::SharedPaneDescriptor pDescriptor (*iPane);
        if ( ! pDescriptor->mbIsActive)
            continue;
        if ( ! pDescriptor->mbIsOpaque)
            continue;
        if ( ! pDescriptor->mxBorderWindow.is() || ! pDescriptor->mxContentWindow.is())
            continue;
        Reference<awt::XWindow2> xWindow (pDescriptor->mxBorderWindow, UNO_QUERY);
        if (xWindow.is() && ! xWindow->isVisible())
            continue;

        const awt::Rectangle aOuterBorderBox (pDescriptor->mxBorderWindow->getPosSize());
        awt::Rectangle aInnerBorderBox (pDescriptor->mxContentWindow->getPosSize());
        aInnerBorderBox.X += aOuterBorderBox.X;
        aInnerBorderBox.Y += aOuterBorderBox.Y;
        aRectangles.push_back(aInnerBorderBox);
    }
    Reference<rendering::XPolyPolygon2D> xPolyPolygon (
        PresenterGeometryHelper::CreatePolygon(
            aRectangles,
            mxParentCanvas->getDevice()));
    if (xPolyPolygon.is())
        xPolyPolygon->setFillRule(rendering::FillRule_EVEN_ODD);
    return xPolyPolygon;
}

void PresenterWindowManager::UpdateWindowList (void)
{
#ifdef ENABLE_PANE_RESIZING
    try
    {
        OSL_ASSERT(mxComponentContext.is());

        Reference<lang::XComponent> xComponent (mxPaneBorderManager, UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();

        Reference<lang::XMultiComponentFactory> xFactory (mxComponentContext->getServiceManager());
        if (xFactory.is())
        {
            Sequence<Any> aArguments (1 + mpPaneContainer->maPanes.size()*2);
            sal_Int32 nIndex (0);
            aArguments[nIndex++] = Any(mxParentWindow);
            for (sal_uInt32 nPaneIndex=0; nPaneIndex<mpPaneContainer->maPanes.size(); ++nPaneIndex)
            {
                if ( ! mpPaneContainer->maPanes[nPaneIndex]->mbIsActive)
                    continue;

                const Reference<awt::XWindow> xBorderWindow (
                    mpPaneContainer->maPanes[nPaneIndex]->mxBorderWindow);
                const Reference<awt::XWindow> xContentWindow (
                    mpPaneContainer->maPanes[nPaneIndex]->mxContentWindow);
                const Reference<awt::XWindow2> xBorderWindow2(xBorderWindow, UNO_QUERY);
                if (xBorderWindow.is()
                    && xContentWindow.is()
                    && ( ! xBorderWindow2.is() || xBorderWindow2->isVisible()))
                {
                    aArguments[nIndex++] = Any(xBorderWindow);
                    aArguments[nIndex++] = Any(xContentWindow);
                }
            }

            aArguments.realloc(nIndex);
            rtl::Reference<PresenterPaneBorderManager> pManager (
                new PresenterPaneBorderManager (
                    mxComponentContext,
                    mpPresenterController));
            pManager->initialize(aArguments);
            mxPaneBorderManager = Reference<XInterface>(static_cast<XWeak*>(pManager.get()));
        }
    }
    catch (RuntimeException&)
    {
    }
#endif
}

void PresenterWindowManager::Invalidate (void)
{
    mpPresenterController->GetPaintManager()->Invalidate(mxParentWindow);
}
void PresenterWindowManager::Update (void)
{
    mxClipPolygon = NULL;
    mbIsLayoutPending = true;

    UpdateWindowList();
    Invalidate();
}

void PresenterWindowManager::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString( "PresenterWindowManager has already been disposed"),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

} } // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
