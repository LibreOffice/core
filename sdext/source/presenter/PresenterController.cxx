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

#include <string_view>

#include "PresenterController.hxx"

#include "PresenterAccessibility.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterCurrentSlideObserver.hxx"
#include "PresenterFrameworkObserver.hxx"
#include "PresenterHelper.hxx"
#include "PresenterScreen.hxx"
#include "PresenterNotesView.hxx"
#include "PresenterPaintManager.hxx"
#include "PresenterPaneBase.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterPaneBorderPainter.hxx"
#include "PresenterTheme.hxx"
#include "PresenterViewFactory.hxx"
#include "PresenterWindowManager.hxx"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/XPresentation.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::drawing::framework;

namespace {
    const sal_Int32 ResourceActivationEventType = 0;
    const sal_Int32 ResourceDeactivationEventType = 1;
    const sal_Int32 ConfigurationUpdateEndEventType = 2;
}

namespace sdext { namespace presenter {

IPresentationTime::~IPresentationTime()
{
}

PresenterController::InstanceContainer PresenterController::maInstances;

::rtl::Reference<PresenterController> PresenterController::Instance (
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    InstanceContainer::const_iterator iInstance (maInstances.find(rxFrame));
    if (iInstance != maInstances.end())
        return iInstance->second;
    else
        return ::rtl::Reference<PresenterController>();
}

PresenterController::PresenterController (
    const css::uno::WeakReference<css::lang::XEventListener> &rxScreen,
    const Reference<XComponentContext>& rxContext,
    const Reference<frame::XController>& rxController,
    const Reference<presentation::XSlideShowController>& rxSlideShowController,
    const rtl::Reference<PresenterPaneContainer>& rpPaneContainer,
    const Reference<XResourceId>& rxMainPaneId)
    : PresenterControllerInterfaceBase(m_aMutex),
      mxScreen(rxScreen),
      mxComponentContext(rxContext),
      mxController(rxController),
      mxConfigurationController(),
      mxSlideShowController(rxSlideShowController),
      mxMainPaneId(rxMainPaneId),
      mpPaneContainer(rpPaneContainer),
      mnCurrentSlideIndex(-1),
      mxCurrentSlide(),
      mxNextSlide(),
      mpWindowManager(new PresenterWindowManager(rxContext,mpPaneContainer,this)),
      mpTheme(),
      mxMainWindow(),
      mpPaneBorderPainter(),
      mpCanvasHelper(new PresenterCanvasHelper()),
      mxPresenterHelper(),
      mpPaintManager(),
      mnPendingSlideNumber(-1),
      mxUrlTransformer(),
      mpAccessibleObject(),
      mbIsAccessibilityActive(false)
{
    OSL_ASSERT(mxController.is());

    if ( ! mxSlideShowController.is())
        throw lang::IllegalArgumentException(
            "missing slide show controller",
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
            "ResourceActivation",
            Any(ResourceActivationEventType));
        mxConfigurationController->addConfigurationChangeListener(
            this,
            "ResourceDeactivation",
            Any(ResourceDeactivationEventType));
        mxConfigurationController->addConfigurationChangeListener(
            this,
            "ConfigurationUpdateEnd",
            Any(ConfigurationUpdateEndEventType));
    }

    // Listen for the frame being activated.
    Reference<frame::XFrame> xFrame (mxController->getFrame());
    if (xFrame.is())
        xFrame->addFrameActionListener(this);

    // Create the border painter.
    mpPaneBorderPainter = new PresenterPaneBorderPainter(rxContext);
    mpWindowManager->SetPaneBorderPainter(mpPaneBorderPainter);

    // Create an object that is able to load the bitmaps in a format that is
    // supported by the canvas.
    Reference<lang::XMultiComponentFactory> xFactory (
        rxContext->getServiceManager(), UNO_QUERY);
    if ( ! xFactory.is())
        return;
    mxPresenterHelper.set(
        xFactory->createInstanceWithContext(
            "com.sun.star.drawing.PresenterHelper",
            rxContext),
        UNO_QUERY_THROW);

    if (mxSlideShowController.is())
    {
        mxSlideShowController->activate();
        Reference<beans::XPropertySet> xProperties (mxSlideShowController, UNO_QUERY);
        if (xProperties.is())
        {
            Reference<awt::XWindow> xWindow (
                xProperties->getPropertyValue("ParentWindow"), UNO_QUERY);
            if (xWindow.is())
                xWindow->addKeyListener(this);
        }
    }

    UpdateCurrentSlide(0);

    maInstances[mxController->getFrame()] = this;

    // Create a URLTransformer.
    if (xFactory.is())
    {
        mxUrlTransformer.set(util::URLTransformer::create(mxComponentContext));
    }
}

PresenterController::~PresenterController()
{
}

void PresenterController::disposing()
{
    maInstances.erase(mxController->getFrame());

    if (mxMainWindow.is())
    {
        mxMainWindow->removeKeyListener(this);
        mxMainWindow->removeFocusListener(this);
        mxMainWindow->removeMouseListener(this);
        mxMainWindow->removeMouseMotionListener(this);
        mxMainWindow = nullptr;
    }
    if (mxConfigurationController.is())
        mxConfigurationController->removeConfigurationChangeListener(this);

    Reference<XComponent> xWindowManagerComponent (
        static_cast<XWeak*>(mpWindowManager.get()), UNO_QUERY);
    mpWindowManager = nullptr;
    if (xWindowManagerComponent.is())
        xWindowManagerComponent->dispose();

    if (mxController.is())
    {
        Reference<frame::XFrame> xFrame (mxController->getFrame());
        if (xFrame.is())
            xFrame->removeFrameActionListener(this);
        mxController = nullptr;
    }

    mxComponentContext = nullptr;
    mxConfigurationController = nullptr;
    mxSlideShowController = nullptr;
    mxMainPaneId = nullptr;
    mpPaneContainer = nullptr;
    mnCurrentSlideIndex = -1;
    mxCurrentSlide = nullptr;
    mxNextSlide = nullptr;
    mpTheme.reset();
    {
        Reference<lang::XComponent> xComponent (
            static_cast<XWeak*>(mpPaneBorderPainter.get()), UNO_QUERY);
        mpPaneBorderPainter = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }
    mpCanvasHelper.reset();
    {
        Reference<lang::XComponent> xComponent (mxPresenterHelper, UNO_QUERY);
        mxPresenterHelper = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }
    mpPaintManager.reset();
    mnPendingSlideNumber = -1;
    {
        Reference<lang::XComponent> xComponent (mxUrlTransformer, UNO_QUERY);
        mxUrlTransformer = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }
}

void PresenterController::UpdateCurrentSlide (const sal_Int32 nOffset)
{
    // std::cerr << "Updating current Slide to " << nOffset << std::endl;
    GetSlides(nOffset);
    UpdatePaneTitles();
    UpdateViews();

    // Update the accessibility object.
    if (IsAccessibilityActive())
    {
        mpAccessibleObject->NotifyCurrentSlideChange();
    }
}

void PresenterController::GetSlides (const sal_Int32 nOffset)
{
    if ( ! mxSlideShowController.is())
        return;

    // Get the current slide from the slide show controller.
    mxCurrentSlide = nullptr;
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
            {
                mnCurrentSlideIndex = nSlideIndex;
                mxCurrentSlide.set( xIndexAccess->getByIndex(nSlideIndex), UNO_QUERY);
            }
        }
    }
    catch (RuntimeException&)
    {
    }

    // Get the next slide.
    mxNextSlide = nullptr;
    try
    {
        const sal_Int32 nNextSlideIndex (mxSlideShowController->getNextSlideIndex()+nOffset);
        if (nNextSlideIndex >= 0)
        {
            if (xIndexAccess.is())
            {
                if (nNextSlideIndex < xIndexAccess->getCount())
                    mxNextSlide.set( xIndexAccess->getByIndex(nNextSlideIndex), UNO_QUERY);
            }
        }
    }
    catch (RuntimeException&)
    {
    }
}

void PresenterController::UpdatePaneTitles()
{
    if ( ! mxSlideShowController.is())
        return;

    // Get placeholders and their values.
    const OUString sCurrentSlideNumberPlaceholder ("CURRENT_SLIDE_NUMBER");
    const OUString sCurrentSlideNamePlaceholder ("CURRENT_SLIDE_NAME");
    const OUString sSlideCountPlaceholder ("SLIDE_COUNT");

    // Get string for slide count.
    OUString sSlideCount ("---");
    Reference<container::XIndexAccess> xIndexAccess(mxSlideShowController, UNO_QUERY);
    if (xIndexAccess.is())
        sSlideCount = OUString::number(xIndexAccess->getCount());

    // Get string for current slide index.
    OUString sCurrentSlideNumber (OUString::number(mnCurrentSlideIndex + 1));

    // Get name of the current slide.
    OUString sCurrentSlideName;
    Reference<container::XNamed> xNamedSlide (mxCurrentSlide, UNO_QUERY);
    if (xNamedSlide.is())
        sCurrentSlideName = xNamedSlide->getName();
    Reference<beans::XPropertySet> xSlideProperties (mxCurrentSlide, UNO_QUERY);
    if (xSlideProperties.is())
    {
        try
        {
            OUString sName;
            if (xSlideProperties->getPropertyValue("LinkDisplayName") >>= sName)
            {
                // Find out whether the name of the current slide has been
                // automatically created or has been set by the user.
                if (sName != sCurrentSlideName)
                    sCurrentSlideName = sName;
            }
        }
        catch (const beans::UnknownPropertyException&)
        {
        }
    }

    // Replace the placeholders with their current values.
    for (auto& rxPane : mpPaneContainer->maPanes)
    {
        OSL_ASSERT(rxPane != nullptr);

        OUString sTemplate (IsAccessibilityActive()
            ? rxPane->msAccessibleTitleTemplate
            : rxPane->msTitleTemplate);
        if (sTemplate.isEmpty())
            continue;

        OUStringBuffer sResult;
        sResult.ensureCapacity(sTemplate.getLength());

        sal_Int32 nIndex (0);
        while (true)
        {
            sal_Int32 nStartIndex = sTemplate.indexOf('%', nIndex);
            if (nStartIndex < 0)
            {
                // Add the remaining part of the string.
                sResult.append(std::u16string_view(sTemplate).substr(nIndex));
                break;
            }
            else
            {
                // Add the part preceding the next %.
                sResult.append(std::u16string_view(sTemplate).substr(nIndex, nStartIndex-nIndex));

                // Get the placeholder
                ++nStartIndex;
                const sal_Int32 nEndIndex (sTemplate.indexOf('%', nStartIndex+1));
                const OUString sPlaceholder (sTemplate.copy(nStartIndex, nEndIndex-nStartIndex));
                nIndex = nEndIndex+1;

                // Replace the placeholder with its current value.
                if (sPlaceholder == sCurrentSlideNumberPlaceholder)
                    sResult.append(sCurrentSlideNumber);
                else if (sPlaceholder == sCurrentSlideNamePlaceholder)
                    sResult.append(sCurrentSlideName);
                else if (sPlaceholder == sSlideCountPlaceholder)
                    sResult.append(sSlideCount);
            }
        }

        rxPane->msTitle = sResult.makeStringAndClear();
        if (rxPane->mxPane.is())
            rxPane->mxPane->SetTitle(rxPane->msTitle);
    }
}

void PresenterController::UpdateViews()
{
    // Tell all views about the slides they should display.
    for (const auto& rxPane : mpPaneContainer->maPanes)
    {
        Reference<drawing::XDrawView> xDrawView (rxPane->mxView, UNO_QUERY);
        if (xDrawView.is())
            xDrawView->setCurrentPage(mxCurrentSlide);
    }
}

SharedBitmapDescriptor
    PresenterController::GetViewBackground (const OUString& rsViewURL) const
{
    if (mpTheme != nullptr)
    {
        const OUString sStyleName (mpTheme->GetStyleName(rsViewURL));
        return mpTheme->GetBitmap(sStyleName, "Background");
    }
    return SharedBitmapDescriptor();
}

PresenterTheme::SharedFontDescriptor
    PresenterController::GetViewFont (const OUString& rsViewURL) const
{
    if (mpTheme != nullptr)
    {
        const OUString sStyleName (mpTheme->GetStyleName(rsViewURL));
        return mpTheme->GetFont(sStyleName);
    }
    return PresenterTheme::SharedFontDescriptor();
}

const std::shared_ptr<PresenterTheme>& PresenterController::GetTheme() const
{
    return mpTheme;
}

const ::rtl::Reference<PresenterWindowManager>& PresenterController::GetWindowManager() const
{
    return mpWindowManager;
}

const Reference<presentation::XSlideShowController>&
    PresenterController::GetSlideShowController() const
{
    return mxSlideShowController;
}

const rtl::Reference<PresenterPaneContainer>& PresenterController::GetPaneContainer() const
{
    return mpPaneContainer;
}

const ::rtl::Reference<PresenterPaneBorderPainter>& PresenterController::GetPaneBorderPainter() const
{
    return mpPaneBorderPainter;
}

const std::shared_ptr<PresenterCanvasHelper>& PresenterController::GetCanvasHelper() const
{
    return mpCanvasHelper;
}

const Reference<drawing::XPresenterHelper>& PresenterController::GetPresenterHelper() const
{
    return mxPresenterHelper;
}

const std::shared_ptr<PresenterPaintManager>& PresenterController::GetPaintManager() const
{
    return mpPaintManager;
}

void PresenterController::ShowView (const OUString& rsViewURL)
{
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindViewURL(rsViewURL));
    if (pDescriptor.get() == nullptr)
        return;

    pDescriptor->mbIsActive = true;
    mxConfigurationController->requestResourceActivation(
        pDescriptor->mxPaneId,
        ResourceActivationMode_ADD);
    mxConfigurationController->requestResourceActivation(
        ResourceId::createWithAnchor(
            mxComponentContext,
            rsViewURL,
            pDescriptor->mxPaneId),
        ResourceActivationMode_REPLACE);
}

void PresenterController::HideView (const OUString& rsViewURL)
{
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindViewURL(rsViewURL));
    if (pDescriptor.get() != nullptr)
    {
        mxConfigurationController->requestResourceDeactivation(
            ResourceId::createWithAnchor(
                mxComponentContext,
                rsViewURL,
                pDescriptor->mxPaneId));
    }
}

void PresenterController::DispatchUnoCommand (const OUString& rsCommand) const
{
    if ( ! mxUrlTransformer.is())
        return;

    util::URL aURL;
    aURL.Complete = rsCommand;
    mxUrlTransformer->parseStrict(aURL);

    Reference<frame::XDispatch> xDispatch (GetDispatch(aURL));
    if ( ! xDispatch.is())
        return;

    xDispatch->dispatch(aURL, Sequence<beans::PropertyValue>());
}

Reference<css::frame::XDispatch> PresenterController::GetDispatch (const util::URL& rURL) const
{
    if ( ! mxController.is())
        return nullptr;

    Reference<frame::XDispatchProvider> xDispatchProvider (mxController->getFrame(), UNO_QUERY);
    if ( ! xDispatchProvider.is())
        return nullptr;

    return xDispatchProvider->queryDispatch(
        rURL,
        OUString(),
        frame::FrameSearchFlag::SELF);
}

util::URL PresenterController::CreateURLFromString (const OUString& rsURL) const
{
    util::URL aURL;

    if (mxUrlTransformer.is())
    {
        aURL.Complete = rsURL;
        mxUrlTransformer->parseStrict(aURL);
    }

    return aURL;
}

const Reference<drawing::framework::XConfigurationController>&
    PresenterController::GetConfigurationController() const
{
    return mxConfigurationController;
}

const Reference<drawing::XDrawPage>& PresenterController::GetCurrentSlide() const
{
    return mxCurrentSlide;
}

bool PresenterController::HasTransition (Reference<drawing::XDrawPage> const & rxPage)
{
    bool bTransition = false;
    sal_uInt16 aTransitionType = 0;
    if( rxPage.is() )
    {
        Reference<beans::XPropertySet> xSlidePropertySet (rxPage, UNO_QUERY);
        try
        {
            xSlidePropertySet->getPropertyValue("TransitionType") >>= aTransitionType;
            if (aTransitionType > 0)
            {
                bTransition = true;
            }
        }
        catch (const beans::UnknownPropertyException&)
        {
        }
    }
    return bTransition;
}

bool PresenterController::HasCustomAnimation (Reference<drawing::XDrawPage> const & rxPage)
{
    bool bCustomAnimation = false;
    if( rxPage.is() )
    {
        sal_uInt32 i, nCount = rxPage->getCount();
        for ( i = 0; i < nCount; i++ )
        {
            Reference<drawing::XShape> xShape(rxPage->getByIndex(i), UNO_QUERY);
            Reference<beans::XPropertySet> xShapePropertySet(xShape, UNO_QUERY);
            presentation::AnimationEffect aEffect = presentation::AnimationEffect_NONE;
            presentation::AnimationEffect aTextEffect = presentation::AnimationEffect_NONE;
            try
            {
                xShapePropertySet->getPropertyValue("Effect") >>= aEffect;
                xShapePropertySet->getPropertyValue("TextEffect") >>= aTextEffect;
            }
            catch (const beans::UnknownPropertyException&)
            {
            }
            if( aEffect != presentation::AnimationEffect_NONE ||
                aTextEffect != presentation::AnimationEffect_NONE )
            {
                bCustomAnimation = true;
                break;
            }
        }
    }
    return bCustomAnimation;
}

void PresenterController::SetAccessibilityActiveState (const bool bIsActive)
{
    if ( mbIsAccessibilityActive != bIsActive)
    {
        mbIsAccessibilityActive = bIsActive;
        UpdatePaneTitles();
    }
}


void PresenterController::HandleMouseClick (const awt::MouseEvent& rEvent)
{
    if (!mxSlideShowController.is())
        return;

    switch (rEvent.Buttons)
    {
        case awt::MouseButton::LEFT:
            if (rEvent.Modifiers == awt::KeyModifier::MOD2)
                mxSlideShowController->gotoNextSlide();
            else
                mxSlideShowController->gotoNextEffect();
            break;

        case awt::MouseButton::RIGHT:
            mxSlideShowController->gotoPreviousSlide();
            break;

        default:
            // Other or multiple buttons.
            break;
    }
}

void PresenterController::RequestViews (
    const bool bIsSlideSorterActive,
    const bool bIsNotesViewActive,
    const bool bIsHelpViewActive)
{
    for (const auto& rxPane : mpPaneContainer->maPanes)
    {
        bool bActivate (true);
        const OUString sViewURL (rxPane->msViewURL);
        if (sViewURL == PresenterViewFactory::msNotesViewURL)
        {
            bActivate = bIsNotesViewActive && !bIsSlideSorterActive && !bIsHelpViewActive;
        }
        else if (sViewURL == PresenterViewFactory::msSlideSorterURL)
        {
            bActivate = bIsSlideSorterActive;
        }
        else if (sViewURL == PresenterViewFactory::msCurrentSlidePreviewViewURL
            || sViewURL == PresenterViewFactory::msNextSlidePreviewViewURL)
        {
            bActivate = !bIsSlideSorterActive && ! bIsHelpViewActive;
        }
        else if (sViewURL == PresenterViewFactory::msToolBarViewURL)
        {
            bActivate = true;
        }
        else if (sViewURL == PresenterViewFactory::msHelpViewURL)
        {
            bActivate = bIsHelpViewActive;
        }

        if (bActivate)
            ShowView(sViewURL);
        else
            HideView(sViewURL);
    }
}

void PresenterController::SetPresentationTime(IPresentationTime* pPresentationTime)
{
    mpPresentationTime = pPresentationTime;
}

IPresentationTime* PresenterController::GetPresentationTime()
{
    return mpPresentationTime;
}

//----- XConfigurationChangeListener ------------------------------------------

void SAL_CALL PresenterController::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            "PresenterController object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }

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
                }
            }
            else if (rEvent.ResourceId->isBoundTo(mxMainPaneId,AnchorBindingMode_INDIRECT))
            {
                // A view bound to one of the panes has been created and is
                // stored in the pane container along with its pane.
                Reference<XView> xView (rEvent.ResourceObject,UNO_QUERY);
                if (xView.is())
                {
                    mpPaneContainer->StoreView(xView);
                    UpdateViews();
                    mpWindowManager->NotifyViewCreation(xView);
                }
            }
            break;

        case ResourceDeactivationEventType:
            if (rEvent.ResourceId->isBoundTo(mxMainPaneId,AnchorBindingMode_INDIRECT))
            {
                // If this is a view then remove it from the pane container.
                Reference<XView> xView (rEvent.ResourceObject,UNO_QUERY);
                if (xView.is())
                {
                    PresenterPaneContainer::SharedPaneDescriptor pDescriptor(
                        mpPaneContainer->RemoveView(xView));

                    // A possibly opaque view has been removed.  Update()
                    // updates the clip polygon.
                    mpWindowManager->Update();
                    // Request the repainting of the area previously
                    // occupied by the view.
                    if (pDescriptor.get() != nullptr)
                        GetPaintManager()->Invalidate(pDescriptor->mxBorderWindow);
                }
            }
            break;

        case ConfigurationUpdateEndEventType:
            if (IsAccessibilityActive())
            {
                mpAccessibleObject->UpdateAccessibilityHierarchy();
                UpdateCurrentSlide(0);
            }
            break;
    }
}

//----- XEventListener --------------------------------------------------------

void SAL_CALL PresenterController::disposing (
    const lang::EventObject& rEvent)
{
    if (rEvent.Source == mxController)
        mxController = nullptr;
    else if (rEvent.Source == mxConfigurationController)
        mxConfigurationController = nullptr;
    else if (rEvent.Source == mxSlideShowController)
        mxSlideShowController = nullptr;
    else if (rEvent.Source == mxMainWindow)
        mxMainWindow = nullptr;
}

//----- XFrameActionListener --------------------------------------------------

void SAL_CALL PresenterController::frameAction (
    const frame::FrameActionEvent& rEvent)
{
    if (rEvent.Action == frame::FrameAction_FRAME_ACTIVATED)
    {
        if (mxSlideShowController.is())
            mxSlideShowController->activate();
    }
}

//----- XKeyListener ----------------------------------------------------------

void SAL_CALL PresenterController::keyPressed (const awt::KeyEvent& rEvent)
{
    // Tell all views about the unhandled key event.
    for (const auto& rxPane : mpPaneContainer->maPanes)
    {
        if ( ! rxPane->mbIsActive)
            continue;

        Reference<awt::XKeyListener> xKeyListener (rxPane->mxView, UNO_QUERY);
        if (xKeyListener.is())
            xKeyListener->keyPressed(rEvent);
    }
}

void SAL_CALL PresenterController::keyReleased (const awt::KeyEvent& rEvent)
{
    if (rEvent.Source != mxMainWindow)
        return;

    switch (rEvent.KeyCode)
    {
        case awt::Key::ESCAPE:
        case awt::Key::SUBTRACT:
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

        case awt::Key::PAGEDOWN:
            if (mxSlideShowController.is())
            {
                if (rEvent.Modifiers == awt::KeyModifier::MOD2)
                    mxSlideShowController->gotoNextSlide();
                else
                    mxSlideShowController->gotoNextEffect();
            }
            break;

        case awt::Key::RIGHT:
        case awt::Key::SPACE:
        case awt::Key::DOWN:
        case awt::Key::N:
            if (mxSlideShowController.is())
            {
                mxSlideShowController->gotoNextEffect();
            }
            break;

        case awt::Key::PAGEUP:
            if (mxSlideShowController.is())
            {
                if (rEvent.Modifiers == awt::KeyModifier::MOD2)
                    mxSlideShowController->gotoPreviousSlide();
                else
                    mxSlideShowController->gotoPreviousEffect();
            }
            break;

        case awt::Key::LEFT:
        case awt::Key::UP:
        case awt::Key::P:
        case awt::Key::BACKSPACE:
            if (mxSlideShowController.is())
            {
                mxSlideShowController->gotoPreviousEffect();
            }
            break;

        case awt::Key::HOME:
            if (mxSlideShowController.is())
            {
                mxSlideShowController->gotoFirstSlide();
            }
            break;

        case awt::Key::END:
            if (mxSlideShowController.is())
            {
                mxSlideShowController->gotoLastSlide();
            }
            break;

        case awt::Key::W:
        case awt::Key::COMMA:
            if (mxSlideShowController.is())
            {
                if (mxSlideShowController->isPaused())
                    mxSlideShowController->resume();
                else
                    mxSlideShowController->blankScreen(0x00ffffff);
            }
            break;

        case awt::Key::B:
        case awt::Key::POINT:
            if (mxSlideShowController.is())
            {
                if (mxSlideShowController->isPaused())
                    mxSlideShowController->resume();
                else
                    mxSlideShowController->blankScreen(0x00000000);
            }
            break;

        case awt::Key::NUM0:
        case awt::Key::NUM1:
        case awt::Key::NUM2:
        case awt::Key::NUM3:
        case awt::Key::NUM4:
        case awt::Key::NUM5:
        case awt::Key::NUM6:
        case awt::Key::NUM7:
        case awt::Key::NUM8:
        case awt::Key::NUM9:
            HandleNumericKeyPress(rEvent.KeyCode-awt::Key::NUM0, rEvent.Modifiers);
            break;

        case awt::Key::RETURN:
            if (mnPendingSlideNumber > 0)
            {
                if (mxSlideShowController.is())
                    mxSlideShowController->gotoSlideIndex(mnPendingSlideNumber - 1);
                mnPendingSlideNumber = -1;
            }
            else
            {
                if (mxSlideShowController.is())
                    mxSlideShowController->gotoNextEffect();
            }

            break;

        case awt::Key::F1:
            // Toggle the help view.
            if (mpWindowManager.get() != nullptr)
            {
                if (mpWindowManager->GetViewMode() != PresenterWindowManager::VM_Help)
                    mpWindowManager->SetViewMode(PresenterWindowManager::VM_Help);
                else
                    mpWindowManager->SetHelpViewState(false);
            }

            break;

        default:
            // Tell all views about the unhandled key event.
            for (const auto& rxPane : mpPaneContainer->maPanes)
            {
                if ( ! rxPane->mbIsActive)
                    continue;

                Reference<awt::XKeyListener> xKeyListener (rxPane->mxView, UNO_QUERY);
                if (xKeyListener.is())
                    xKeyListener->keyReleased(rEvent);
            }
            break;
    }
}

void PresenterController::HandleNumericKeyPress (
    const sal_Int32 nKey,
    const sal_Int32 nModifiers)
{
    switch (nModifiers)
    {
        case 0:
            if (mnPendingSlideNumber == -1)
                mnPendingSlideNumber = 0;
            UpdatePendingSlideNumber(mnPendingSlideNumber * 10 + nKey);
            break;

        case awt::KeyModifier::MOD1:
            // Ctrl-1, Ctrl-2, and Ctrl-3 are used to switch between views
            // (slide view, notes view, normal). Ctrl-4 switches monitors
            mnPendingSlideNumber = -1;
            if (mpWindowManager.get() == nullptr)
                return;
            switch(nKey)
            {
                case 1:
                    mpWindowManager->SetViewMode(PresenterWindowManager::VM_Standard);
                    break;
                case 2:
                    mpWindowManager->SetViewMode(PresenterWindowManager::VM_Notes);
                    break;
                case 3:
                    mpWindowManager->SetViewMode(PresenterWindowManager::VM_SlideOverview);
                    break;
                case 4:
                    SwitchMonitors();
                    break;
                default:
                    // Ignore unsupported key.
                    break;
            }
            break;

        default:
            // Ignore unsupported modifiers.
            break;
    }
}

//----- XFocusListener --------------------------------------------------------

void SAL_CALL PresenterController::focusGained (const css::awt::FocusEvent&) {}

void SAL_CALL PresenterController::focusLost (const css::awt::FocusEvent&) {}

//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterController::mousePressed (const css::awt::MouseEvent&)
{
    if (mxMainWindow.is())
        mxMainWindow->setFocus();
}

void SAL_CALL PresenterController::mouseReleased (const css::awt::MouseEvent&) {}

void SAL_CALL PresenterController::mouseEntered (const css::awt::MouseEvent&) {}

void SAL_CALL PresenterController::mouseExited (const css::awt::MouseEvent&) {}

//----- XMouseMotionListener --------------------------------------------------

void SAL_CALL PresenterController::mouseMoved (const css::awt::MouseEvent&) {}

void SAL_CALL PresenterController::mouseDragged (const css::awt::MouseEvent&) {}


void PresenterController::InitializeMainPane (const Reference<XPane>& rxPane)
{
    if ( ! rxPane.is())
        return;

    mpAccessibleObject = new PresenterAccessible(
        mxComponentContext,
        this,
        rxPane);

    LoadTheme(rxPane);

    // Main pane has been created and is now observed by the window
    // manager.
    mpWindowManager->SetParentPane(rxPane);
    mpWindowManager->SetTheme(mpTheme);

    if (mpPaneBorderPainter.get() != nullptr)
        mpPaneBorderPainter->SetTheme(mpTheme);

    // Add key listener
    mxMainWindow = rxPane->getWindow();
    if (mxMainWindow.is())
    {
        mxMainWindow->addKeyListener(this);
        mxMainWindow->addFocusListener(this);
        mxMainWindow->addMouseListener(this);
        mxMainWindow->addMouseMotionListener(this);
    }
    Reference<XPane2> xPane2 (rxPane, UNO_QUERY);
    if (xPane2.is())
        xPane2->setVisible(true);

    mpPaintManager.reset(new PresenterPaintManager(mxMainWindow, mxPresenterHelper, mpPaneContainer));

    mxCanvas.set(rxPane->getCanvas(), UNO_QUERY);

    if (mxSlideShowController.is())
        mxSlideShowController->activate();

    UpdateCurrentSlide(0);
}

void PresenterController::LoadTheme (const Reference<XPane>& rxPane)
{
    // Create (load) the current theme.
    if (rxPane.is())
        mpTheme.reset(new PresenterTheme(mxComponentContext, rxPane->getCanvas()));
}

double PresenterController::GetSlideAspectRatio() const
{
    double nSlideAspectRatio (28.0/21.0);

    try
    {
        if (mxController.is())
        {
            Reference<drawing::XDrawPagesSupplier> xSlideSupplier (
                mxController->getModel(), UNO_QUERY_THROW);
            Reference<drawing::XDrawPages> xSlides (xSlideSupplier->getDrawPages());
            if (xSlides.is() && xSlides->getCount()>0)
            {
                Reference<beans::XPropertySet> xProperties(xSlides->getByIndex(0),UNO_QUERY_THROW);
                sal_Int32 nWidth (28000);
                sal_Int32 nHeight (21000);
                if ((xProperties->getPropertyValue("Width") >>= nWidth)
                    && (xProperties->getPropertyValue("Height") >>= nHeight)
                    && nHeight > 0)
                {
                    nSlideAspectRatio = double(nWidth) / double(nHeight);
                }
            }
        }
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
    }

    return nSlideAspectRatio;
}

void PresenterController::UpdatePendingSlideNumber (const sal_Int32 nPendingSlideNumber)
{
    mnPendingSlideNumber = nPendingSlideNumber;

    if (mpTheme == nullptr)
        return;

    if ( ! mxMainWindow.is())
        return;

    PresenterTheme::SharedFontDescriptor pFont (
        mpTheme->GetFont("PendingSlideNumberFont"));
    if (pFont.get() == nullptr)
        return;

    pFont->PrepareFont(Reference<rendering::XCanvas>(mxCanvas, UNO_QUERY));
    if ( ! pFont->mxFont.is())
        return;

    const OUString sText (OUString::number(mnPendingSlideNumber));
    rendering::StringContext aContext (sText, 0, sText.getLength());
    pFont->mxFont->createTextLayout(
            aContext,
            rendering::TextDirection::WEAK_LEFT_TO_RIGHT,
            0);
}

void PresenterController::SwitchMonitors()
{
    Reference<lang::XEventListener> xScreen( mxScreen );
    if (!xScreen.is())
        return;

    PresenterScreen *pScreen = dynamic_cast<PresenterScreen *>(xScreen.get());
    if (!pScreen)
        return;

    pScreen->SwitchMonitors();
}

} } // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
