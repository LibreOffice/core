/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include <cppuhelper/factory.hxx>

#include <documentdigitalsignatures.hxx>
#include <certificatecontainer.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

extern "C"
{
SAL_DLLPUBLIC_EXPORT void* SAL_CALL xmlsecurity_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void* pRet = 0;
    uno::Reference< XInterface > xFactory;

    
    OUString implName = OUString::createFromAscii( pImplName );

    if ( pServiceManager && implName.equals( DocumentDigitalSignatures::GetImplementationName() ) )
    {
        
        xFactory = cppu::createSingleComponentFactory(
            DocumentDigitalSignatures_CreateInstance,
            OUString::createFromAscii( pImplName ),
            DocumentDigitalSignatures::GetSupportedServiceNames() );
    }
    else if ( pServiceManager && implName.equals( CertificateContainer::impl_getStaticImplementationName() ))
    {
        
        xFactory = cppu::createOneInstanceFactory(
            reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            CertificateContainer::impl_createInstance,
            CertificateContainer::impl_getStaticSupportedServiceNames() );
    }

     if (xFactory.is())
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

}   





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
