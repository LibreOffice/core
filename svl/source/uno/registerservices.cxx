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

#include "sal/types.h"
#include "rtl/ustring.hxx"
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <svl/svldllapi.h>

using css::uno::Reference;
using css::uno::Sequence;

// -------------------------------------------------------------------------------------

#define DECLARE_CREATEINSTANCE( ImplName ) \
    Reference< css::uno::XInterface > SAL_CALL ImplName##_CreateInstance( const Reference< css::lang::XMultiServiceFactory >& );

DECLARE_CREATEINSTANCE( SvNumberFormatterServiceObj )
DECLARE_CREATEINSTANCE( SvNumberFormatsSupplierServiceObject )
DECLARE_CREATEINSTANCE( PathService )

// -------------------------------------------------------------------------------------

extern "C"
{

SVL_DLLPUBLIC void* SAL_CALL svl_component_getFactory (
    const sal_Char * pImplementationName, void * _pServiceManager,
    SAL_UNUSED_PARAMETER void * /* _pRegistryKey*/)
{
    void * pResult = 0;
    if ( _pServiceManager )
    {
        Reference< css::lang::XSingleServiceFactory > xFactory;
        if (rtl_str_compare(
                pImplementationName,
                "com.sun.star.uno.util.numbers.SvNumberFormatsSupplierServiceObject") == 0)
        {
            Sequence< OUString > aServiceNames(1);
            aServiceNames.getArray()[0] =
                OUString("com.sun.star.util.NumberFormatsSupplier");

            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< css::lang::XMultiServiceFactory* >(_pServiceManager),
                OUString::createFromAscii( pImplementationName ),
                SvNumberFormatsSupplierServiceObject_CreateInstance,
                aServiceNames);
        }
        else if (rtl_str_compare(
                     pImplementationName,
                     "com.sun.star.uno.util.numbers.SvNumberFormatterServiceObject") == 0)
        {
            Sequence< OUString > aServiceNames(1);
            aServiceNames.getArray()[0] =
                OUString("com.sun.star.util.NumberFormatter");

            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< css::lang::XMultiServiceFactory* >(_pServiceManager),
                OUString::createFromAscii( pImplementationName ),
                SvNumberFormatterServiceObj_CreateInstance,
                aServiceNames);
        }
        else if (rtl_str_compare (
                     pImplementationName, "com.sun.star.comp.svl.PathService") == 0)
        {
            Sequence< OUString > aServiceNames(1);
            aServiceNames.getArray()[0] =
                OUString("com.sun.star.config.SpecialConfigManager");
            xFactory = ::cppu::createSingleFactory (
                reinterpret_cast< css::lang::XMultiServiceFactory* >( _pServiceManager ),
                OUString::createFromAscii( pImplementationName ),
                PathService_CreateInstance,
                aServiceNames);
        }
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pResult = xFactory.get();
        }
    }
    return pResult;
}

}   // "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
