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

#include "hatchwindowfactory.hxx"
#include "hatchwindow.hxx"
#include "cppuhelper/factory.hxx"
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>

#include "documentcloser.hxx"

using namespace ::com::sun::star;

uno::Sequence< OUString > SAL_CALL OHatchWindowFactory::impl_staticGetSupportedServiceNames()
{
    uno::Sequence< OUString > aRet(2);
    aRet[0] = "com.sun.star.embed.HatchWindowFactory";
    aRet[1] = "com.sun.star.comp.embed.HatchWindowFactory";
    return aRet;
}

OUString SAL_CALL OHatchWindowFactory::impl_staticGetImplementationName()
{
    return OUString( "com.sun.star.comp.embed.HatchWindowFactory" );
}

uno::Reference< uno::XInterface > SAL_CALL OHatchWindowFactory::impl_staticCreateSelfInstance(
            const uno::Reference< lang::XMultiServiceFactory >& xServiceManager )
{
    return uno::Reference< uno::XInterface >( *new OHatchWindowFactory( xServiceManager ) );
}

uno::Reference< embed::XHatchWindow > SAL_CALL OHatchWindowFactory::createHatchWindowInstance(
                const uno::Reference< awt::XWindowPeer >& xParent,
                const awt::Rectangle& aBounds,
                const awt::Size& aHandlerSize )
    throw (uno::RuntimeException)
{
    if ( !xParent.is() )
        throw lang::IllegalArgumentException(); // TODO

    SolarMutexGuard aGuard;
    VCLXHatchWindow* pResult = new VCLXHatchWindow();
    pResult->initializeWindow( xParent, aBounds, aHandlerSize );
    return uno::Reference< embed::XHatchWindow >( static_cast< embed::XHatchWindow* >( pResult ) );
}

OUString SAL_CALL OHatchWindowFactory::getImplementationName()
    throw ( uno::RuntimeException )
{
    return impl_staticGetImplementationName();
}

sal_Bool SAL_CALL OHatchWindowFactory::supportsService( const OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL OHatchWindowFactory::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return impl_staticGetSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
