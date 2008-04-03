/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterController.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:57:27 $
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

#include "PresenterController.hxx"

#include "PresenterAnimator.hxx"
#include "PresenterCurrentSlideObserver.hxx"
#include "PresenterFrameworkObserver.hxx"
#include "PresenterHelper.hxx"
#include "PresenterPaneAnimator.hxx"
#include "PresenterPaneBase.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterPaneBorderPainter.hxx"
#include "PresenterPaneFactory.hxx"
#include "PresenterTheme.hxx"
#include "PresenterViewFactory.hxx"
#include "PresenterWindowManager.hxx"

#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/presentation/XPresentation.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace {
    const sal_Int32 ResourceActivationEventType = 0;
    const sal_Int32 ResourceDeactivationEventType = 1;
}


namespace sdext { namespace presenter {


PresenterController::PresenterController (
    const Reference<XComponentContext>& rxContext,
    const Reference<frame::XController>& rxController,
    const Reference<presentation::XSlideShowController>& rxSlideShowController,
    const rtl::Reference<PresenterPaneContainer>& rpPaneContainer,
    const Reference<XResourceId>& rxMainPaneId)
    : PresenterControllerInterfaceBase(m_aMutex),
      mxContext(rxContext),
      mxController(rxController),
      mxConfigurationController(),
      mxSlideShowController(rxSlideShowController),
      mxMainPaneId(rxMainPaneId),
      mpPaneContainer(rpPaneContainer),
      mxCurrentSlide(),
      mxNextSlide(),
      mpWindowManager(new PresenterWindowManager(rxContext,mpPaneContainer,this)),
      mpCurrentPaneAnimation(),
      mnBackgroundColor(0x00ffffff),
      mnWindowBackgroundColor(0x00ffffff),
      mpTheme(),
      mxMainWindow(),
      mpPaneBorderPainter(),
      mpAnimator(new PresenterAnimator())
{
    if ( ! mxSlideShowController.is())
        throw new lang::IllegalArgumentException(
            OUString::createFromAscii("missing slide show controller"),
            static_cast<XWeak*>(this),
            2);

    new PresenterCurrentSlideObserver(this,rxSlideShowController);

    // Listen for configuration changes.
    Reference<XControllerManager> xCM (mxController, UNO_QUERY_THROW);
    mxConfigurationController = xCM->getConfigurationController();
    if (mxConfigurationController.is())
    {
        mxConfigurationController->addConfigurationChangeListener(
            this,
            OUString::createFromAscii("ResourceActivation"),
            Any(ResourceActivationEventType));
        mxConfigurationController->addConfigurationChangeListener(
            this,
            OUString::createFromAscii("ResourceDeactivation"),
            Any(ResourceDeactivationEventType));
    }

    // Listen for the frame being activated.
    Reference<frame::XFrame> xFrame (mxController->getFrame());
    if (xFrame.is())
        xFrame->addFrameActionListener(this);

    // Create the border painter.
    mpPaneBorderPainter = new PresenterPaneBorderPainter(rxContext);
    mpWindowManager->SetPaneBorderPainter(mpPaneBorderPainter);

    if (mxSlideShowController.is())
        mxSlideShowController->activate();

    UpdateCurrentSlide(0);
}




PresenterController::~PresenterController (void)
{
}




void PresenterController::disposing (void)
{
    if (mxMainWindow.is())
    {
        mxMainWindow->removeKeyListener(this);
        mxMainWindow->removeFocusListener(this);
        mxMainWindow = NULL;
    }
    if (mxConfigurationController.is())
        mxConfigurationController->removeConfigurationChangeListener(this);

    Reference<XComponent> xWindowManagerComponent (
        static_cast<XWeak*>(mpWindowManager.get()), UNO_QUERY);
    if (xWindowManagerComponent.is())
        xWindowManagerComponent->dispose();

    if (mxController.is())
    {
        Reference<frame::XFrame> xFrame (mxController->getFrame());
        if (xFrame.is())
            xFrame->removeFrameActionListener(this);
        mxController = NULL;
    }
}




void PresenterController::UpdateCurrentSlide (const sal_Int32 nOffset)
{
    ToggleSlideSorter(Hide, 0);
    GetSlides(nOffset);
    UpdateViews();
}




void PresenterController::GetSlides (const sal_Int32 nOffset)
{
    if ( ! mxSlideShowController.is())
        return;

    // Get the current slide from the slide show controller.
    mxCurrentSlide = NULL;
    Reference<container::XIndexAccess> xIndexAccess(mxSlideShowController, UNO_QUERY);
    sal_Int32 nSlideIndex = -1;
    try
    {
        nSlideIndex = mxSlideShowController->getCurrentSlideIndex() + nOffset;
        if (mxSlideShowController->isPaused())
            nSlideIndex = -1;

        if (xIndexAccess.is() && nSlideIndex>=0)
        {
            if (nSlideIndex < xIndexAccess->getCount())
                mxCurrentSlide = Reference<drawing::XDrawPage>(
                    xIndexAccess->getByIndex(nSlideIndex), UNO_QUERY);
        }
    }
    catch (RuntimeException&)
    {
    }

    // Get the next slide.
    mxNextSlide = NULL;
    try
    {
        const sal_Int32 nNextSlideIndex (mxSlideShowController->getNextSlideIndex()+nOffset);
        if (nNextSlideIndex >= 0)
        {
            if (xIndexAccess.is())
            {
                if (nNextSlideIndex < xIndexAccess->getCount())
                    mxNextSlide = Reference<drawing::XDrawPage>(
                        xIndexAccess->getByIndex(nNextSlideIndex), UNO_QUERY);
            }
        }
    }
    catch (RuntimeException&)
    {
    }
}




void PresenterController::UpdateViews (void)
{
    // Tell all views about the slides they should display.
    PresenterPaneContainer::PaneList::const_iterator iPane;
    for (iPane=mpPaneContainer->maPanes.begin(); iPane!=mpPaneContainer->maPanes.end(); ++iPane)
    {
        Reference<drawing::XDrawView> xDrawView ((*iPane)->mxView, UNO_QUERY);
        if (xDrawView.is())
            xDrawView->setCurrentPage(mxCurrentSlide);
    }
}




sal_Int32 PresenterController::GetViewBackgroundColor (const OUString& rsViewURL) const
{
    sal_Int32 nColor (0x00ffffff);

    if (mpTheme.get() != NULL)
    {
        try
        {
            OUString sStyleName (mpTheme->GetViewStyle(rsViewURL));
            mpTheme->getPropertyValue(
                sStyleName+OUString::createFromAscii("_Background_Color")) >>= nColor;
        }
        catch(Exception&)
        {}
    }

    return nColor;
}




Reference<rendering::XBitmap> PresenterController::GetViewBackgroundBitmap (const OUString& rsViewURL) const
{
    Reference<rendering::XBitmap> xBitmap;

    if (mpTheme.get() != NULL)
    {
        try
        {
            OUString sStyleName (mpTheme->GetViewStyle(rsViewURL));
            mpTheme->getPropertyValue(
                sStyleName+OUString::createFromAscii("_Background_Bitmap")) >>= xBitmap;
        }
        catch(Exception&)
        {}
    }

    return xBitmap;
}




awt::FontDescriptor PresenterController::GetViewFontDescriptor (const OUString& rsViewURL) const
{
    awt::FontDescriptor aDescriptor;

    if (mpTheme.get() != NULL)
    {
        try
        {
            OUString sStyleName (mpTheme->GetViewStyle(rsViewURL));
            mpTheme->getPropertyValue(
                sStyleName+OUString::createFromAscii("_Font_Name")) >>= aDescriptor.Name;
            sal_Int32 nHeight = 0;
            mpTheme->getPropertyValue(
                sStyleName+OUString::createFromAscii("_Font_Size")) >>= nHeight;
            aDescriptor.Height = sal::static_int_cast<sal_Int16>(nHeight);
        }
        catch(Exception&)
        {
        }
    }
    return aDescriptor;
}




rendering::FontRequest PresenterController::GetViewFontRequest (const OUString& rsViewURL) const
{
    rendering::FontRequest aRequest;

    if (mpTheme.get() != NULL)
    {
        try
        {
            OUString sStyleName (mpTheme->GetViewStyle(rsViewURL));
            mpTheme->getPropertyValue(
                sStyleName+OUString::createFromAscii("_Font_Name"))
                >>= aRequest.FontDescription.FamilyName;
            sal_Int32 nHeight = 0;
            mpTheme->getPropertyValue(
                sStyleName+OUString::createFromAscii("_Font_Size")) >>= nHeight;
            aRequest.CellSize = sal::static_int_cast<sal_Int16>(nHeight);
        }
        catch(Exception&)
        {
        }
    }
    return aRequest;
}




util::Color PresenterController::GetViewFontColor (const OUString& rsViewURL) const
{
    sal_Int32 nColor (0);

    if (mpTheme.get() != NULL)
    {
        try
        {
            OUString sStyleName (mpTheme->GetViewStyle(rsViewURL));
            mpTheme->getPropertyValue(
                sStyleName+OUString::createFromAscii("_Font_Color")) >>= nColor;
        }
        catch(Exception&)
        {
        }
    }

    return nColor;
}




::boost::shared_ptr<PresenterTheme> PresenterController::GetTheme (void) const
{
    return mpTheme;
}




::rtl::Reference<PresenterWindowManager> PresenterController::GetWindowManager (void) const
{
    return mpWindowManager;
}




Reference<presentation::XSlideShowController>
    PresenterController::GetSlideShowController(void) const
{
    return mxSlideShowController;
}




Reference<awt::XWindow> PresenterController::GetParentWindow (void) const
{
    return mxMainWindow;
}




rtl::Reference<PresenterPaneContainer> PresenterController::GetPaneContainer (void) const
{
    return mpPaneContainer;
}




::rtl::Reference<PresenterPaneBorderPainter> PresenterController::GetPaneBorderPainter (void) const
{
    return mpPaneBorderPainter;
}




::boost::shared_ptr<PresenterAnimator> PresenterController::GetAnimator (void) const
{
    return mpAnimator;
}




void PresenterController::HideSlideSorter (void)
{
    if (mpCurrentPaneAnimation.get() != NULL)
    {
        mpCurrentPaneAnimation->HidePane();
        mpCurrentPaneAnimation.reset();
    }
}




//----- XConfigurationChangeListener ------------------------------------------

void SAL_CALL PresenterController::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    sal_Int32 nType (0);
    if ( ! (rEvent.UserData >>= nType))
        return;

    switch (nType)
    {
        case ResourceActivationEventType:
            if (rEvent.ResourceId->compareTo(mxMainPaneId) == 0)
            {
                InitializeMainPane(Reference<XPane>(rEvent.ResourceObject,UNO_QUERY));
            }
            else if (rEvent.ResourceId->isBoundTo(mxMainPaneId,AnchorBindingMode_DIRECT))
            {
                // A pane bound to the main pane has been created and is
                // stored in the pane container.
                Reference<XPane> xPane (rEvent.ResourceObject,UNO_QUERY);
                if (xPane.is())
                {
                    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
                        mpPaneContainer->FindPaneId(xPane->getResourceId()));
                    mpWindowManager->NotifyPaneCreation(pDescriptor);
                }
            }
            else if (rEvent.ResourceId->isBoundTo(mxMainPaneId,AnchorBindingMode_INDIRECT))
            {
                // A view bound to one of the panes has been created and is
                // stored in the pane container along with its pane.
                Reference<XView> xView (rEvent.ResourceObject,UNO_QUERY);
                if (xView.is())
                {
                    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
                        mpPaneContainer->StoreView(
                            xView,
                            GetViewBackgroundColor(xView->getResourceId()->getResourceURL()),
                            GetViewBackgroundBitmap(xView->getResourceId()->getResourceURL())));
                    OSL_ASSERT(pDescriptor.get() != NULL);
                    pDescriptor->mxPane->SetBackground(
                        GetViewBackgroundColor(xView->getResourceId()->getResourceURL()),
                        GetViewBackgroundBitmap(xView->getResourceId()->getResourceURL()));

                    UpdateViews();
                    mpWindowManager->NotifyViewCreation(xView);
                }
            }
            break;

        case ResourceDeactivationEventType:
            break;
    }
}




//----- XEventListener --------------------------------------------------------

void SAL_CALL PresenterController::disposing (
    const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Source == mxController)
        mxController = NULL;
    else if (rEvent.Source == mxConfigurationController)
        mxConfigurationController = NULL;
    else if (rEvent.Source == mxSlideShowController)
        mxSlideShowController = NULL;
    else if (rEvent.Source == mxMainWindow)
        mxMainWindow = NULL;
}




//----- XFrameActionListener --------------------------------------------------

void SAL_CALL PresenterController::frameAction (
    const frame::FrameActionEvent& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Action == frame::FrameAction_FRAME_ACTIVATED)
    {
        if (mxSlideShowController.is())
            mxSlideShowController->activate();
    }
}




//----- XKeyListener ----------------------------------------------------------

void SAL_CALL PresenterController::keyPressed (const awt::KeyEvent& rEvent)
    throw (RuntimeException)
{
    // Tell all views about the unhandled key event.
    PresenterPaneContainer::PaneList::const_iterator iPane;
    for (iPane=mpPaneContainer->maPanes.begin(); iPane!=mpPaneContainer->maPanes.end(); ++iPane)
    {
        if ( ! (*iPane)->mbIsActive)
            continue;

        Reference<awt::XKeyListener> xKeyListener ((*iPane)->mxView, UNO_QUERY);
        if (xKeyListener.is())
            xKeyListener->keyPressed(rEvent);
    }
}




void SAL_CALL PresenterController::keyReleased (const awt::KeyEvent& rEvent)
    throw (RuntimeException)
{
    switch (rEvent.KeyCode)
    {
        case awt::Key::ESCAPE:
        {
            if( mxController.is() )
            {
                Reference< XPresentationSupplier > xPS( mxController->getModel(), UNO_QUERY );
                if( xPS.is() )
                {
                    Reference< XPresentation > xP( xPS->getPresentation() );
                    if( xP.is() )
                        xP->end();
                }
            }
        }
        break;

        case awt::Key::SPACE:
        case awt::Key::PAGEDOWN:
        case awt::Key::RETURN:
            if (mxSlideShowController.is())
            {
                mxSlideShowController->gotoNextSlide();
            }
            break;

        case awt::Key::BACKSPACE:
        case awt::Key::PAGEUP:
            if (mxSlideShowController.is())
            {
                mxSlideShowController->gotoPreviousSlide();
            }
            break;

        case awt::Key::F1:
            ToggleHelpWindow(true);
            break;

        case awt::Key::F2:
            ToggleSlideSorter(Toggle, 2);
            break;

        case awt::Key::F3:
            ToggleSlideSorter(Toggle, 0);
            break;

        case awt::Key::F4:
            ToggleSlideSorter(Toggle, 1);
            break;

        default:
            // Tell all views about the unhandled key event.
            PresenterPaneContainer::PaneList::const_iterator iPane;
            for (iPane=mpPaneContainer->maPanes.begin(); iPane!=mpPaneContainer->maPanes.end(); ++iPane)
            {
                if ( ! (*iPane)->mbIsActive)
                    continue;

                Reference<awt::XKeyListener> xKeyListener ((*iPane)->mxView, UNO_QUERY);
                if (xKeyListener.is())
                    xKeyListener->keyReleased(rEvent);
            }
            break;
    }
}




//----- XFocusListener --------------------------------------------------------

void SAL_CALL PresenterController::focusGained (const css::awt::FocusEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterController::focusLost (const css::awt::FocusEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}




//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterController::mousePressed (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
    if (mxMainWindow.is())
        mxMainWindow->setFocus();
}




void SAL_CALL PresenterController::mouseReleased (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterController::mouseEntered (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterController::mouseExited (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}




//-----------------------------------------------------------------------------

void PresenterController::InitializeMainPane (const Reference<XPane>& rxPane)
{
    if ( ! rxPane.is())
        return;

    LoadTheme(rxPane);

    // Main pane has been created and is now observed by the window
    // manage.r
    mpWindowManager->SetParentPane(rxPane);
    mpWindowManager->SetTheme(mpTheme);

    if (mpPaneBorderPainter.get() != NULL)
        mpPaneBorderPainter->SetTheme(mpTheme);

    // Add key listener
    mxMainWindow = rxPane->getWindow();
    if (mxMainWindow.is())
    {
        mxMainWindow->addKeyListener(this);
        mxMainWindow->addFocusListener(this);
    }

    if (mxSlideShowController.is())
        mxSlideShowController->activate();
}




sal_Int32 PresenterController::GetBackgroundColor (void) const
{
    return mnBackgroundColor;
}




void PresenterController::LoadTheme (const Reference<XPane>& rxPane)
{
    // Create (load) the current theme.
    if (rxPane.is())
        mpTheme.reset(new PresenterTheme(mxContext, OUString(), rxPane->getCanvas()));

    // Store locally some values from the current theme.
    if (mpTheme.get() != NULL)
    {
        try
        {
            mpTheme->getPropertyValue(
                OUString::createFromAscii("Background_Color")) >>= mnBackgroundColor;
        }
        catch(Exception&)
        {}
    }
}




void PresenterController::ToggleHelpWindow (const bool bUseAnimation)
{
    if (mxConfigurationController.is())
    {

        Reference<XResourceId> xPaneId (ResourceId::createWithAnchorURL(
            mxContext,
            PresenterPaneFactory::msOverlayPaneURL + OUString::createFromAscii("?Sprite=1"),
            PresenterHelper::msFullScreenPaneURL));
        Reference<XResourceId> xViewId (ResourceId::createWithAnchor(
            mxContext,
            PresenterViewFactory::msHelpViewURL,
            xPaneId));

        if ( ! mxConfigurationController->getResource(xViewId).is())
        {
            mpPaneContainer->PreparePane(
                xPaneId,
                OUString::createFromAscii(""),
                PresenterPaneContainer::ViewInitializationFunction(),
                0.0,
                0.0,
                1.0,
                1.0);

            mxConfigurationController->requestResourceActivation(
                xPaneId,
                ResourceActivationMode_ADD);
            mxConfigurationController->requestResourceActivation(
                xViewId,
                ResourceActivationMode_REPLACE);

            // Show a pending hide animation.
            if (mpCurrentPaneAnimation.get() != NULL)
                mpCurrentPaneAnimation->HidePane();

            // Create a new pane animator object.
            EndActions aHideActions;
            aHideActions.push_back(::boost::bind(
                &drawing::framework::XConfigurationController::requestResourceDeactivation,
                mxConfigurationController,
                xViewId));
            mpCurrentPaneAnimation = CreateUnfoldInCenterAnimator(
                xPaneId, this, bUseAnimation, EndActions(), aHideActions);

            // Schedule the ShowPane execution.
            PresenterFrameworkObserver::RunOnResourceActivation(
                mxConfigurationController,
                xViewId,
                ::boost::bind(&PresenterPaneAnimator::ShowPane, mpCurrentPaneAnimation));
        }
        else
        {
            if (mpCurrentPaneAnimation.get() != NULL)
            {
                mpCurrentPaneAnimation->HidePane();
                mpCurrentPaneAnimation.reset();
            }
        }
    }
}




void PresenterController::ToggleSlideSorter (
    const ToggleState eState,
    const sal_Int32 nAppearMode)
{
    if (mxConfigurationController.is())
    {

        Reference<XResourceId> xPaneId (ResourceId::createWithAnchorURL(
            mxContext,
            PresenterPaneFactory::msOverlayPaneURL + OUString::createFromAscii("?Sprite=1"),
            //PresenterPaneFactory::msOverlayPaneURL,
            PresenterHelper::msFullScreenPaneURL));
        Reference<XResourceId> xSlideSorterViewId (ResourceId::createWithAnchor(
            mxContext,
            PresenterViewFactory::msSlideSorterURL,
            xPaneId));

        const bool bIsVisible (mxConfigurationController->getResource(xSlideSorterViewId).is());
        if ( ! bIsVisible && (eState==Toggle || eState==Show))
        {
            // Show a pending hide animation.
            if (mpCurrentPaneAnimation.get() != NULL)
            {
                mpCurrentPaneAnimation->HidePane();
                mpCurrentPaneAnimation.reset();
            }

            // Request slide sorter pane and view.
            mpPaneContainer->PreparePane(
                xPaneId,
                OUString::createFromAscii(""),
                PresenterPaneContainer::ViewInitializationFunction(),
                0.2,
                0.3,
                0.8,
                0.7);
            mxConfigurationController->requestResourceActivation(
                xPaneId,
                ResourceActivationMode_ADD);
            mxConfigurationController->requestResourceActivation(
                xSlideSorterViewId,
                ResourceActivationMode_REPLACE);

            EndActions aHideActions;
            aHideActions.push_back(::boost::bind(
                &drawing::framework::XConfigurationController::requestResourceDeactivation,
                mxConfigurationController,
                xSlideSorterViewId));

            switch (nAppearMode)
            {
                case 0:
                    mpCurrentPaneAnimation = CreateUnfoldInCenterAnimator(
                        xPaneId, this, true, EndActions(), aHideActions);
                    break;

                case 1:
                    mpCurrentPaneAnimation = CreateMoveInFromBottomAnimator(
                        xPaneId, this, true, EndActions(), aHideActions);
                    break;

                case 2:
                    mpCurrentPaneAnimation = CreateTransparentOverlay(
                        xPaneId, this, true, EndActions(), aHideActions);
                    break;
            }

            PresenterFrameworkObserver::RunOnResourceActivation(
                mxConfigurationController,
                xSlideSorterViewId,
                ::boost::bind(&PresenterPaneAnimator::ShowPane, mpCurrentPaneAnimation));
        }
        else if (bIsVisible && (eState==Toggle || eState==Hide))
        {
            if (mpCurrentPaneAnimation.get() != NULL)
            {
                mpCurrentPaneAnimation->HidePane();
                mpCurrentPaneAnimation.reset();
            }
        }
    }
}




void PresenterController::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterController object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}


} } // end of namespace ::sdext::presenter

