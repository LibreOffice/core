/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <services.h>
#include <dispatch/dispatchdisabler.hxx>

#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <cppuhelper/supportsservice.hxx>

using namespace css;
using namespace framework;

DispatchDisabler::DispatchDisabler(const uno::Reference< uno::XComponentContext >& )
{
}

// XInitialization
void SAL_CALL DispatchDisabler::initialize( const uno::Sequence< uno::Any >& aArguments )
{
    uno::Sequence< OUString > aDisabledURLs;
    if( aArguments.hasElements() &&
        ( aArguments[0] >>= aDisabledURLs ) )
    {
        for (OUString const& url : aDisabledURLs)
            maDisabledURLs.insert(url);
    }
}

// XDispatchProvider
uno::Reference< frame::XDispatch > SAL_CALL
DispatchDisabler::queryDispatch( const util::URL& rURL,
                                 const OUString& rTargetFrameName,
                                 ::sal_Int32 nSearchFlags )
{
    // If present - disabled.
    if( maDisabledURLs.find(rURL.Complete) != maDisabledURLs.end() ||
        !mxSlave.is() )
        return uno::Reference< frame::XDispatch >();
    else
        return mxSlave->queryDispatch(rURL, rTargetFrameName, nSearchFlags);
}

uno::Sequence< uno::Reference< frame::XDispatch > > SAL_CALL
DispatchDisabler::queryDispatches( const uno::Sequence< frame::DispatchDescriptor >& rRequests )
{
    uno::Sequence< uno::Reference< frame::XDispatch > > aResult(rRequests.getLength());
    auto aResultRange = asNonConstRange(aResult);
    for( sal_Int32 i = 0; i < rRequests.getLength(); ++i )
        aResultRange[i] = queryDispatch(rRequests[i].FeatureURL,
                                   rRequests[i].FrameName,
                                   rRequests[i].SearchFlags);
    return aResult;
}

// XDispatchProviderInterceptor
uno::Reference< frame::XDispatchProvider > SAL_CALL
DispatchDisabler::getSlaveDispatchProvider()
{
    return mxSlave;
}

void SAL_CALL DispatchDisabler::setSlaveDispatchProvider( const uno::Reference< frame::XDispatchProvider >& xNewDispatchProvider )
{
    mxSlave = xNewDispatchProvider;
}

uno::Reference< frame::XDispatchProvider > SAL_CALL
DispatchDisabler::getMasterDispatchProvider()
{
    return mxMaster;
}
void SAL_CALL
DispatchDisabler::setMasterDispatchProvider( const uno::Reference< frame::XDispatchProvider >& xNewSupplier )
{
    mxMaster = xNewSupplier;
}

// XInterceptorInfo
uno::Sequence< OUString > SAL_CALL
    DispatchDisabler::getInterceptedURLs()
{
    uno::Sequence< OUString > aDisabledURLs(maDisabledURLs.size());
    auto aDisabledURLsRange = asNonConstRange(aDisabledURLs);
    sal_Int32 n = 0;
    for (auto const& disabledURL : maDisabledURLs)
        aDisabledURLsRange[n++] = disabledURL;
    return aDisabledURLs;
}

// XElementAccess
uno::Type SAL_CALL DispatchDisabler::getElementType()
{
    uno::Type aModuleType = cppu::UnoType<OUString>::get();
    return aModuleType;
}

::sal_Bool SAL_CALL DispatchDisabler::hasElements()
{
    return !maDisabledURLs.empty();
}

// XNameAccess
uno::Any SAL_CALL DispatchDisabler::getByName( const OUString& )
{
    return uno::Any();
}

uno::Sequence< OUString > SAL_CALL DispatchDisabler::getElementNames()
{
    return getInterceptedURLs();
}

sal_Bool SAL_CALL DispatchDisabler::hasByName( const OUString& rName )
{
    return maDisabledURLs.find(rName) != maDisabledURLs.end();
}

// XNameReplace
void SAL_CALL DispatchDisabler::replaceByName( const OUString& rName, const uno::Any& aElement )
{
    removeByName( rName );
    insertByName( rName, aElement );
}

// XNameContainer
void DispatchDisabler::insertByName( const OUString& rName, const uno::Any& )
{
    maDisabledURLs.insert(rName);
}

void DispatchDisabler::removeByName( const OUString& rName )
{
    auto it = maDisabledURLs.find(rName);
    if( it != maDisabledURLs.end() )
        maDisabledURLs.erase(it);
}

// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL DispatchDisabler::getImplementationName()
{
    return u"com.sun.star.comp.framework.services.DispatchDisabler"_ustr;
}

sal_Bool SAL_CALL DispatchDisabler::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL DispatchDisabler::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.DispatchDisabler"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_DispatchDisabler_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::DispatchDisabler(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
