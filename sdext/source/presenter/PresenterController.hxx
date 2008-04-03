/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterController.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:57:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SDEXT_PRESENTER_CONTROLLER_HXX
#define SDEXT_PRESENTER_CONTROLLER_HXX

#include <cppuhelper/compbase5.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/presentation/XSlideShowListener.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/Color.hpp>
#include <rtl/ref.hxx>
#include <vector>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

class PresenterAnimator;
class PresenterPaneAnimator;
class PresenterPaneContainer;
class PresenterPaneBorderPainter;
class PresenterTheme;
class PresenterWindowManager;

namespace {
    typedef ::cppu::WeakComponentImplHelper5 <
        css::drawing::framework::XConfigurationChangeListener,
        css::frame::XFrameActionListener,
        css::awt::XKeyListener,
        css::awt::XFocusListener,
        css::awt::XMouseListener
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
    PresenterController (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::frame::XController>& rxController,
        const css::uno::Reference<css::presentation::XSlideShowController>& rxSlideShowController,
        const rtl::Reference<PresenterPaneContainer>& rpPaneContainer,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxMainPaneId);
    ~PresenterController (void);

    virtual void SAL_CALL disposing (void);

    void UpdateCurrentSlide (const sal_Int32 nOffset);

    sal_Int32 GetBackgroundColor (void) const;
    sal_Int32 GetViewBackgroundColor (const ::rtl::OUString& rsViewURL) const;
    css::uno::Reference<css::rendering::XBitmap> GetViewBackgroundBitmap (const ::rtl::OUString& rsViewURL) const;
    css::awt::FontDescriptor GetViewFontDescriptor (const ::rtl::OUString& rsViewURL) const;
    css::rendering::FontRequest GetViewFontRequest (const ::rtl::OUString& rsViewURL) const;
    css::util::Color GetViewFontColor (const ::rtl::OUString& rsViewURL) const;
    ::boost::shared_ptr<PresenterTheme> GetTheme (void) const;
    ::rtl::Reference<PresenterWindowManager> GetWindowManager (void) const;
    css::uno::Reference<css::presentation::XSlideShowController>
        GetSlideShowController (void) const;
    css::uno::Reference<css::awt::XWindow> GetParentWindow (void) const;
    rtl::Reference<PresenterPaneContainer> GetPaneContainer (void) const;
    ::rtl::Reference<PresenterPaneBorderPainter> GetPaneBorderPainter (void) const;
    ::boost::shared_ptr<PresenterAnimator> GetAnimator (void) const;
    void HideSlideSorter (void);

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

private:
    css::uno::Reference<css::uno::XComponentContext> mxContext;
    css::uno::Reference<css::frame::XController> mxController;
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;
    css::uno::Reference<css::presentation::XSlideShowController> mxSlideShowController;
    css::uno::Reference<css::drawing::framework::XResourceId> mxMainPaneId;
    rtl::Reference<PresenterPaneContainer> mpPaneContainer;
    css::uno::Reference<css::drawing::XDrawPage> mxCurrentSlide;
    css::uno::Reference<css::drawing::XDrawPage> mxNextSlide;
    ::rtl::Reference<PresenterWindowManager> mpWindowManager;
    ::boost::shared_ptr<PresenterPaneAnimator> mpCurrentPaneAnimation;
    sal_Int32 mnBackgroundColor;
    sal_Int32 mnWindowBackgroundColor;
    ::boost::shared_ptr<PresenterTheme> mpTheme;
    css::uno::Reference<css::awt::XWindow> mxMainWindow;
    ::rtl::Reference<PresenterPaneBorderPainter> mpPaneBorderPainter;
    ::boost::shared_ptr<PresenterAnimator> mpAnimator;

    void InitializePresenterScreen (void);
    void InitializeSlideShowView (const css::uno::Reference<css::uno::XInterface>& rxView);
    void GetSlides (const sal_Int32 nOffset);
    void UpdateViews (void);
    void InitializeMainPane (const css::uno::Reference<css::drawing::framework::XPane>& rxPane);
    void LoadTheme (const css::uno::Reference<css::drawing::framework::XPane>& rxPane);
    void ToggleHelpWindow (const bool bUseAnimation);
    enum ToggleState { Toggle, Show, Hide };
    void ToggleSlideSorter (
        const ToggleState eState,
        const sal_Int32 nAppearMode);

    void ThrowIfDisposed (void) const throw (::com::sun::star::lang::DisposedException);
};


} } // end of namespace ::sdext::presenter

#endif
