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
#if defined(_MSC_VER) && (_MSC_VER > 1310)
#pragma warning(disable : 4917 4555)
#endif

#ifdef __MINGW32__
#define INITGUID
#endif
#include "servprov.hxx"
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <rtl/ustring.h>
#include <cppuhelper/factory.hxx>


using namespace ::com::sun::star;


uno::Reference<uno::XInterface> SAL_CALL EmbedServer_createInstance(
    const uno::Reference<lang::XMultiServiceFactory> & xSMgr)
throw (uno::Exception)
{
    uno::Reference<uno::XInterface > xService = *new EmbedServer_Impl( xSMgr );
    return xService;
}

::rtl::OUString SAL_CALL EmbedServer_getImplementationName() throw()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.ole.EmbedServer") );

}

uno::Sequence< ::rtl::OUString > SAL_CALL EmbedServer_getSupportedServiceNames() throw()
{
    uno::Sequence< ::rtl::OUString > aServiceNames( 1 );
    aServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.OleEmbeddedServerRegistration" ) );
    return aServiceNames;
}

extern "C" {

SAL_DLLPUBLIC_EXPORT void * SAL_CALL emser_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;

    ::rtl::OUString aImplName( ::rtl::OUString::createFromAscii( pImplName ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    if(pServiceManager && aImplName.equals( EmbedServer_getImplementationName() ) )
    {
        xFactory= ::cppu::createOneInstanceFactory( reinterpret_cast< lang::XMultiServiceFactory*>(pServiceManager),
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

// Fix strange warnings about some
// ATL::CAxHostWindow::QueryInterface|AddRef|Releae functions.
// warning C4505: 'xxx' : unreferenced local function has been removed
#if defined(_MSC_VER)
#pragma warning(disable: 4505)
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
