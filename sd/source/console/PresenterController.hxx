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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERCONTROLLER_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERCONTROLLER_HXX

#include "PresenterAccessibility.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterTheme.hxx"
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <rtl/ref.hxx>
#include <unotools/weakref.hxx>
#include <map>
#include <memory>

namespace sd { class DrawController; }

namespace sdext::presenter {

class PresenterCanvasHelper;
class PresenterPaintManager;
class PresenterPaneAnimator;
class PresenterPaneContainer;
class PresenterPaneBorderPainter;
class PresenterScreen;
class PresenterTheme;
class PresenterWindowManager;

typedef ::cppu::WeakComponentImplHelper <
    css::drawing::framework::XConfigurationChangeListener,
    css::frame::XFrameActionListener,
    css::awt::XKeyListener,
    css::awt::XMouseListener
> PresenterControllerInterfaceBase;

/// Represents an element in the toolbar that shows the time elapsed since the presentation started.
class IPresentationTime
{
public:
    virtual void restart() = 0;
    virtual bool isPaused() = 0;
    virtual void setPauseStatus(const bool pauseStatus) = 0;
    virtual ~IPresentationTime();
};

/** The controller of the presenter screen is responsible for telling the
    individual views which slides to show.  Additionally it provides access
    to frequently used values of the current theme.
*/
class PresenterController
    : protected ::cppu::BaseMutex,
      public PresenterControllerInterfaceBase
{
public:
    static ::rtl::Reference<PresenterController> Instance (
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    PresenterController (
        unotools::WeakReference<PresenterScreen> xScreen,
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const rtl::Reference<::sd::DrawController>& rxController,
        const css::uno::Reference<css::presentation::XSlideShowController>& rxSlideShowController,
        rtl::Reference<PresenterPaneContainer> xPaneContainer,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxMainPaneId);
    virtual ~PresenterController() override;

    virtual void SAL_CALL disposing() override;

    void UpdateCurrentSlide (const sal_Int32 nOffset);

    SharedBitmapDescriptor
        GetViewBackground (const OUString& rsViewURL) const;
    PresenterTheme::SharedFontDescriptor
        GetViewFont (const OUString& rsViewURL) const;
    const std::shared_ptr<PresenterTheme>& GetTheme() const;
    const ::rtl::Reference<PresenterWindowManager>& GetWindowManager() const;
    const css::uno::Reference<css::presentation::XSlideShowController>&
        GetSlideShowController() const;
    const rtl::Reference<PresenterPaneContainer>& GetPaneContainer() const;
    const ::rtl::Reference<PresenterPaneBorderPainter>& GetPaneBorderPainter() const;
    const std::shared_ptr<PresenterCanvasHelper>& GetCanvasHelper() const;
    const css::uno::Reference<css::drawing::XPresenterHelper>& GetPresenterHelper() const;
    const std::shared_ptr<PresenterPaintManager>& GetPaintManager() const;
    double GetSlideAspectRatio() const;
    void ShowView (const OUString& rsViewURL);
    void HideView (const OUString& rsViewURL);
    void SwitchMonitors();
    void ExitPresenter();
    void DispatchUnoCommand (const OUString& rsCommand) const;
    css::uno::Reference<css::frame::XDispatch> GetDispatch (
        const css::util::URL& rURL) const;
    css::util::URL CreateURLFromString (const OUString& rsURL) const;
    const css::uno::Reference<css::drawing::framework::XConfigurationController>&
        GetConfigurationController() const;
    const css::uno::Reference<css::drawing::XDrawPage>& GetCurrentSlide() const;
    static bool HasTransition (css::uno::Reference<css::drawing::XDrawPage> const & rxPage);
    static bool HasCustomAnimation (css::uno::Reference<css::drawing::XDrawPage> const & rxPage);
    void SetAccessibilityActiveState (const bool bIsActive);
    bool IsAccessibilityActive() const { return mbIsAccessibilityActive;}

    void HandleMouseClick (const css::awt::MouseEvent& rEvent);
    void UpdatePaneTitles();

    // check if the 'NextSlide' needs an update when the given
    // XShape is changed and trigger that update
    void CheckNextSlideUpdate(const css::uno::Reference<css::drawing::XShape>& rxShape);

    /** Request activation or deactivation of (some of) the views according
        to the given parameters.
    */
    void RequestViews (
        const bool bIsSlideSorterActive,
        const bool bIsNotesViewActive,
        const bool bIsHelpViewActive);

    void SetPresentationTime(IPresentationTime* pPresentationTime);
    IPresentationTime* GetPresentationTime();

    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent) override;

    // XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent) override;

    // XFrameActionListener

    virtual void SAL_CALL frameAction (
        const css::frame::FrameActionEvent& rEvent) override;

    // XKeyListener

    virtual void SAL_CALL keyPressed (const css::awt::KeyEvent& rEvent) override;
    virtual void SAL_CALL keyReleased (const css::awt::KeyEvent& rEvent) override;

    // XMouseListener

    virtual void SAL_CALL mousePressed (const css::awt::MouseEvent& rEvent) override;

    virtual void SAL_CALL mouseReleased (const css::awt::MouseEvent& rEvent) override;

    virtual void SAL_CALL mouseEntered (const css::awt::MouseEvent& rEvent) override;

    virtual void SAL_CALL mouseExited (const css::awt::MouseEvent& rEvent) override;

private:
    typedef ::std::map<css::uno::Reference<css::frame::XFrame>,rtl::Reference<PresenterController> > InstanceContainer;
    static InstanceContainer maInstances;

    unotools::WeakReference<PresenterScreen> mxScreen;
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::Reference<css::rendering::XSpriteCanvas> mxCanvas;
    rtl::Reference<::sd::DrawController> mxController;
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;
    css::uno::Reference<css::presentation::XSlideShowController> mxSlideShowController;
    css::uno::Reference<css::drawing::framework::XResourceId> mxMainPaneId;
    rtl::Reference<PresenterPaneContainer> mpPaneContainer;
    sal_Int32 mnCurrentSlideIndex;
    css::uno::Reference<css::drawing::XDrawPage> mxCurrentSlide;
    css::uno::Reference<css::drawing::XDrawPage> mxNextSlide;
    ::rtl::Reference<PresenterWindowManager> mpWindowManager;
    std::shared_ptr<PresenterTheme> mpTheme;
    css::uno::Reference<css::awt::XWindow> mxMainWindow;
    ::rtl::Reference<PresenterPaneBorderPainter> mpPaneBorderPainter;
    std::shared_ptr<PresenterCanvasHelper> mpCanvasHelper;
    css::uno::Reference<css::drawing::XPresenterHelper> mxPresenterHelper;
    std::shared_ptr<PresenterPaintManager> mpPaintManager;
    sal_Int32 mnPendingSlideNumber;
    css::uno::Reference<css::util::XURLTransformer> mxUrlTransformer;
    ::rtl::Reference<PresenterAccessible> mpAccessibleObject;
    bool mbIsAccessibilityActive;
    IPresentationTime* mpPresentationTime;

    void GetSlides (const sal_Int32 nOffset);
    void UpdateViews();
    void InitializeMainPane (const css::uno::Reference<css::drawing::framework::XPane>& rxPane);
    void LoadTheme (const css::uno::Reference<css::drawing::framework::XPane>& rxPane);
    void UpdatePendingSlideNumber (const sal_Int32 nPendingSlideNumber);

    /** This method is called when the user pressed one of the numerical
        keys.  Depending on the modifier, numeric keys switch to another
        slide (no modifier), or change to another view (Ctrl modifier).
        @param nKey
            Numeric value that is printed on the pressed key.  For example
            pressing the key '4' will result in the value 4, not the ASCII
            code (0x34?).
        @param nModifiers
            The modifier bit field as provided by the key up event.
    */
    void HandleNumericKeyPress (const sal_Int32 nKey, const sal_Int32 nModifiers);
};

} // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
