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

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <cppuhelper/factory.hxx>

#include "xexecutor.hxx"

using namespace ::com::sun::star;


extern "C" {

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    ::rtl::OUString aImplName( ::rtl::OUString::createFromAscii( pImplName ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    if ( pServiceManager && aImplName.equals( UNOMainThreadExecutor::impl_staticGetImplementationName() ) )
    {
        xFactory= ::cppu::createOneInstanceFactory( reinterpret_cast< lang::XMultiServiceFactory*>( pServiceManager ),
                                            UNOMainThreadExecutor::impl_staticGetImplementationName(),
                                            UNOMainThreadExecutor::impl_staticCreateSelfInstance,
                                            UNOMainThreadExecutor::impl_staticGetSupportedServiceNames() );
    }

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

sal_Bool SAL_CALL component_writeInfo( void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            uno::Reference< registry::XRegistryKey > xKey( reinterpret_cast< registry::XRegistryKey* >( pRegistryKey ) );

            uno::Reference< registry::XRegistryKey >  xNewKey;

            xNewKey = xKey->createKey( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) +
                                        UNOMainThreadExecutor::impl_staticGetImplementationName() +
                                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );

            uno::Sequence< ::rtl::OUString > &rServices = UNOMainThreadExecutor::impl_staticGetSupportedServiceNames();
            for( sal_Int32 ind = 0; ind < rServices.getLength(); ind++ )
                xNewKey->createKey( rServices.getConstArray()[ind] );

            return sal_True;
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_FAIL( "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
