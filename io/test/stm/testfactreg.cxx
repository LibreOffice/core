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

#include <stdio.h>
#include <string.h>

#include <osl/diagnose.h>

#include <cppuhelper/factory.hxx>  // for EXTERN_SERVICE_CALLTYPE

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

#include "testfactreg.hxx"


extern "C"
{

sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey ) );

            OUString str = OUString("/") +
                OPipeTest_getImplementationName() +
                OUString("/UNO/SERVICES");
            Reference< XRegistryKey >  xNewKey = xKey->createKey( str );
            xNewKey->createKey( OPipeTest_getServiceName() );

            str = OUString("/") +
                OPumpTest_getImplementationName() +
                OUString("/UNO/SERVICES");
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OPumpTest_getServiceName() );

            str = OUString("/") +
                ODataStreamTest_getImplementationName(1) +
                OUString("/UNO/SERVICES");
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( ODataStreamTest_getServiceName(1) );

            str = OUString("/") +
                ODataStreamTest_getImplementationName(2) +
                OUString("/UNO/SERVICES");
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( ODataStreamTest_getServiceName(2) );

            str = OUString("/") +
                OObjectStreamTest_getImplementationName(1) +
                OUString("/UNO/SERVICES");
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OObjectStreamTest_getServiceName(1) );

            str = OUString("/") +
                OObjectStreamTest_getImplementationName(2) +
                OUString("/UNO/SERVICES");
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OObjectStreamTest_getServiceName(2) );

            str = OUString("/") +
                OMarkableOutputStreamTest_getImplementationName() +
                OUString("/UNO/SERVICES");
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OMarkableOutputStreamTest_getServiceName() );

            str = OUString("/") +
                OMarkableInputStreamTest_getImplementationName() +
                OUString("/UNO/SERVICES");
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OMarkableInputStreamTest_getServiceName() );

            str = OUString("/") +
                OMyPersistObject_getImplementationName() +
                OUString("/UNO/SERVICES");
            xNewKey = xKey->createKey( str );
            xNewKey->createKey( OMyPersistObject_getServiceName() );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_FAIL( "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager )
    {
        Reference< XSingleServiceFactory > xRet;
        Reference< XMultiServiceFactory > xSMgr =
            reinterpret_cast< XMultiServiceFactory * > ( pServiceManager );

        OUString aImplementationName = OUString::createFromAscii( pImplName );

        if (aImplementationName == OPipeTest_getImplementationName() )
        {
            xRet = createSingleFactory( xSMgr, aImplementationName,
                                        OPipeTest_CreateInstance,
                                        OPipeTest_getSupportedServiceNames() );
        }
        else if (aImplementationName == OPumpTest_getImplementationName() )
        {
            xRet = createSingleFactory( xSMgr, aImplementationName,
                                        OPumpTest_CreateInstance,
                                        OPumpTest_getSupportedServiceNames() );
        }

        else if( aImplementationName == ODataStreamTest_getImplementationName(1) ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        ODataStreamTest_CreateInstance,
                                        ODataStreamTest_getSupportedServiceNames(1) );
        }
        else if( aImplementationName == ODataStreamTest_getImplementationName(2) ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        ODataStreamTest_CreateInstance,
                                        ODataStreamTest_getSupportedServiceNames(2) );
        }
        else if( aImplementationName == OObjectStreamTest_getImplementationName(1) ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        OObjectStreamTest_CreateInstance,
                                        OObjectStreamTest_getSupportedServiceNames(1) );
        }
        else if( aImplementationName == OObjectStreamTest_getImplementationName(2) ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        OObjectStreamTest_CreateInstance,
                                        OObjectStreamTest_getSupportedServiceNames(2) );
        }
        else if( aImplementationName == OMarkableOutputStreamTest_getImplementationName() ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        OMarkableOutputStreamTest_CreateInstance,
                                        OMarkableOutputStreamTest_getSupportedServiceNames() );
        }
        else if( aImplementationName == OMarkableInputStreamTest_getImplementationName() ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                        OMarkableInputStreamTest_CreateInstance,
                                        OMarkableInputStreamTest_getSupportedServiceNames() );
        }
        else if( aImplementationName == OMyPersistObject_getImplementationName() ) {
            xRet = createSingleFactory( xSMgr , aImplementationName,
                                    OMyPersistObject_CreateInstance,
                                        OMyPersistObject_getSupportedServiceNames() );
        }
        if (xRet.is())
        {
            xRet->acquire();
            pRet = xRet.get();
        }
    }

    return pRet;
}

}

Sequence<sal_Int8 > createSeq( char * p )
{
    Sequence<sal_Int8> seq( strlen( p )+1 );
    strcpy( (char * ) seq.getArray() , p );
    return seq;
}

Sequence<sal_Int8> createIntSeq( sal_Int32 i )
{
    char pcCount[20];
    sprintf( pcCount , "%d" , i );
    return createSeq( pcCount );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
