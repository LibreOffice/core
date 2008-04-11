/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterProtocolHandler.cxx,v $
 *
 * $Revision: 1.3 $
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

#include "PresenterProtocolHandler.hxx"
#include "PresenterController.hxx"
#include "PresenterHelper.hxx"
#include "PresenterPaneFactory.hxx"
#include "PresenterViewFactory.hxx"
#include "PresenterWindowManager.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterConfigurationAccess.hxx"
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/drawing/SlideSorter.hpp>
#include <com/sun/star/drawing/framework/Configuration.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>
#include <com/sun/star/presentation/XSlideShow.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <boost/bind.hpp>
#include <tools/debug.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

namespace {
    const static OUString gsProtocol (
        OUString::createFromAscii("vnd.com.sun.star.comp.PresenterScreen:"));
    const static OUString gsAboutCommand (
        OUString::createFromAscii("about"));
}



//----- Service ---------------------------------------------------------------

OUString PresenterProtocolHandler::getImplementationName_static (void)
{
    return OUString::createFromAscii("vnd.sun.star.sdext.presenter.PresenterProtocolHandler");
}




Sequence<OUString> PresenterProtocolHandler::getSupportedServiceNames_static (void)
{
    static const ::rtl::OUString sServiceName(
        ::rtl::OUString::createFromAscii("com.sun.star.frame.ProtocolHandler"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




Reference<XInterface> PresenterProtocolHandler::Create (
    const Reference<uno::XComponentContext>& rxContext)
    SAL_THROW((Exception))
{
    return Reference<XInterface>(static_cast<XWeak*>(new PresenterProtocolHandler(rxContext)));
}




//===== PresenterProtocolHandler =========================================================


PresenterProtocolHandler::PresenterProtocolHandler (const Reference<XComponentContext>& rxContext)
    : PresenterProtocolHandlerInterfaceBase(m_aMutex)
{
    (void)rxContext;
}




PresenterProtocolHandler::~PresenterProtocolHandler (void)
{
}




void SAL_CALL PresenterProtocolHandler::disposing (void)
{
}




//----- XInitialize -----------------------------------------------------------

void SAL_CALL PresenterProtocolHandler::initialize (const Sequence<Any>& aArguments)
    throw (Exception, RuntimeException)
{
    ThrowIfDisposed();
    if (aArguments.getLength() > 0)
    {
        try
        {
        }
        catch (RuntimeException&)
        {
            OSL_ASSERT(false);
        }
    }
}




//----- XDispatchProvider -----------------------------------------------------

Reference<frame::XDispatch > SAL_CALL PresenterProtocolHandler::queryDispatch (
    const css::util::URL& rURL,
    const rtl::OUString& rsTargetFrameName,
    sal_Int32 nSearchFlags)
    throw(RuntimeException)
{
    (void)rsTargetFrameName;
    (void)nSearchFlags;
    ThrowIfDisposed();

    if (rURL.Protocol == gsProtocol)
        return this;
    else
        return NULL;
}




Sequence<Reference<frame::XDispatch> > SAL_CALL PresenterProtocolHandler::queryDispatches(
    const Sequence<frame::DispatchDescriptor>& rDescriptors)
    throw(RuntimeException)
{
    (void)rDescriptors;
    ThrowIfDisposed();
    return Sequence<Reference<frame::XDispatch> >();
}




//----- XDispatch -------------------------------------------------------------

void SAL_CALL PresenterProtocolHandler::dispatch(
    const css::util::URL& rURL,
    const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
    throw(css::uno::RuntimeException)
{
    (void)rArguments;
    ThrowIfDisposed();

    if (rURL.Protocol == gsProtocol)
    {
        if (rURL.Path == gsAboutCommand)
        {
            //          AboutDialog aAboutDialog (mxMSF, mxFrame);
            //          aAboutDialog.execute();
        }
    }
}




void SAL_CALL PresenterProtocolHandler::addStatusListener(
    const css::uno::Reference<css::frame::XStatusListener>& rxListener,
    const css::util::URL& rURL)
    throw(css::uno::RuntimeException)
{
    (void)rxListener;
    (void)rURL;
}




void SAL_CALL PresenterProtocolHandler::removeStatusListener (
    const css::uno::Reference<css::frame::XStatusListener>& rxListener,
    const css::util::URL& rURL)
    throw(css::uno::RuntimeException)
{
    (void)rxListener;
    (void)rURL;
}




//-----------------------------------------------------------------------------

void PresenterProtocolHandler::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterProtocolHandler object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}



} } // end of namespace ::sdext::presenter
