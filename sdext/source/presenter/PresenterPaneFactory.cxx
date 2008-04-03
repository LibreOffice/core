/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterPaneFactory.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:02:33 $
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

#include "PresenterPaneFactory.hxx"
#include "PresenterController.hxx"
#include "PresenterPane.hxx"
#include "PresenterPaneBorderPainter.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterSpritePane.hxx"
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

const ::rtl::OUString PresenterPaneFactory::msCurrentSlidePreviewPaneURL(
    OUString::createFromAscii("private:resource/pane/Presenter/Pane1"));
const ::rtl::OUString PresenterPaneFactory::msNextSlidePreviewPaneURL(
    OUString::createFromAscii("private:resource/pane/Presenter/Pane2"));
const ::rtl::OUString PresenterPaneFactory::msNotesPaneURL(
    OUString::createFromAscii("private:resource/pane/Presenter/Pane3"));
const ::rtl::OUString PresenterPaneFactory::msToolBarPaneURL(
    OUString::createFromAscii("private:resource/pane/Presenter/Pane4"));
const ::rtl::OUString PresenterPaneFactory::msSlideSorterPaneURL(
    OUString::createFromAscii("private:resource/pane/Presenter/Pane5"));
const ::rtl::OUString PresenterPaneFactory::msClockPaneURL(
    OUString::createFromAscii("private:resource/pane/Presenter/Pane6"));

const ::rtl::OUString PresenterPaneFactory::msDebugPaneURL(
    OUString::createFromAscii("private:resource/pane/Presenter/Debug"));

const ::rtl::OUString PresenterPaneFactory::msOverlayPaneURL(
    OUString::createFromAscii("private:resource/pane/Presenter/Overlay"));



//===== PresenterPaneFactory ==================================================

Reference<drawing::framework::XResourceFactory> PresenterPaneFactory::Create (
    const Reference<uno::XComponentContext>& rxContext,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
{
    rtl::Reference<PresenterPaneFactory> pFactory (
        new PresenterPaneFactory(rxContext,rpPresenterController));
    pFactory->Register(rxController);
    return Reference<drawing::framework::XResourceFactory>(
        static_cast<XWeak*>(pFactory.get()), UNO_QUERY);
}




PresenterPaneFactory::PresenterPaneFactory (
    const Reference<uno::XComponentContext>& rxContext,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterPaneFactoryInterfaceBase(m_aMutex),
      mxComponentContextWeak(rxContext),
      mxConfigurationControllerWeak(),
      mpPresenterController(rpPresenterController)
{
}




void PresenterPaneFactory::Register (const Reference<frame::XController>& rxController)
{
    Reference<XConfigurationController> xCC;
    try
    {
        // Get the configuration controller.
        Reference<XControllerManager> xCM (rxController, UNO_QUERY_THROW);
        xCC = Reference<XConfigurationController>(xCM->getConfigurationController());
        mxConfigurationControllerWeak = xCC;
        if ( ! xCC.is())
        {
            throw RuntimeException();
        }
        else
        {
            xCC->addResourceFactory(
                OUString::createFromAscii("private:resource/pane/Presenter/*"),
                this);
        }
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
        if (xCC.is())
            xCC->removeResourceFactoryForReference(this);
        mxConfigurationControllerWeak = WeakReference<XConfigurationController>();

        throw;
    }
}




PresenterPaneFactory::~PresenterPaneFactory (void)
{
}




void SAL_CALL PresenterPaneFactory::disposing (void)
    throw (RuntimeException)
{
    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is())
        xCC->removeResourceFactoryForReference(this);
        mxConfigurationControllerWeak = WeakReference<XConfigurationController>();
}




//----- XPaneFactory ----------------------------------------------------------

Reference<XResource> SAL_CALL PresenterPaneFactory::createResource (
    const Reference<XResourceId>& rxPaneId)
    throw (RuntimeException)
{
    return CreatePane(rxPaneId, OUString());
}




void SAL_CALL PresenterPaneFactory::releaseResource (const Reference<XResource>& rxResource)
    throw (RuntimeException)
{
    if ( ! rxResource.is())
        throw lang::IllegalArgumentException();

    // Remove the pane from the container.
    Reference<XPane> rxPane (rxResource, UNO_QUERY_THROW);
    rtl::Reference<PresenterPaneContainer> pPaneContainer(
        mpPresenterController->GetPaneContainer());
    pPaneContainer->RemovePane(rxPane->getResourceId());

    // Dispose the pane (together with the content window.)
    Reference<lang::XComponent> xPaneComponent (rxPane, UNO_QUERY);
    if (xPaneComponent.is())
        xPaneComponent->dispose();
}




//-----------------------------------------------------------------------------

Reference<XResource> PresenterPaneFactory::CreatePane (
    const Reference<XResourceId>& rxPaneId,
    const OUString& rsTitle)
{
    (void)rsTitle;

    if ( ! rxPaneId.is())
        return NULL;

    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if ( ! xCC.is())
        return NULL;

    Reference<XComponentContext> xContext (mxComponentContextWeak);
    if ( ! xContext.is())
        return NULL;

    Reference<XPane> xParentPane (xCC->getResource(rxPaneId->getAnchor()), UNO_QUERY);
    if ( ! xParentPane.is())
        return NULL;

    try
    {
        return CreatePane(
            rxPaneId,
            rsTitle,
            xParentPane,
            rxPaneId->getFullResourceURL().Arguments.compareToAscii("Sprite=1") == 0);
    }
    catch (Exception&)
    {
        OSL_ASSERT(false);
    }

    return NULL;
}




Reference<XResource> PresenterPaneFactory::CreatePane (
    const Reference<XResourceId>& rxPaneId,
    const OUString& rsTitle,
    const Reference<drawing::framework::XPane>& rxParentPane,
    const bool bIsSpritePane)
{
    Reference<XComponentContext> xContext (mxComponentContextWeak);
    Reference<lang::XMultiComponentFactory> xFactory (
        xContext->getServiceManager(), UNO_QUERY_THROW);

    // Create a border window and canvas and store it in the pane
    // container.

    // Create the pane.
    ::rtl::Reference<PresenterPaneBase> xPane;
    if (bIsSpritePane)
        xPane = ::rtl::Reference<PresenterPaneBase>(new PresenterSpritePane(xContext));
    else
        xPane = ::rtl::Reference<PresenterPaneBase>(new PresenterPane(xContext));

    // Supply arguments.
    Sequence<Any> aArguments (6);
    aArguments[0] <<= rxPaneId;
    aArguments[1] <<= rxParentPane->getWindow();
    aArguments[2] <<= rxParentPane->getCanvas();
    aArguments[3] <<= rsTitle;
    aArguments[4] <<= Reference<drawing::framework::XPaneBorderPainter>(
        static_cast<XWeak*>(mpPresenterController->GetPaneBorderPainter().get()),
        UNO_QUERY);
    aArguments[5] <<= bIsSpritePane ? false : true;
    xPane->initialize(aArguments);

    // Get the window of the frame and make that visible.
    if ( ! bIsSpritePane)
    {
        Reference<awt::XWindow> xWindow (xPane->getWindow(), UNO_QUERY_THROW);
        xWindow->setVisible(sal_True);
    }

    // Store pane and canvases and windows in container.
    ::rtl::Reference<PresenterPaneContainer> pContainer (
        mpPresenterController->GetPaneContainer());
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor(
        pContainer->StoreBorderWindow(rxPaneId, xPane->GetBorderWindow()));
    pContainer->StorePane(xPane);
    if (pDescriptor.get() != NULL)
        if (bIsSpritePane)
        {
            pDescriptor->maSpriteProvider = ::boost::bind(
                &PresenterSpritePane::GetSprite,
                dynamic_cast<PresenterSpritePane*>(xPane.get()));
            pDescriptor->mbIsSprite = true;
            pDescriptor->mbNeedsClipping = false;
        }
        else
        {
            pDescriptor->mbIsSprite = false;
            pDescriptor->mbNeedsClipping = true;
        }

    return Reference<XResource>(static_cast<XWeak*>(xPane.get()), UNO_QUERY_THROW);
}




void PresenterPaneFactory::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterPaneFactory object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}


} } // end of namespace sdext::presenter
