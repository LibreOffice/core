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

#include "PresenterScreen.hxx"
#include "PresenterConfigurationAccess.hxx"
#include "PresenterController.hxx"
#include "PresenterFrameworkObserver.hxx"
#include "PresenterHelper.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterPaneFactory.hxx"
#include "PresenterViewFactory.hxx"
#include "PresenterWindowManager.hxx"
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/framework/Configuration.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>
#include <com/sun/star/presentation/XSlideShow.hpp>
#include <com/sun/star/presentation/XPresentation2.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <cppuhelper/compbase.hxx>

#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::drawing::framework;

namespace sdext { namespace presenter {

namespace {
    typedef ::cppu::WeakComponentImplHelper <
        css::document::XEventListener
        > PresenterScreenListenerInterfaceBase;

    /** One instance of a PresenterScreenListener is registered per Impress
        document and waits for the full screen slide show to start and to
        end.
    */
    class PresenterScreenListener
        : private ::boost::noncopyable,
          private ::cppu::BaseMutex,
          public PresenterScreenListenerInterfaceBase
    {
    public:
        PresenterScreenListener (
            const css::uno::Reference<css::uno::XComponentContext>& rxContext,
            const css::uno::Reference<css::frame::XModel2>& rxModel);
        virtual ~PresenterScreenListener();

        void Initialize();
        virtual void SAL_CALL disposing() override;

        // document::XEventListener

        virtual void SAL_CALL notifyEvent( const css::document::EventObject& Event ) throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener

        virtual void SAL_CALL disposing ( const css::lang::EventObject& rEvent) throw (css::uno::RuntimeException, std::exception) override;

    private:
        css::uno::Reference<css::frame::XModel2 > mxModel;
        css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
        rtl::Reference<PresenterScreen> mpPresenterScreen;

        void ThrowIfDisposed() const throw (css::lang::DisposedException);
    };
}

//----- Service ---------------------------------------------------------------

OUString PresenterScreenJob::getImplementationName_static()
{
    return OUString("org.libreoffice.comp.PresenterScreenJob");
}

Sequence<OUString> PresenterScreenJob::getSupportedServiceNames_static()
{
    return Sequence<OUString>();
}

Reference<XInterface> PresenterScreenJob::Create (const Reference<uno::XComponentContext>& rxContext)
{
    return Reference<XInterface>(static_cast<XWeak*>(new PresenterScreenJob(rxContext)));
}

//===== PresenterScreenJob ====================================================

PresenterScreenJob::PresenterScreenJob (const Reference<XComponentContext>& rxContext)
    : PresenterScreenJobInterfaceBase(m_aMutex),
      mxComponentContext(rxContext)
{
}

PresenterScreenJob::~PresenterScreenJob()
{
}

void SAL_CALL PresenterScreenJob::disposing()
{
    mxComponentContext = nullptr;
}

//----- XJob -----------------------------------------------------------

Any SAL_CALL PresenterScreenJob::execute(
    const Sequence< beans::NamedValue >& Arguments )
    throw (lang::IllegalArgumentException, Exception, RuntimeException, std::exception)
{
    Sequence< beans::NamedValue > lEnv;

    sal_Int32               i = 0;
    sal_Int32               c = Arguments.getLength();
    const beans::NamedValue* p = Arguments.getConstArray();
    for (i=0; i<c; ++i)
    {
        if ( p[i].Name == "Environment" )
        {
            p[i].Value >>= lEnv;
            break;
        }
    }

    Reference<frame::XModel2> xModel;
    c = lEnv.getLength();
    p = lEnv.getConstArray();
    for (i=0; i<c; ++i)
    {
        if ( p[i].Name == "Model" )
        {
            p[i].Value >>= xModel;
            break;
        }
    }

    Reference< XServiceInfo > xInfo( xModel, UNO_QUERY );
    if( xInfo.is() && xInfo->supportsService("com.sun.star.presentation.PresentationDocument") )
    {
        // Create a new listener that waits for the full screen presentation
        // to start and to end.  It takes care of its own lifetime.
        ::rtl::Reference<PresenterScreenListener> pListener (
            new PresenterScreenListener(mxComponentContext, xModel));
        pListener->Initialize();
    }

    return Any();
}

//===== PresenterScreenListener ===============================================

namespace {

PresenterScreenListener::PresenterScreenListener (
    const css::uno::Reference<css::uno::XComponentContext>& rxContext,
    const css::uno::Reference<css::frame::XModel2>& rxModel)
    : PresenterScreenListenerInterfaceBase(m_aMutex),
      mxModel(rxModel),
      mxComponentContext(rxContext),
      mpPresenterScreen()
{
}

void PresenterScreenListener::Initialize()
{
    Reference< document::XEventListener > xDocListener(
        static_cast< document::XEventListener* >(this), UNO_QUERY);
    Reference< document::XEventBroadcaster > xDocBroadcaster( mxModel, UNO_QUERY );
    if( xDocBroadcaster.is() )
        xDocBroadcaster->addEventListener(xDocListener);
}

PresenterScreenListener::~PresenterScreenListener()
{
}

void SAL_CALL PresenterScreenListener::disposing()
{
    Reference< document::XEventBroadcaster > xDocBroadcaster( mxModel, UNO_QUERY );
    if( xDocBroadcaster.is() )
        xDocBroadcaster->removeEventListener(
            Reference<document::XEventListener>(
                static_cast<document::XEventListener*>(this), UNO_QUERY));

    if (mpPresenterScreen.is())
    {
        mpPresenterScreen->RequestShutdownPresenterScreen();
        mpPresenterScreen = nullptr;
    }
}

// document::XEventListener

void SAL_CALL PresenterScreenListener::notifyEvent( const css::document::EventObject& Event ) throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    if ( Event.EventName == "OnStartPresentation" )
    {
        mpPresenterScreen = new PresenterScreen(mxComponentContext, mxModel);
        if(PresenterScreen::isPresenterScreenEnabled(mxComponentContext))
            mpPresenterScreen->InitializePresenterScreen();
    }
    else if ( Event.EventName == "OnEndPresentation" )
    {
        if (mpPresenterScreen.is())
        {
            mpPresenterScreen->RequestShutdownPresenterScreen();
            mpPresenterScreen = nullptr;
        }
    }
}

// XEventListener

void SAL_CALL PresenterScreenListener::disposing (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    (void)rEvent;

    if (mpPresenterScreen.is())
    {
        mpPresenterScreen->RequestShutdownPresenterScreen();
        mpPresenterScreen = nullptr;
    }
}

void PresenterScreenListener::ThrowIfDisposed() const throw (
    css::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(
                "PresenterScreenListener object has already been disposed"),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

} // end of anonymous namespace

//===== PresenterScreen =======================================================

PresenterScreen::PresenterScreen (
    const Reference<XComponentContext>& rxContext,
    const css::uno::Reference<css::frame::XModel2>& rxModel)
    : PresenterScreenInterfaceBase(m_aMutex),
      mxModel(rxModel),
      mxController(),
      mxConfigurationControllerWeak(),
      mxContextWeak(rxContext),
      mxSlideShowControllerWeak(),
      mpPresenterController(),
      mxSlideShowViewId(),
      mxSavedConfiguration(),
      mpPaneContainer(),
      mnComponentIndex(0),
      mxPaneFactory(),
      mxViewFactory(),
      maViewDescriptors()
{
}

PresenterScreen::~PresenterScreen()
{
}

bool PresenterScreen::isPresenterScreenEnabled(const css::uno::Reference<css::uno::XComponentContext>& rxContext)
{
        bool dEnablePresenterScreen=true;
        PresenterConfigurationAccess aConfiguration (
            rxContext,
            OUString("/org.openoffice.Office.Impress/"),
            PresenterConfigurationAccess::READ_ONLY);
        aConfiguration.GetConfigurationNode("Misc/Start/EnablePresenterScreen")
            >>= dEnablePresenterScreen;
        return dEnablePresenterScreen;
}
void SAL_CALL PresenterScreen::disposing()
{
    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is() && mxSavedConfiguration.is())
    {
        xCC->restoreConfiguration(mxSavedConfiguration);
    }
    mxConfigurationControllerWeak = Reference<XConfigurationController>(nullptr);

    Reference<lang::XComponent> xViewFactoryComponent (mxViewFactory, UNO_QUERY);
    if (xViewFactoryComponent.is())
       xViewFactoryComponent->dispose();
    Reference<lang::XComponent> xPaneFactoryComponent (mxPaneFactory, UNO_QUERY);
    if (xPaneFactoryComponent.is())
        xPaneFactoryComponent->dispose();

    mxModel = nullptr;
}

//----- XEventListener --------------------------------------------------------

void SAL_CALL PresenterScreen::disposing (const lang::EventObject& /*rEvent*/)
    throw (RuntimeException, std::exception)
{
    mxSlideShowControllerWeak = WeakReference<presentation::XSlideShowController>();
    RequestShutdownPresenterScreen();
}


void PresenterScreen::InitializePresenterScreen()
{
    try
    {
        Reference<XComponentContext> xContext (mxContextWeak);
        mpPaneContainer =
            new PresenterPaneContainer(Reference<XComponentContext>(xContext));

        Reference<XPresentationSupplier> xPS ( mxModel, UNO_QUERY_THROW);
        Reference<XPresentation2> xPresentation(xPS->getPresentation(), UNO_QUERY_THROW);
        Reference<presentation::XSlideShowController> xSlideShowController( xPresentation->getController() );
        mxSlideShowControllerWeak = xSlideShowController;

        if( !xSlideShowController.is() || !xSlideShowController->isFullScreen() )
            return;

        // find first controller that is not the current controller (the one with the slideshow
        mxController = mxModel->getCurrentController();
        Reference< container::XEnumeration > xEnum( mxModel->getControllers() );
        if( xEnum.is() )
        {
            while( xEnum->hasMoreElements() )
            {
                Reference< frame::XController > xC( xEnum->nextElement(), UNO_QUERY );
                if( xC.is() && (xC != mxController) )
                {
                    mxController = xC;
                    break;
                }
            }
        }
        // Get the XController from the first argument.
        Reference<XControllerManager> xCM(mxController, UNO_QUERY_THROW);

        Reference<XConfigurationController> xCC( xCM->getConfigurationController());
        mxConfigurationControllerWeak = xCC;

        Reference<drawing::framework::XResourceId> xMainPaneId(
            GetMainPaneId(xPresentation));
        // An empty reference means that the presenter screen can
        // not or must not be displayed.
        if ( ! xMainPaneId.is())
            return;

        if (xCC.is() && xContext.is())
        {
            // Store the current configuration so that we can restore it when
            // the presenter view is deactivated.
            mxSavedConfiguration = xCC->getRequestedConfiguration();
            xCC->lock();

            try
            {
                // At the moment the presenter controller is displayed in its
                // own full screen window that is controlled by the same
                // configuration controller as the Impress document from
                // which the presentation was started.  Therefore the main
                // pane is activated additionally to the already existing
                // panes and does not replace them.
                xCC->requestResourceActivation(
                    xMainPaneId,
                    ResourceActivationMode_ADD);
                SetupConfiguration(xContext, xMainPaneId);

                mpPresenterController = new PresenterController(
                    css::uno::WeakReference<css::lang::XEventListener>(this),
                    xContext,
                    mxController,
                    xSlideShowController,
                    mpPaneContainer,
                    xMainPaneId);

                // Create pane and view factories and integrate them into the
                // drawing framework.
                SetupPaneFactory(xContext);
                SetupViewFactory(xContext);

                mpPresenterController->GetWindowManager()->RestoreViewMode();
            }
            catch (const RuntimeException&)
            {
                xCC->restoreConfiguration(mxSavedConfiguration);
            }
            xCC->unlock();
        }
    }
    catch (const Exception&)
    {
    }
}

void PresenterScreen::SwitchMonitors()
{
    try {
        Reference<XPresentationSupplier> xPS ( mxModel, UNO_QUERY_THROW);
        Reference<XPresentation2> xPresentation(xPS->getPresentation(), UNO_QUERY_THROW);

        // Get the existing presenter console screen, we want to switch the
        // presentation to use that instead.
        sal_Int32 nNewScreen = GetPresenterScreenNumber (xPresentation);
        if (nNewScreen < 0)
            return;

        // Adapt that display number to be the 'default' setting of 0 if it matches
        sal_Int32 nExternalDisplay = Application::GetDisplayExternalScreen();

        if (nNewScreen == nExternalDisplay)
            nNewScreen = 0; // screen zero is best == the primary display
        else
            nNewScreen++; // otherwise we store screens offset by one.

        // Set the new presentation display
        Reference<beans::XPropertySet> xProperties (xPresentation, UNO_QUERY_THROW);
        xProperties->setPropertyValue("Display", Any(nNewScreen));
    } catch (const uno::Exception &) {
    }
}

/**
 * Return the real VCL screen number to show the presenter console
 * on or -1 to not show anything.
 */
sal_Int32 PresenterScreen::GetPresenterScreenNumber (
    const Reference<presentation::XPresentation2>& rxPresentation) const
{
    sal_Int32 nScreenNumber (0);
    sal_Int32 nScreenCount (1);
    try
    {
        Reference<beans::XPropertySet> xProperties (rxPresentation, UNO_QUERY);
        if ( ! xProperties.is())
            return -1;

        // Determine the screen on which the full screen presentation is being
        // displayed.
        sal_Int32 nDisplayNumber (-1);
        if ( ! (xProperties->getPropertyValue("Display") >>= nDisplayNumber))
            return -1;
        if (nDisplayNumber == -1)
        {
            // The special value -1 indicates that the slide show
            // spans all available displays.  That leaves no room for
            // the presenter screen.
            return -1;
        }

        SAL_INFO("sdext.presenter", "Display number is " << nDisplayNumber);

        if (nDisplayNumber > 0)
        {
            nScreenNumber = nDisplayNumber - 1;
        }
        else if (nDisplayNumber == 0)
        {
            // A display number value of 0 indicates the primary screen.
            // Find out which screen number that is.
            if (nDisplayNumber <= 0)
                nScreenNumber = Application::GetDisplayExternalScreen();
        }

        // We still have to determine the number of screens to decide
        // whether the presenter screen may be shown at all.
        nScreenCount = Application::GetScreenCount();

        if (nScreenCount < 2 || nDisplayNumber > nScreenCount)
        {
            // There is either only one screen or the full screen
            // presentation spans all available screens.  The presenter
            // screen is shown only when a special flag in the configuration
            // is set.
            Reference<XComponentContext> xContext (mxContextWeak);
            PresenterConfigurationAccess aConfiguration (
                xContext,
                OUString("/org.openoffice.Office.PresenterScreen/"),
                PresenterConfigurationAccess::READ_ONLY);
            bool bStartAlways (false);
            if (aConfiguration.GetConfigurationNode(
                "Presenter/StartAlways") >>= bStartAlways)
            {
                if (bStartAlways)
                    return GetPresenterScreenFromScreen(nScreenNumber);
            }
            return -1;
        }
    }
    catch (const beans::UnknownPropertyException&)
    {
        OSL_ASSERT(false);
        // For some reason we can not access the screen number.  Use
        // the default instead.
    }
    SAL_INFO("sdext.presenter", "Get presenter screen for screen " << nScreenNumber);
    return GetPresenterScreenFromScreen(nScreenNumber);
}

sal_Int32 PresenterScreen::GetPresenterScreenFromScreen( sal_Int32 nPresentationScreen )
{
    // Setup the resource id of the full screen background pane so that
    // it is displayed on another screen than the presentation.
    sal_Int32 nPresenterScreenNumber (1);
    switch (nPresentationScreen)
    {
        case 0:
            nPresenterScreenNumber = 1;
            break;

        case 1:
            nPresenterScreenNumber = 0;
            break;

        default:
            SAL_INFO("sdext.presenter", "Warning unexpected, out of bound screen "
                     "mapped to 0" << nPresentationScreen);
            // When the full screen presentation is displayed on a screen
            // other than 0 or 1 then place the presenter on the first
            // available screen.
            nPresenterScreenNumber = 0;
            break;
    }
    return nPresenterScreenNumber;
}

Reference<drawing::framework::XResourceId> PresenterScreen::GetMainPaneId (
    const Reference<presentation::XPresentation2>& rxPresentation) const
{
    // A negative value means that the presentation spans all available
    // displays.  That leaves no room for the presenter.
    const sal_Int32 nScreen(GetPresenterScreenNumber(rxPresentation));
    if (nScreen < 0)
        return nullptr;

    return ResourceId::create(
        Reference<XComponentContext>(mxContextWeak),
        PresenterHelper::msFullScreenPaneURL
                + "?FullScreen=true&ScreenNumber="
                + OUString::number(nScreen));
}

void PresenterScreen::RequestShutdownPresenterScreen()
{
    // Restore the configuration that was active before the presenter screen
    // has been activated.  Now, that the presenter screen is displayed in
    // its own top level window this probably not necessary, but one never knows.
    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is() && mxSavedConfiguration.is())
    {
        xCC->restoreConfiguration(mxSavedConfiguration);
        mxSavedConfiguration = nullptr;
    }

    if (xCC.is())
    {
        // The actual restoration of the configuration takes place
        // asynchronously.  The view and pane factories can only by disposed
        // after that.  Therefore, set up a listener and wait for the
        // restoration.
        rtl::Reference<PresenterScreen> pSelf (this);
        PresenterFrameworkObserver::RunOnUpdateEnd(
            xCC,
            ::boost::bind(&PresenterScreen::ShutdownPresenterScreen, pSelf));
        xCC->update();
    }
}

void PresenterScreen::ShutdownPresenterScreen()
{
    Reference<lang::XComponent> xViewFactoryComponent (mxViewFactory, UNO_QUERY);
    if (xViewFactoryComponent.is())
        xViewFactoryComponent->dispose();
    mxViewFactory = nullptr;

    Reference<lang::XComponent> xPaneFactoryComponent (mxPaneFactory, UNO_QUERY);
    if (xPaneFactoryComponent.is())
        xPaneFactoryComponent->dispose();
    mxPaneFactory = nullptr;

    if (mpPresenterController.get() != nullptr)
    {
        mpPresenterController->dispose();
        mpPresenterController.clear();
    }
    mpPaneContainer = new PresenterPaneContainer(Reference<XComponentContext>(mxContextWeak));
}

void PresenterScreen::SetupPaneFactory (const Reference<XComponentContext>& rxContext)
{
    try
    {
        if ( ! mxPaneFactory.is())
            mxPaneFactory = PresenterPaneFactory::Create(
                rxContext,
                mxController,
                mpPresenterController);
    }
    catch (const RuntimeException&)
    {
        OSL_ASSERT(false);
    }
}

void PresenterScreen::SetupViewFactory (const Reference<XComponentContext>& rxContext)
{
    try
    {
        if ( ! mxViewFactory.is())
            mxViewFactory = PresenterViewFactory::Create(
                rxContext,
                mxController,
                mpPresenterController);
    }
    catch (const RuntimeException&)
    {
        OSL_ASSERT(false);
    }
}

void PresenterScreen::SetupConfiguration (
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxAnchorId)
{
    try
    {
        PresenterConfigurationAccess aConfiguration (
            rxContext,
            OUString("org.openoffice.Office.PresenterScreen"),
            PresenterConfigurationAccess::READ_ONLY);
        maViewDescriptors.clear();
        ProcessViewDescriptions(aConfiguration);
        OUString sLayoutName ("DefaultLayout");
        aConfiguration.GetConfigurationNode(
            "Presenter/CurrentLayout") >>= sLayoutName;
        ProcessLayout(aConfiguration, sLayoutName, rxContext, rxAnchorId);
    }
    catch (const RuntimeException&)
    {
    }
}

void PresenterScreen::ProcessLayout (
    PresenterConfigurationAccess& rConfiguration,
    const OUString& rsLayoutName,
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxAnchorId)
{
    try
    {
        Reference<container::XHierarchicalNameAccess> xLayoutNode (
            rConfiguration.GetConfigurationNode(
                "Presenter/Layouts/"+rsLayoutName),
            UNO_QUERY_THROW);

        // Read the parent layout first, if one is referenced.
        OUString sParentLayout;
        PresenterConfigurationAccess::GetConfigurationNode(
            xLayoutNode,
            "ParentLayout") >>= sParentLayout;
        if (!sParentLayout.isEmpty())
        {
            // Prevent infinite recursion.
            if (rsLayoutName != sParentLayout)
                ProcessLayout(rConfiguration, sParentLayout, rxContext, rxAnchorId);
        }

        // Process the actual layout list.
        Reference<container::XNameAccess> xList (
            PresenterConfigurationAccess::GetConfigurationNode(
                xLayoutNode,
                "Layout"),
            UNO_QUERY_THROW);

        ::std::vector<OUString> aProperties (6);
        aProperties[0] = "PaneURL";
        aProperties[1] = "ViewURL";
        aProperties[2] = "RelativeX";
        aProperties[3] = "RelativeY";
        aProperties[4] = "RelativeWidth";
        aProperties[5] = "RelativeHeight";
        mnComponentIndex = 1;
        PresenterConfigurationAccess::ForAll(
            xList,
            aProperties,
            ::boost::bind(&PresenterScreen::ProcessComponent, this,
                _1,
                _2,
                rxContext,
                rxAnchorId));
    }
    catch (const RuntimeException&)
    {
    }
}

void PresenterScreen::ProcessViewDescriptions (
    PresenterConfigurationAccess& rConfiguration)
{
    try
    {
        Reference<container::XNameAccess> xViewDescriptionsNode (
            rConfiguration.GetConfigurationNode("Presenter/Views"),
            UNO_QUERY_THROW);

        ::std::vector<OUString> aProperties (4);
        aProperties[0] = "ViewURL";
        aProperties[1] = "Title";
        aProperties[2] = "AccessibleTitle";
        aProperties[3] = "IsOpaque";
        mnComponentIndex = 1;
        PresenterConfigurationAccess::ForAll(
            xViewDescriptionsNode,
            aProperties,
            ::boost::bind(&PresenterScreen::ProcessViewDescription, this, _1, _2));
    }
    catch (const RuntimeException&)
    {
        OSL_ASSERT(false);
    }
}

void PresenterScreen::ProcessComponent (
    const OUString& rsKey,
    const ::std::vector<Any>& rValues,
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxAnchorId)
{
    (void)rsKey;

    if (rValues.size() != 6)
        return;

    try
    {
        OUString sPaneURL;
        OUString sViewURL;
        double nX = 0;
        double nY = 0;
        double nWidth = 0;
        double nHeight = 0;
        rValues[0] >>= sPaneURL;
        rValues[1] >>= sViewURL;
        rValues[2] >>= nX;
        rValues[3] >>= nY;
        rValues[4] >>= nWidth;
        rValues[5] >>= nHeight;

        if (nX>=0 && nY>=0 && nWidth>0 && nHeight>0)
        {
            SetupView(
                rxContext,
                rxAnchorId,
                sPaneURL,
                sViewURL,
                PresenterPaneContainer::ViewInitializationFunction(),
                nX,
                nY,
                nX+nWidth,
                nY+nHeight);
        }
       }
    catch (const Exception&)
    {
        OSL_ASSERT(false);
    }
}

void PresenterScreen::ProcessViewDescription (
    const OUString& rsKey,
    const ::std::vector<Any>& rValues)
{
    (void)rsKey;

    if (rValues.size() != 4)
        return;

    try
    {
        ViewDescriptor aViewDescriptor;
        OUString sViewURL;
        rValues[0] >>= sViewURL;
        rValues[1] >>= aViewDescriptor.msTitle;
        rValues[2] >>= aViewDescriptor.msAccessibleTitle;
        rValues[3] >>= aViewDescriptor.mbIsOpaque;
        if (aViewDescriptor.msAccessibleTitle.isEmpty())
            aViewDescriptor.msAccessibleTitle = aViewDescriptor.msTitle;
        maViewDescriptors[sViewURL] = aViewDescriptor;
    }
    catch (const Exception&)
    {
        OSL_ASSERT(false);
    }
}

void PresenterScreen::SetupView(
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxAnchorId,
    const OUString& rsPaneURL,
    const OUString& rsViewURL,
    const PresenterPaneContainer::ViewInitializationFunction& rViewInitialization,
    const double nLeft,
    const double nTop,
    const double nRight,
    const double nBottom)
{
    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is())
    {
        Reference<XResourceId> xPaneId (ResourceId::createWithAnchor(rxContext,rsPaneURL,rxAnchorId));
        // Look up the view descriptor.
        ViewDescriptor aViewDescriptor;
        ViewDescriptorContainer::const_iterator iDescriptor (maViewDescriptors.find(rsViewURL));
        if (iDescriptor != maViewDescriptors.end())
            aViewDescriptor = iDescriptor->second;

        // Prepare the pane.
        OSL_ASSERT(mpPaneContainer.get() != nullptr);
        mpPaneContainer->PreparePane(
            xPaneId,
            rsViewURL,
            aViewDescriptor.msTitle,
            aViewDescriptor.msAccessibleTitle,
            aViewDescriptor.mbIsOpaque,
            rViewInitialization,
            nLeft,
            nTop,
            nRight,
            nBottom);
    }
}

} } // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
