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
#ifdef _MSC_VER
#pragma warning(disable : 4917 4555)
#endif

#include "servprov.hxx"
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <rtl/ustring.h>
#include <cppuhelper/factory.hxx>


using namespace ::com::sun::star;

/// @throws uno::Exception
uno::Reference<uno::XInterface> SAL_CALL EmbedServer_createInstance(
    const uno::Reference<lang::XMultiServiceFactory> & xSMgr)
{
    uno::Reference<uno::XInterface > xService = *new EmbedServer_Impl( xSMgr );
    return xService;
}

OUString SAL_CALL EmbedServer_getImplementationName() throw()
{
    return OUString("com.sun.star.comp.ole.EmbedServer");

}

uno::Sequence< OUString > SAL_CALL EmbedServer_getSupportedServiceNames() throw()
{
    uno::Sequence<OUString> aServiceNames { "com.sun.star.document.OleEmbeddedServerRegistration" };
    return aServiceNames;
}

extern "C" {

SAL_DLLPUBLIC_EXPORT void * SAL_CALL emser_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = nullptr;

    OUString aImplName( OUString::createFromAscii( pImplName ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    if(pServiceManager && aImplName.equals( EmbedServer_getImplementationName() ) )
    {
        xFactory= ::cppu::createOneInstanceFactory( static_cast< lang::XMultiServiceFactory*>(pServiceManager),
                                            EmbedServer_getImplementationName(),
                                            EmbedServer_createInstance,
                                            EmbedServer_getSupportedServiceNames() );
    }

    if (xFactory.is())
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
