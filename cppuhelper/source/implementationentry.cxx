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
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

using rtl::OUString;
using rtl::OUStringBuffer;

namespace cppu {

sal_Bool component_writeInfoHelper(
    SAL_UNUSED_PARAMETER void *, void * pRegistryKey,
    ImplementationEntry const * entries)
{
    sal_Bool bRet = sal_False;
    try
    {
        if( pRegistryKey )
        {
            for( sal_Int32 i = 0; entries[i].create ; i ++ )
            {
                OUStringBuffer buf( 124 );
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("/") );
                buf.append( entries[i].getImplementationName() );
                buf.appendAscii(RTL_CONSTASCII_STRINGPARAM( "/UNO/SERVICES" ) );
                Reference< XRegistryKey > xNewKey(
                    reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( buf.makeStringAndClear()  ) );

                Sequence< OUString > seq = entries[i].getSupportedServiceNames();
                const OUString *pArray = seq.getConstArray();
                for ( sal_Int32 nPos = 0 ; nPos < seq.getLength(); nPos ++ )
                    xNewKey->createKey( pArray[nPos] );
            }
            bRet = sal_True;
        }
    }
    catch ( InvalidRegistryException & )
    {
        OSL_FAIL( "### InvalidRegistryException!" );
    }
    return bRet;
}


void * component_getFactoryHelper(
    char const * pImplName, SAL_UNUSED_PARAMETER void *,
    SAL_UNUSED_PARAMETER void *, ImplementationEntry const * entries)
{

      void * pRet = 0;
    Reference< XSingleComponentFactory > xFactory;

    for( sal_Int32 i = 0 ; entries[i].create ; i ++ )
    {
        OUString implName = entries[i].getImplementationName();
        if( 0 == implName.compareToAscii( pImplName ) )
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
