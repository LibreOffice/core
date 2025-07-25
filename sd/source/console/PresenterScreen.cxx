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
#include "PresenterPaneContainer.hxx"
#include "PresenterPaneFactory.hxx"
#include "PresenterViewFactory.hxx"
#include "PresenterWindowManager.hxx"
#include <DrawController.hxx>
#include <ResourceId.hxx>
#include <framework/ConfigurationController.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/presentation/XPresentation2.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <utility>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::drawing::framework;

constexpr OUString FULL_SCREEN_PANE_URL = u"private:resource/pane/FullScreenPane"_ustr;

namespace sdext::presenter {

namespace {
    typedef ::cppu::WeakComponentImplHelper <
        css::document::XEventListener
        > PresenterScreenListenerInterfaceBase;

    /** One instance of a PresenterScreenListener is registered per Impress
        document and waits for the full screen slide show to start and to
        end.
    */
    class PresenterScreenListener
        : private ::cppu::BaseMutex,
          public PresenterScreenListenerInterfaceBase
    {
    public:
        PresenterScreenListener (
            css::uno::Reference<css::uno::XComponentContext> xContext,
            css::uno::Reference<css::frame::XModel2> xModel);
        PresenterScreenListener(const PresenterScreenListener&) = delete;
        PresenterScreenListener& operator=(const PresenterScreenListener&) = delete;

        void Initialize();
        virtual void SAL_CALL disposing() override;

        // document::XEventListener

        virtual void SAL_CALL notifyEvent( const css::document::EventObject& Event ) override;

        // XEventListener

        virtual void SAL_CALL disposing ( const css::lang::EventObject& rEvent) override;

    private:
        css::uno::Reference<css::frame::XModel2 > mxModel;
        css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
        rtl::Reference<PresenterScreen> mpPresenterScreen;
    };
}

//----- XServiceInfo ---------------------------------------------------------------

Sequence< OUString > SAL_CALL PresenterScreenJob::getSupportedServiceNames()
{
    return {  };
}

OUString SAL_CALL PresenterScreenJob::getImplementationName()
{
    return u"org.libreoffice.comp.PresenterScreenJob"_ustr;
}

sal_Bool SAL_CALL PresenterScreenJob::supportsService(const OUString& aServiceName)
{
    return cppu::supportsService(this, aServiceName);
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
sd_PresenterScreenJob_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new PresenterScreenJob(context));
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
{
    Sequence< beans::NamedValue > lEnv;
    auto pArg = std::find_if(Arguments.begin(), Arguments.end(),
        [](const beans::NamedValue& rArg) { return rArg.Name == "Environment"; });
    if (pArg != Arguments.end())
        pArg->Value >>= lEnv;

    Reference<frame::XModel2> xModel;
    auto pProp = std::find_if(std::cbegin(lEnv), std::cend(lEnv),
        [](const beans::NamedValue& rProp) { return rProp.Name == "Model"; });
    if (pProp != std::cend(lEnv))
        pProp->Value >>= xModel;

    Reference< XServiceInfo > xInfo( xModel, UNO_QUERY );
    if( xInfo.is() && xInfo->supportsService(u"com.sun.star.presentation.PresentationDocument"_ustr) )
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
    css::uno::Reference<css::uno::XComponentContext> xContext,
    css::uno::Reference<css::frame::XModel2> xModel)
    : PresenterScreenListenerInterfaceBase(m_aMutex),
      mxModel(std::move(xModel)),
      mxComponentContext(std::move(xContext))
{
}

void PresenterScreenListener::Initialize()
{
    Reference< document::XEventListener > xDocListener(this);
    Reference< document::XEventBroadcaster > xDocBroadcaster( mxModel, UNO_QUERY );
    if( xDocBroadcaster.is() )
        xDocBroadcaster->addEventListener(xDocListener);
}

void SAL_CALL PresenterScreenListener::disposing()
{
    Reference< document::XEventBroadcaster > xDocBroadcaster( mxModel, UNO_QUERY );
    if( xDocBroadcaster.is() )
        xDocBroadcaster->removeEventListener(
            Reference<document::XEventListener>(this) );

    if (mpPresenterScreen.is())
    {
        mpPresenterScreen->RequestShutdownPresenterScreen();
        mpPresenterScreen = nullptr;
    }
}

// document::XEventListener

void SAL_CALL PresenterScreenListener::notifyEvent( const css::document::EventObject& Event )
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            u"PresenterScreenListener object has already been disposed"_ustr,
            static_cast<uno::XWeak*>(this));
    }

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
    else if ( Event.EventName == "ShapeModified" )
    {
        if (mpPresenterScreen.is())
        {
            Reference<drawing::XShape> xShape(Event.Source, UNO_QUERY);

            if (xShape.is())
            {
                // when presenter is used and shape changes, check
                // and evtl. trigger update of 'NextSlide' view
                mpPresenterScreen->CheckNextSlideUpdate(xShape);
            }
        }
    }
}

// XEventListener

void SAL_CALL PresenterScreenListener::disposing (const css::lang::EventObject&)
{
    if (mpPresenterScreen.is())
    {
        mpPresenterScreen->RequestShutdownPresenterScreen();
        mpPresenterScreen = nullptr;
    }
}

} // end of anonymous namespace

//===== PresenterScreen =======================================================

PresenterScreen::PresenterScreen (
    const Reference<XComponentContext>& rxContext,
    css::uno::Reference<css::frame::XModel2> xModel)
: PresenterScreenInterfaceBase(m_aMutex)
, mxModel(std::move(xModel))
, mxController()
, mxContextWeak(rxContext)
, mpPresenterController()
, mxSavedConfiguration()
, mpPaneContainer()
, mxPaneFactory()
, mxViewFactory()
, mbIsInitialized(false)
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
            u"/org.openoffice.Office.Impress/"_ustr,
            PresenterConfigurationAccess::READ_ONLY);
        aConfiguration.GetConfigurationNode(u"Misc/Start/EnablePresenterScreen"_ustr)
            >>= dEnablePresenterScreen;
        return dEnablePresenterScreen;
}

bool PresenterScreen::isPresenterScreenFullScreen(const css::uno::Reference<css::uno::XComponentContext>& rxContext)
{
    bool dPresenterScreenFullScreen = true;
    PresenterConfigurationAccess aConfiguration (
        rxContext,
        u"/org.openoffice.Office.Impress/"_ustr,
        PresenterConfigurationAccess::READ_ONLY);
    aConfiguration.GetConfigurationNode(u"Misc/Start/PresenterScreenFullScreen"_ustr)
        >>= dPresenterScreenFullScreen;
    return dPresenterScreenFullScreen;
}

void SAL_CALL PresenterScreen::disposing()
{
    rtl::Reference<::sd::framework::ConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is() && mxSavedConfiguration.is())
    {
        xCC->restoreConfiguration(mxSavedConfiguration);
    }
    mxConfigurationControllerWeak.clear();

    if (mxViewFactory.is())
       mxViewFactory->dispose();
    if (mxPaneFactory.is())
        mxPaneFactory->dispose();

    mxModel = nullptr;
}

//----- XEventListener --------------------------------------------------------

void SAL_CALL PresenterScreen::disposing (const lang::EventObject& /*rEvent*/)
{
    RequestShutdownPresenterScreen();
}


void PresenterScreen::InitializePresenterScreen()
{
    // IASS: already initialized (may even assert here?)
    if (mbIsInitialized)
        return;

    try
    {
        mpPaneContainer = new PresenterPaneContainer();

        Reference<XPresentationSupplier> xPS ( mxModel, UNO_QUERY_THROW);
        Reference<XPresentation2> xPresentation(xPS->getPresentation(), UNO_QUERY_THROW);
        Reference<presentation::XSlideShowController> xSlideShowController( xPresentation->getController() );

        if( !xSlideShowController.is() || !xSlideShowController->isFullScreen() )
            return;

        // find first controller that is not the current controller (the one with the slideshow
        auto tmpController = mxModel->getCurrentController();
        Reference< container::XEnumeration > xEnum( mxModel->getControllers() );
        if( xEnum.is() )
        {
            while( xEnum->hasMoreElements() )
            {
                Reference< frame::XController > xC( xEnum->nextElement(), UNO_QUERY );
                if( xC.is() && (xC.get() != tmpController.get()) )
                {
                    mxController = dynamic_cast<::sd::DrawController*>(xC.get());
                    assert(bool(mxController) == bool(xC) && "only support instances of type DrawController");
                    break;
                }
            }
        }
        // Get the XController from the first argument.

        rtl::Reference<::sd::framework::ConfigurationController> xCC( mxController->getConfigurationController());
        mxConfigurationControllerWeak = xCC.get();

        Reference<XComponentContext> xContext(mxContextWeak);
        rtl::Reference<sd::framework::ResourceId> xMainPaneId(
            GetMainPaneId(xPresentation, xContext));
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
                    sd::framework::ResourceActivationMode::ADD);
                SetupConfiguration(xContext, xMainPaneId);

                mpPresenterController = new PresenterController(
                    this,
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

    // IASS: Remember we are initialized
    mbIsInitialized = true;
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
        xProperties->setPropertyValue(u"Display"_ustr, Any(nNewScreen));
    } catch (const uno::Exception &) {
    }
}

void PresenterScreen::CheckNextSlideUpdate(const Reference<drawing::XShape>& rxShape)
{
    if (nullptr == mpPresenterController)
        return;

    // forward to PresenterController if used
    mpPresenterController->CheckNextSlideUpdate(rxShape);
}

/**
 * Return the real VCL screen number to show the presenter console
 * on or -1 to not show anything.
 */
sal_Int32 PresenterScreen::GetPresenterScreenNumber (
    const Reference<presentation::XPresentation2>& rxPresentation) const
{
    sal_Int32 nScreenNumber (0);
    try
    {
        if ( ! rxPresentation.is())
            return -1;

        // Determine the screen on which the full screen presentation is being
        // displayed.
        sal_Int32 nDisplayNumber (-1);
        if ( ! (rxPresentation->getPropertyValue(u"Display"_ustr) >>= nDisplayNumber))
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
            nScreenNumber = Application::GetDisplayExternalScreen();
        }

        // We still have to determine the number of screens to decide
        // whether the presenter screen may be shown at all.
        sal_Int32 nScreenCount = Application::GetScreenCount();

        if (nScreenCount < 2 || nDisplayNumber > nScreenCount)
        {
            // There is either only one screen or the full screen
            // presentation spans all available screens.  The presenter
            // screen is shown only when a special flag in the configuration
            // is set or when the presenter screen will be shown as
            // non-full screen window
            Reference<XComponentContext> xContext (mxContextWeak);
            PresenterConfigurationAccess aConfiguration (
                xContext,
                u"/org.openoffice.Office.PresenterScreen/"_ustr,
                PresenterConfigurationAccess::READ_ONLY);
            bool bStartAlways (false);
            bool bPresenterScreenFullScreen = isPresenterScreenFullScreen(xContext);
            if (aConfiguration.GetConfigurationNode(
                u"Presenter/StartAlways"_ustr) >>= bStartAlways)
            {
                if (bStartAlways || !bPresenterScreenFullScreen)
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

rtl::Reference<sd::framework::ResourceId> PresenterScreen::GetMainPaneId (
    const Reference<presentation::XPresentation2>& rxPresentation,
    const Reference<XComponentContext>& xContext) const
{
    // A negative value means that the presentation spans all available
    // displays.  That leaves no room for the presenter.
    const sal_Int32 nScreen(GetPresenterScreenNumber(rxPresentation));
    if (nScreen < 0)
        return nullptr;

    auto fullScreenStr = isPresenterScreenFullScreen(xContext)
        ? u"true"_ustr
        : u"false"_ustr;

    return new sd::framework::ResourceId(
        FULL_SCREEN_PANE_URL
                + "?FullScreen="
                + fullScreenStr
                + "&ScreenNumber="
                + OUString::number(nScreen));
}

void PresenterScreen::RequestShutdownPresenterScreen()
{
    // IASS: only cleanup when we are initialized
    if (!mbIsInitialized)
        return;

    // Restore the configuration that was active before the presenter screen
    // has been activated.  Now, that the presenter screen is displayed in
    // its own top level window this probably not necessary, but one never knows.
    rtl::Reference<::sd::framework::ConfigurationController> xCC (mxConfigurationControllerWeak);
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
        rtl::Reference<PresenterScreen> xSelf(this);
        PresenterFrameworkObserver::RunOnUpdateEnd(
            xCC,
            [xSelf=std::move(xSelf)](bool){ return xSelf->ShutdownPresenterScreen(); });
        xCC->update();
    }

    // IASS: reset to non-initialized
    mbIsInitialized = false;
}

void PresenterScreen::ShutdownPresenterScreen()
{
    if (mxViewFactory.is())
        mxViewFactory->dispose();
    mxViewFactory = nullptr;

    if (mxPaneFactory.is())
        mxPaneFactory->dispose();
    mxPaneFactory = nullptr;

    if (mpPresenterController)
    {
        mpPresenterController->dispose();
        mpPresenterController.clear();
    }
    mpPaneContainer = new PresenterPaneContainer();
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
    const rtl::Reference<sd::framework::ResourceId>& rxAnchorId)
{
    try
    {
        PresenterConfigurationAccess aConfiguration (
            rxContext,
            u"org.openoffice.Office.PresenterScreen"_ustr,
            PresenterConfigurationAccess::READ_ONLY);
        maViewDescriptors.clear();
        ProcessViewDescriptions(aConfiguration);
        OUString sLayoutName (u"DefaultLayout"_ustr);
        aConfiguration.GetConfigurationNode(
            u"Presenter/CurrentLayout"_ustr) >>= sLayoutName;
        ProcessLayout(aConfiguration, sLayoutName, rxContext, rxAnchorId);
    }
    catch (const RuntimeException&)
    {
    }
}

void PresenterScreen::ProcessLayout (
    PresenterConfigurationAccess& rConfiguration,
    std::u16string_view rsLayoutName,
    const Reference<XComponentContext>& rxContext,
    const rtl::Reference<sd::framework::ResourceId>& rxAnchorId)
{
    try
    {
        Reference<container::XHierarchicalNameAccess> xLayoutNode (
            rConfiguration.GetConfigurationNode(
                OUString::Concat("Presenter/Layouts/")+rsLayoutName),
            UNO_QUERY_THROW);

        // Read the parent layout first, if one is referenced.
        OUString sParentLayout;
        PresenterConfigurationAccess::GetConfigurationNode(
            xLayoutNode,
            u"ParentLayout"_ustr) >>= sParentLayout;
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
                u"Layout"_ustr),
            UNO_QUERY_THROW);

        ::std::vector<OUString> aProperties
        {
            u"PaneURL"_ustr,
            u"ViewURL"_ustr,
            u"RelativeX"_ustr,
            u"RelativeY"_ustr,
            u"RelativeWidth"_ustr,
            u"RelativeHeight"_ustr
        };
        PresenterConfigurationAccess::ForAll(
            xList,
            aProperties,
            [this, rxAnchorId](std::vector<uno::Any> const& rArgs)
            {
                this->ProcessComponent(rArgs, rxAnchorId);
            });
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
            rConfiguration.GetConfigurationNode(u"Presenter/Views"_ustr),
            UNO_QUERY_THROW);

        ::std::vector<OUString> aProperties
        {
            u"ViewURL"_ustr,
            u"Title"_ustr,
            u"AccessibleTitle"_ustr,
            u"IsOpaque"_ustr
        };
        PresenterConfigurationAccess::ForAll(
            xViewDescriptionsNode,
            aProperties,
            [this](std::vector<uno::Any> const& rArgs)
            {
                return this->ProcessViewDescription(rArgs);
            });
    }
    catch (const RuntimeException&)
    {
        OSL_ASSERT(false);
    }
}

void PresenterScreen::ProcessComponent (
    const ::std::vector<Any>& rValues,
    const rtl::Reference<sd::framework::ResourceId>& rxAnchorId)
{
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
                rxAnchorId,
                sPaneURL,
                sViewURL,
                PresenterPaneContainer::ViewInitializationFunction());
        }
       }
    catch (const Exception&)
    {
        OSL_ASSERT(false);
    }
}

void PresenterScreen::ProcessViewDescription (
    const ::std::vector<Any>& rValues)
{
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
        maViewDescriptors[sViewURL] = std::move(aViewDescriptor);
    }
    catch (const Exception&)
    {
        OSL_ASSERT(false);
    }
}

void PresenterScreen::SetupView(
    const rtl::Reference<sd::framework::ResourceId>& rxAnchorId,
    const OUString& rsPaneURL,
    const OUString& rsViewURL,
    const PresenterPaneContainer::ViewInitializationFunction& rViewInitialization)
{
    rtl::Reference<::sd::framework::ConfigurationController> xCC (mxConfigurationControllerWeak);
    if (!xCC.is())
        return;

    rtl::Reference<sd::framework::ResourceId> xPaneId (new sd::framework::ResourceId(rsPaneURL,rxAnchorId));
    // Look up the view descriptor.
    ViewDescriptor aViewDescriptor;
    ViewDescriptorContainer::const_iterator iDescriptor (maViewDescriptors.find(rsViewURL));
    if (iDescriptor != maViewDescriptors.end())
        aViewDescriptor = iDescriptor->second;

    // Prepare the pane.
    assert(mpPaneContainer);
    mpPaneContainer->PreparePane(
        xPaneId,
        rsViewURL,
        aViewDescriptor.msTitle,
        aViewDescriptor.msAccessibleTitle,
        aViewDescriptor.mbIsOpaque,
        rViewInitialization);
}

} // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
