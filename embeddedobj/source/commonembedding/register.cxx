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

#include "xfactory.hxx"
#include "xcreator.hxx"

using namespace ::com::sun::star;


extern "C" {

SAL_DLLPUBLIC_EXPORT void * SAL_CALL embobj_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager,
    SAL_UNUSED_PARAMETER void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    ::rtl::OUString aImplName( ::rtl::OUString::createFromAscii( pImplName ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    if ( pServiceManager )
    {
        if ( aImplName.equals( OOoEmbeddedObjectFactory::impl_staticGetImplementationName() ) )
        {
            xFactory= ::cppu::createOneInstanceFactory( reinterpret_cast< lang::XMultiServiceFactory*>( pServiceManager ),
                                                OOoEmbeddedObjectFactory::impl_staticGetImplementationName(),
                                                OOoEmbeddedObjectFactory::impl_staticCreateSelfInstance,
                                                OOoEmbeddedObjectFactory::impl_staticGetSupportedServiceNames() );
        }
        else if ( aImplName.equals( OOoSpecialEmbeddedObjectFactory::impl_staticGetImplementationName() ) )
        {
            xFactory= ::cppu::createOneInstanceFactory( reinterpret_cast< lang::XMultiServiceFactory*>( pServiceManager ),
                                                OOoSpecialEmbeddedObjectFactory::impl_staticGetImplementationName(),
                                                OOoSpecialEmbeddedObjectFactory::impl_staticCreateSelfInstance,
                                                OOoSpecialEmbeddedObjectFactory::impl_staticGetSupportedServiceNames() );
        }
        else if ( aImplName.equals( UNOEmbeddedObjectCreator::impl_staticGetImplementationName() ) )
        {
            xFactory= ::cppu::createOneInstanceFactory( reinterpret_cast< lang::XMultiServiceFactory*>( pServiceManager ),
                                                UNOEmbeddedObjectCreator::impl_staticGetImplementationName(),
                                                UNOEmbeddedObjectCreator::impl_staticCreateSelfInstance,
                                                UNOEmbeddedObjectCreator::impl_staticGetSupportedServiceNames() );
        }
    }

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
