/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "services.h"
#include "dispatch/dispatchdisabler.hxx"

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>

using namespace css;
using namespace framework;

DispatchDisabler::DispatchDisabler(const uno::Reference< uno::XComponentContext >& rxContext) :
    mxContext( rxContext )
{
}

// XInitialization
void SAL_CALL DispatchDisabler::initialize( const uno::Sequence< uno::Any >& aArguments )
        throw (uno::Exception, uno::RuntimeException, ::std::exception)
{
    uno::Sequence< OUString > aDisabledURLs;
    if( aArguments.getLength() > 0 &&
        ( aArguments[0] >>= aDisabledURLs ) )
    {
        for( sal_Int32 i = 0; i < aDisabledURLs.getLength(); ++i )
            maDisabledURLs.insert(aDisabledURLs[i]);
    }
}

// XDispatchProvider
uno::Reference< frame::XDispatch > SAL_CALL
DispatchDisabler::queryDispatch( const util::URL& rURL,
                                 const OUString& rTargetFrameName,
                                 ::sal_Int32 nSearchFlags )
    throw (uno::RuntimeException, ::std::exception)
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
    throw (uno::RuntimeException, ::std::exception)
{
    uno::Sequence< uno::Reference< frame::XDispatch > > aResult(rRequests.getLength());
    for( sal_Int32 i = 0; i < rRequests.getLength(); ++i )
        aResult[i] = queryDispatch(rRequests[i].FeatureURL,
                                   rRequests[i].FrameName,
                                   rRequests[i].SearchFlags);
    return aResult;
}

// XDispatchProviderInterceptor
uno::Reference< frame::XDispatchProvider > SAL_CALL
DispatchDisabler::getSlaveDispatchProvider() throw (uno::RuntimeException, ::std::exception)
{
    return mxSlave;
}

void SAL_CALL DispatchDisabler::setSlaveDispatchProvider( const uno::Reference< frame::XDispatchProvider >& xNewDispatchProvider )
    throw (uno::RuntimeException, ::std::exception)
{
    mxSlave = xNewDispatchProvider;
}

uno::Reference< frame::XDispatchProvider > SAL_CALL
DispatchDisabler::getMasterDispatchProvider() throw (uno::RuntimeException, ::std::exception)
{
    return mxMaster;
}
void SAL_CALL
DispatchDisabler::setMasterDispatchProvider( const uno::Reference< frame::XDispatchProvider >& xNewSupplier )
        throw (uno::RuntimeException, ::std::exception)
{
    mxMaster = xNewSupplier;
}

// XInterceptorInfo
uno::Sequence< OUString > SAL_CALL
    DispatchDisabler::getInterceptedURLs()
    throw (uno::RuntimeException, ::std::exception)
{
    uno::Sequence< OUString > aDisabledURLs(maDisabledURLs.size());
    sal_Int32 n = 0;
    for (auto i = maDisabledURLs.begin(); i != maDisabledURLs.end(); ++i)
        aDisabledURLs[n++] = *i;
    return aDisabledURLs;
}

// XElementAccess
uno::Type SAL_CALL DispatchDisabler::getElementType()
    throw (uno::RuntimeException, ::std::exception)
{
    uno::Type aModuleType = cppu::UnoType<OUString>::get();
    return aModuleType;
}

::sal_Bool SAL_CALL DispatchDisabler::hasElements()
        throw (uno::RuntimeException, ::std::exception)
{
    return maDisabledURLs.size() > 0;
}

// XNameAccess
uno::Any SAL_CALL DispatchDisabler::getByName( const OUString& )
        throw (container::NoSuchElementException, lang::WrappedTargetException,
               uno::RuntimeException, ::std::exception)
{
    return uno::Any();
}

uno::Sequence< OUString > SAL_CALL DispatchDisabler::getElementNames()
        throw (uno::RuntimeException, ::std::exception)
{
    return getInterceptedURLs();
}

sal_Bool SAL_CALL DispatchDisabler::hasByName( const OUString& rName )
        throw (uno::RuntimeException, ::std::exception)
{
    return maDisabledURLs.find(rName) != maDisabledURLs.end();
}

// XNameReplace
void SAL_CALL DispatchDisabler::replaceByName( const OUString& rName, const uno::Any& aElement )
        throw (lang::IllegalArgumentException, container::NoSuchElementException,
               lang::WrappedTargetException, uno::RuntimeException, ::std::exception)
{
    removeByName( rName );
    insertByName( rName, aElement );
}

// XNameContainer
void DispatchDisabler::insertByName( const OUString& rName, const uno::Any& )
    throw (lang::IllegalArgumentException, container::ElementExistException,
           lang::WrappedTargetException, uno::RuntimeException, ::std::exception)
{
    maDisabledURLs.insert(rName);
}

void DispatchDisabler::removeByName( const OUString& rName )
    throw (container::NoSuchElementException, lang::WrappedTargetException,
           uno::RuntimeException, ::std::exception)
{
    auto it = maDisabledURLs.find(rName);
    if( it != maDisabledURLs.end() )
        maDisabledURLs.erase(it);
}

DEFINE_INIT_SERVICE(DispatchDisabler, {})

// XServiceInfo
DEFINE_XSERVICEINFO_MULTISERVICE_2(DispatchDisabler,
                                   ::cppu::OWeakObject,
                                   "com.sun.star.frame.DispatchDisabler",
                                   IMPLEMENTATIONNAME_DISPATCHDISABLER)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
