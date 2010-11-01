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

#ifndef SDEXT_PRESENTER_CONTROLLER_HXX
#define SDEXT_PRESENTER_CONTROLLER_HXX

#include "PresenterAccessibility.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterTheme.hxx"
#include "PresenterSprite.hxx"
#include <cppuhelper/compbase6.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/presentation/XSlideShowListener.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <rtl/ref.hxx>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

class PresenterAnimator;
class PresenterCanvasHelper;
class PresenterPaintManager;
class PresenterPaneAnimator;
class PresenterPaneContainer;
class PresenterPaneBorderPainter;
class PresenterTheme;
class PresenterWindowManager;

namespace {
    typedef ::cppu::WeakComponentImplHelper6 <
        css::drawing::framework::XConfigurationChangeListener,
        css::frame::XFrameActionListener,
        css::awt::XKeyListener,
        css::awt::XFocusListener,
        css::awt::XMouseListener,
        css::awt::XMouseMotionListener
    > PresenterControllerInterfaceBase;
}


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
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::frame::XController>& rxController,
        const css::uno::Reference<css::presentation::XSlideShowController>& rxSlideShowController,
        const rtl::Reference<PresenterPaneContainer>& rpPaneContainer,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxMainPaneId);
    ~PresenterController (void);

    virtual void SAL_CALL disposing (void);

    void UpdateCurrentSlide (const sal_Int32 nOffset);

    SharedBitmapDescriptor
        GetViewBackground (const ::rtl::OUString& rsViewURL) const;
    PresenterTheme::SharedFontDescriptor
        GetViewFont (const ::rtl::OUString& rsViewURL) const;
    ::boost::shared_ptr<PresenterTheme> GetTheme (void) const;
    ::rtl::Reference<PresenterWindowManager> GetWindowManager (void) const;
    css::uno::Reference<css::presentation::XSlideShowController>
        GetSlideShowController (void) const;
    rtl::Reference<PresenterPaneContainer> GetPaneContainer (void) const;
    ::rtl::Reference<PresenterPaneBorderPainter> GetPaneBorderPainter (void) const;
    ::boost::shared_ptr<PresenterAnimator> GetAnimator (void) const;
    ::boost::shared_ptr<PresenterCanvasHelper> GetCanvasHelper (void) const;
    css::uno::Reference<css::drawing::XPresenterHelper> GetPresenterHelper (void) const;
    ::boost::shared_ptr<PresenterPaintManager> GetPaintManager (void) const;
    void HideSlideSorter (void);
    double GetSlideAspectRatio (void) const;
    void ShowView (const ::rtl::OUString& rsViewURL);
    void HideView (const ::rtl::OUString& rsViewURL);
    bool IsViewVisible (const ::rtl::OUString& rsViewURL) const;
    void DispatchUnoCommand (const ::rtl::OUString& rsCommand) const;
    css::uno::Reference<css::frame::XDispatch> GetDispatch (
        const css::util::URL& rURL) const;
    css::util::URL CreateURLFromString (const ::rtl::OUString& rsURL) const;
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        GetConfigurationController (void) const;
    css::uno::Reference<css::drawing::XDrawPage> GetCurrentSlide (void) const;
    ::rtl::Reference<PresenterAccessible> GetAccessible (void) const;
    void SetAccessibilityActiveState (const bool bIsActive);
    bool IsAccessibilityActive (void) const;

    void HandleMouseClick (const css::awt::MouseEvent& rEvent);
    void UpdatePaneTitles (void);

    /** Request activation or deactivation of (some of) the views according
        to the given parameters.
    */
    void RequestViews (
        const bool bIsSlideSorterActive,
        const bool bIsNotesViewActive,
        const bool bIsHelpViewActive);

    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const com::sun::star::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (com::sun::star::uno::RuntimeException);


    // XEventListener

    virtual void SAL_CALL disposing (
        const com::sun::star::lang::EventObject& rEvent)
        throw (com::sun::star::uno::RuntimeException);


    // XFrameActionListener

    virtual void SAL_CALL frameAction (
        const css::frame::FrameActionEvent& rEvent)
        throw (com::sun::star::uno::RuntimeException);


    // XKeyListener

    virtual void SAL_CALL keyPressed (const css::awt::KeyEvent& rEvent)
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL keyReleased (const css::awt::KeyEvent& rEvent)
        throw (css::uno::RuntimeException);


    // XFocusListener

    virtual void SAL_CALL focusGained (const css::awt::FocusEvent& rEvent)
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL focusLost (const css::awt::FocusEvent& rEvent)
        throw (css::uno::RuntimeException);


    // XMouseListener

    virtual void SAL_CALL mousePressed (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseReleased (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseEntered (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseExited (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);


    // XMouseMotionListener

    virtual void SAL_CALL mouseMoved (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseDragged (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

private:
    typedef ::std::map<css::uno::Reference<css::frame::XFrame>,rtl::Reference<PresenterController> > InstanceContainer;
    static InstanceContainer maInstances;

    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::Reference<css::rendering::XSpriteCanvas> mxCanvas;
    css::uno::Reference<css::frame::XController> mxController;
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;
    css::uno::Reference<css::presentation::XSlideShowController> mxSlideShowController;
    css::uno::Reference<css::drawing::framework::XResourceId> mxMainPaneId;
    rtl::Reference<PresenterPaneContainer> mpPaneContainer;
    sal_Int32 mnCurrentSlideIndex;
    css::uno::Reference<css::drawing::XDrawPage> mxCurrentSlide;
    css::uno::Reference<css::drawing::XDrawPage> mxNextSlide;
    ::rtl::Reference<PresenterWindowManager> mpWindowManager;
    ::boost::shared_ptr<PresenterPaneAnimator> mpCurrentPaneAnimation;
    sal_Int32 mnWindowBackgroundColor;
    ::boost::shared_ptr<PresenterTheme> mpTheme;
    css::uno::Reference<css::awt::XWindow> mxMainWindow;
    ::rtl::Reference<PresenterPaneBorderPainter> mpPaneBorderPainter;
    ::boost::shared_ptr<PresenterAnimator> mpAnimator;
    ::boost::shared_ptr<PresenterCanvasHelper> mpCanvasHelper;
    css::uno::Reference<css::drawing::XPresenterHelper> mxPresenterHelper;
    ::boost::shared_ptr<PresenterPaintManager> mpPaintManager;
    sal_Int32 mnPendingSlideNumber;
    css::uno::Reference<css::util::XURLTransformer> mxUrlTransformer;
    ::rtl::Reference<PresenterAccessible> mpAccessibleObject;
    bool mbIsAccessibilityActive;

    void InitializePresenterScreen (void);
    void InitializeSlideShowView (const css::uno::Reference<css::uno::XInterface>& rxView);
    void GetSlides (const sal_Int32 nOffset);
    void UpdateViews (void);
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

    void ThrowIfDisposed (void) const throw (::com::sun::star::lang::DisposedException);
};


} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
