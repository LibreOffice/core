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

#include <cppuhelper/implementationentry.hxx>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

namespace cppu {

sal_Bool component_writeInfoHelper(
    SAL_UNUSED_PARAMETER void *, void * pRegistryKey,
    const struct ImplementationEntry entries[])
{
    bool bRet = false;
    try
    {
        if( pRegistryKey )
        {
            for( sal_Int32 i = 0; entries[i].create ; i ++ )
            {
                OUString sKey = "/" + entries[i].getImplementationName() + "/UNO/SERVICES";
                Reference< XRegistryKey > xNewKey(
                    static_cast< XRegistryKey * >( pRegistryKey )->createKey( sKey ) );

                Sequence< OUString > seq = entries[i].getSupportedServiceNames();
                const OUString *pArray = seq.getConstArray();
                for ( sal_Int32 nPos = 0 ; nPos < seq.getLength(); nPos ++ )
                    xNewKey->createKey( pArray[nPos] );
            }
            bRet = true;
        }
    }
    catch ( InvalidRegistryException & )
    {
        OSL_FAIL( "### InvalidRegistryException!" );
    }
    return bRet;
}


void * component_getFactoryHelper(
    sal_Char const * pImplName, SAL_UNUSED_PARAMETER void *,
    SAL_UNUSED_PARAMETER void *, const struct ImplementationEntry entries[])
{

    void * pRet = nullptr;
    Reference< XSingleComponentFactory > xFactory;

    for( sal_Int32 i = 0 ; entries[i].create ; i ++ )
    {
        OUString implName = entries[i].getImplementationName();
        if( implName.equalsAscii( pImplName ) )
        {
            xFactory = entries[i].createFactory(
                entries[i].create,
                implName,
                entries[i].getSupportedServiceNames(),
                entries[i].moduleCounter );
        }
    }

    if( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
