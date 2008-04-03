/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterScreen.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:03:16 $
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

#include "PresenterScreen.hxx"
#include "PresenterController.hxx"
#include "PresenterHelper.hxx"
#include "PresenterPaneFactory.hxx"
#include "PresenterViewFactory.hxx"
#include "PresenterWindowManager.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterConfigurationAccess.hxx"
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/SlideSorter.hpp>
#include <com/sun/star/drawing/framework/Configuration.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>
#include <com/sun/star/presentation/XSlideShow.hpp>
#include <com/sun/star/presentation/XPresentation2.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <boost/bind.hpp>
#include <tools/debug.hxx>

#include <com/sun/star/view/XSelectionSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

//----- Service ---------------------------------------------------------------

OUString PresenterScreen::getImplementationName_static (void)
{
    return OUString::createFromAscii("com.sun.star.comp.Draw.framework.PresenterScreen");
}




Sequence<OUString> PresenterScreen::getSupportedServiceNames_static (void)
{
    static const ::rtl::OUString sServiceName(
        ::rtl::OUString::createFromAscii("com.sun.star.drawing.framework.PresenterScreen"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




Reference<XInterface> PresenterScreen::Create (const Reference<uno::XComponentContext>& rxContext)
    SAL_THROW((css::uno::Exception))
{
    return Reference<XInterface>(static_cast<XWeak*>(new PresenterScreen(rxContext)));
}




//===== PresenterScreen =========================================================


PresenterScreen::PresenterScreen (const Reference<XComponentContext>& rxContext)
    : PresenterScreenInterfaceBase(m_aMutex),
      mxModel(),
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
      mxViewFactory()
{
}




PresenterScreen::~PresenterScreen (void)
{
}




void SAL_CALL PresenterScreen::disposing (void)
{
    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is())
    {
        if (mxSavedConfiguration.is())
            xCC->restoreConfiguration(mxSavedConfiguration);

        mxConfigurationControllerWeak = Reference<XConfigurationController>(NULL);
    }

    Reference<lang::XComponent> xViewFactoryComponent (mxViewFactory, UNO_QUERY);
    if (xViewFactoryComponent.is())
        xViewFactoryComponent->dispose();
    Reference<lang::XComponent> xPaneFactoryComponent (mxPaneFactory, UNO_QUERY);
    if (xPaneFactoryComponent.is())
        xPaneFactoryComponent->dispose();
}




//----- XJob -----------------------------------------------------------

Any SAL_CALL PresenterScreen::execute(
    const Sequence< beans::NamedValue >& Arguments )
    throw (lang::IllegalArgumentException, Exception, RuntimeException)
{
    Sequence< beans::NamedValue > lEnv;

    sal_Int32               i = 0;
    sal_Int32               c = Arguments.getLength();
    const beans::NamedValue* p = Arguments.getConstArray();
    for (i=0; i<c; ++i)
    {
        if (p[i].Name.equalsAscii("Environment"))
        {
            p[i].Value >>= lEnv;
            break;
        }
    }

    c = lEnv.getLength();
    p = lEnv.getConstArray();
    for (i=0; i<c; ++i)
    {
        if (p[i].Name.equalsAscii("Model"))
        {
            p[i].Value >>= mxModel;
            break;
        }
    }

    Reference< XServiceInfo > xInfo( mxModel, UNO_QUERY );
    if( xInfo.is() && xInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PresentationDocument" ) ) ) )
    {
        Reference< document::XEventListener > xDocListener(static_cast< document::XEventListener* >(this), UNO_QUERY);
        Reference< document::XEventBroadcaster > xDocBroadcaster( mxModel, UNO_QUERY );
        if( xDocBroadcaster.is() )
            xDocBroadcaster->addEventListener(xDocListener);
    }

    return Any();
}




//----- document::XEventListener ----------------------------------------------

void SAL_CALL PresenterScreen::notifyEvent( const css::document::EventObject& Event ) throw (css::uno::RuntimeException)
{
    if( Event.EventName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "OnStartPresentation" ) ) )
    {
        InitializePresenterScreen();
    }
    else if( Event.EventName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "OnEndPresentation" ) ) )
    {
        ShutdownPresenterScreen();
    }
}




//----- XEventListener --------------------------------------------------------

void SAL_CALL PresenterScreen::disposing (const lang::EventObject& /*rEvent*/)
    throw (RuntimeException)
{
    mxSlideShowControllerWeak = WeakReference<presentation::XSlideShowController>();
    ShutdownPresenterScreen();
}




//-----------------------------------------------------------------------------

void PresenterScreen::InitializePresenterScreen (void)
{
    try
    {
        Reference<XComponentContext> xContext (mxContextWeak);
        Reference<drawing::framework::XResourceId> xMainPaneId(
            ResourceId::create(
                xContext,
                PresenterHelper::msFullScreenPaneURL
                    +OUString::createFromAscii("?FullScreen=false&ScreenNumber=1")));
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
                // pane is actived additionally to the already existing
                // panes and does not replace them.
                xCC->requestResourceActivation(
                    xMainPaneId,
                    ResourceActivationMode_ADD);
                SetupConfiguration(xContext, xMainPaneId);

                mpPresenterController = new PresenterController(
                    xContext,
                    mxController,
                    xSlideShowController,
                    mpPaneContainer,
                    xMainPaneId);

                // Create pane and view factories and integrate them into the
                // drawing framework.
                SetupPaneFactory(xContext);
                SetupViewFactory(xContext);
            }
            catch (RuntimeException&)
            {
                xCC->restoreConfiguration(mxSavedConfiguration);
            }
            xCC->unlock();
        }
    }
    catch (Exception&)
    {
    }
}




void PresenterScreen::DeactivatePanes (const Reference<XConfigurationController>& rxCC)
{
    OSL_ASSERT(rxCC.is());

    Reference<XComponentContext> xContext (mxContextWeak);
    if ( ! xContext.is())
        return;
    Reference<XResourceId> xCenterPaneId(ResourceId::create(
          xContext,
          PresenterHelper::msCenterPaneURL));
    if ( ! xCenterPaneId.is())
        return;

    Reference<XConfiguration> xCurrentConfiguration (rxCC->getCurrentConfiguration());
    if (xCurrentConfiguration.is())
    {
        // First explicitly deactivate the view in the center pane.  This
        // view is used as marker by some other modules and it is important
        // that its deactivation is requested first.
        const Sequence<Reference<XResourceId> > aViews (xCurrentConfiguration->getResources(
            xCenterPaneId,
            PresenterHelper::msViewURLPrefix,
            AnchorBindingMode_DIRECT));
        for (sal_Int32 nIndex=0; nIndex<aViews.getLength(); ++nIndex)
            rxCC->requestResourceDeactivation(aViews[nIndex]);

        // Now deactivate all top level panes and and all resources anchored
        // to them.
        const Sequence<Reference<XResourceId> > aPanes (xCurrentConfiguration->getResources(
            xCenterPaneId,
            PresenterHelper::msPaneURLPrefix,
            AnchorBindingMode_DIRECT));
        for (sal_Int32 nIndex=0; nIndex<aPanes.getLength(); ++nIndex)
            rxCC->requestResourceDeactivation(aPanes[nIndex]);
    }
}




void PresenterScreen::ShutdownPresenterScreen (void)
{
    if (mpPresenterController.get() != NULL)
    {
        mpPresenterController->dispose();
        mpPresenterController = rtl::Reference<PresenterController>();
    }

    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is() && mxSavedConfiguration.is())
    {
        xCC->restoreConfiguration(mxSavedConfiguration);
    }
    mpPaneContainer = new PresenterPaneContainer(Reference<XComponentContext>(mxContextWeak));

    Reference<lang::XComponent> xViewFactoryComponent (mxViewFactory, UNO_QUERY);
    if (xViewFactoryComponent.is())
        xViewFactoryComponent->dispose();
    mxViewFactory = NULL;

    Reference<lang::XComponent> xPaneFactoryComponent (mxPaneFactory, UNO_QUERY);
    if (xPaneFactoryComponent.is())
        xPaneFactoryComponent->dispose();
    mxPaneFactory = NULL;

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
    catch (RuntimeException&)
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
    catch (RuntimeException&)
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
            OUString::createFromAscii("org.openoffice.Office.extension.PresenterScreen"),
            PresenterConfigurationAccess::READ_ONLY);
        OUString sLayoutName (OUString::createFromAscii("DefaultLayout"));
        aConfiguration.GetConfigurationNode(
            OUString::createFromAscii("Presenter/CurrentLayout")) >>= sLayoutName;
        ProcessLayout(aConfiguration, sLayoutName, rxContext, rxAnchorId);
    }
    catch (RuntimeException&)
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
                OUString::createFromAscii("Presenter/Layouts/")+rsLayoutName),
            UNO_QUERY_THROW);

        // Read the parent layout first, if one is referenced.
        OUString sParentLayout;
        rConfiguration.GetConfigurationNode(
            xLayoutNode,
            OUString::createFromAscii("ParentLayout")) >>= sParentLayout;
        if (sParentLayout.getLength() > 0)
        {
            // Prevent infinite recursion.
            if (rsLayoutName != sParentLayout)
                ProcessLayout(rConfiguration, sParentLayout, rxContext, rxAnchorId);
        }

        // Process the actual layout list.
        Reference<container::XNameAccess> xList (
            rConfiguration.GetConfigurationNode(
                xLayoutNode,
                OUString::createFromAscii("Layout")),
            UNO_QUERY_THROW);

        ::std::vector<rtl::OUString> aProperties (7);
        aProperties[0] = OUString::createFromAscii("PaneURL");
        aProperties[1] = OUString::createFromAscii("ViewURL");
        aProperties[2] = OUString::createFromAscii("Title");
        aProperties[3] = OUString::createFromAscii("RelativeX");
        aProperties[4] = OUString::createFromAscii("RelativeY");
        aProperties[5] = OUString::createFromAscii("RelativeWidth");
        aProperties[6] = OUString::createFromAscii("RelativeHeight");
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
    catch (RuntimeException&)
    {
    }
}




void PresenterScreen::ProcessComponent (
    const OUString& rsKey,
    const ::std::vector<Any>& rValues,
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxAnchorId)
{
    (void)rsKey;

    if (rValues.size() != 7)
        return;

    try
    {
        OUString sPaneURL;
        OUString sViewURL;
        OUString sTitle;
        double nX = 0;
        double nY = 0;
        double nWidth = 0;
        double nHeight = 0;
        rValues[0] >>= sPaneURL;
        rValues[1] >>= sViewURL;
        rValues[2] >>= sTitle;
        rValues[3] >>= nX;
        rValues[4] >>= nY;
        rValues[5] >>= nWidth;
        rValues[6] >>= nHeight;

        if (nX>=0 && nY>=0 && nWidth>0 && nHeight>0)
        {
            SetupView(
                rxContext,
                rxAnchorId,
                sPaneURL,
                sViewURL,
                sTitle,
                PresenterPaneContainer::ViewInitializationFunction(),
                nX,
                nY,
                nX+nWidth,
                nY+nHeight);
        }
       }
    catch (Exception& e)
    {
        (void)e;
        OSL_ASSERT(false);
    }
}




Reference<XResourceId> PresenterScreen::SetupView(
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxAnchorId,
    const OUString& rsPaneURL,
    const OUString& rsViewURL,
    const OUString& rsTitle,
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
        OSL_ASSERT(mpPaneContainer.get() != NULL);
        mpPaneContainer->PreparePane(
            xPaneId,
            rsTitle,
            rViewInitialization,
            nLeft,
            nTop,
            nRight,
            nBottom);

        Reference<XResourceId> xViewId (ResourceId::createWithAnchor(rxContext,rsViewURL,xPaneId));
        if (rsViewURL.getLength() > 0)
        {
            xCC->requestResourceActivation(
                xPaneId,
                ResourceActivationMode_ADD);

            xCC->requestResourceActivation(
                xViewId,
                ResourceActivationMode_REPLACE);
        }

        return xViewId;
    }
    else
        return Reference<XResourceId>();
}




void PresenterScreen::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterScreen object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}



} } // end of namespace ::sdext::presenter
