/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include <string.h>

#include <usr/reflserv.hxx>  // for EXTERN_SERVICE_CALLTYPE

#include <usr/factoryhlp.hxx>
#include "testfactreg.hxx"


using namespace vos;
using namespace usr;

#ifdef __cplusplus
extern "C"
{
#endif

BOOL EXTERN_SERVICE_CALLTYPE exService_writeRegEntry(
        const UNO_INTERFACE(XRegistryKey)* xUnoKey)

{
    XRegistryKeyRef   xKey;
    uno2smart(xKey, *xUnoKey);

    UString str = UString( L"/" ) + OPipeTest_getImplementationName() + UString( L"/UNO/SERVICES" );
    XRegistryKeyRef xNewKey = xKey->createKey( str );
    xNewKey->createKey( OPipeTest_getServiceName() );

    str = UString( L"/" ) + ODataStreamTest_getImplementationName(1) + UString( L"/UNO/SERVICES" );
    xNewKey = xKey->createKey( str );
    xNewKey->createKey( ODataStreamTest_getServiceName(1) );

    str = UString( L"/" ) + ODataStreamTest_getImplementationName(2) + UString( L"/UNO/SERVICES" );
    xNewKey = xKey->createKey( str );
    xNewKey->createKey( ODataStreamTest_getServiceName(2) );

    str = UString( L"/" ) + OObjectStreamTest_getImplementationName(1) + UString( L"/UNO/SERVICES" );
    xNewKey = xKey->createKey( str );
    xNewKey->createKey( OObjectStreamTest_getServiceName(1) );

    str = UString( L"/" ) + OObjectStreamTest_getImplementationName(2) + UString( L"/UNO/SERVICES" );
    xNewKey = xKey->createKey( str );
    xNewKey->createKey( OObjectStreamTest_getServiceName(2) );

    str = UString( L"/" ) + OMarkableOutputStreamTest_getImplementationName() + UString( L"/UNO/SERVICES" );
    xNewKey = xKey->createKey( str );
    xNewKey->createKey( OMarkableOutputStreamTest_getServiceName() );

    str = UString( L"/" ) + OMarkableInputStreamTest_getImplementationName() + UString( L"/UNO/SERVICES" );
    xNewKey = xKey->createKey( str );
    xNewKey->createKey( OMarkableInputStreamTest_getServiceName() );

    str = UString( L"/" ) + OMyPersistObject_getImplementationName() + UString( L"/UNO/SERVICES" );
    xNewKey = xKey->createKey( str );
    xNewKey->createKey( OMyPersistObject_getServiceName() );

    return TRUE;
}


UNO_INTERFACE(XInterface) EXTERN_SERVICE_CALLTYPE exService_getFactory
(
    const wchar_t* implementationName,
    const UNO_INTERFACE(XMultiServiceFactory)* xUnoFact,
    const UNO_INTERFACE(XRegistryKey)*
)
{
    UNO_INTERFACE(XInterface) xUnoRet = {0, 0};

    XInterfaceRef           xRet;
    XMultiServiceFactoryRef xSMgr;
    UString                 aImplementationName(implementationName);

    uno2smart(xSMgr, *xUnoFact);

    if (aImplementationName == OPipeTest_getImplementationName() )
    {
        xRet = createSingleFactory( xSMgr, implementationName,
                                    OPipeTest_CreateInstance,
                                    OPipeTest_getSupportedServiceNames() );
    }
    else if( aImplementationName == ODataStreamTest_getImplementationName(1) ) {
        xRet = createSingleFactory( xSMgr , implementationName,
                                    ODataStreamTest_CreateInstance,
                                    ODataStreamTest_getSupportedServiceNames(1) );
    }
    else if( aImplementationName == ODataStreamTest_getImplementationName(2) ) {
        xRet = createSingleFactory( xSMgr , implementationName,
                                    ODataStreamTest_CreateInstance,
                                    ODataStreamTest_getSupportedServiceNames(2) );
    }
    else if( aImplementationName == OObjectStreamTest_getImplementationName(1) ) {
        xRet = createSingleFactory( xSMgr , implementationName,
                                    OObjectStreamTest_CreateInstance,
                                    OObjectStreamTest_getSupportedServiceNames(1) );
    }
    else if( aImplementationName == OObjectStreamTest_getImplementationName(2) ) {
        xRet = createSingleFactory( xSMgr , implementationName,
                                    OObjectStreamTest_CreateInstance,
                                    OObjectStreamTest_getSupportedServiceNames(2) );
    }
    else if( aImplementationName == OMarkableOutputStreamTest_getImplementationName() ) {
        xRet = createSingleFactory( xSMgr , implementationName,
                                    OMarkableOutputStreamTest_CreateInstance,
                                    OMarkableOutputStreamTest_getSupportedServiceNames() );
    }
    else if( aImplementationName == OMarkableInputStreamTest_getImplementationName() ) {
        xRet = createSingleFactory( xSMgr , implementationName,
                                    OMarkableInputStreamTest_CreateInstance,
                                    OMarkableInputStreamTest_getSupportedServiceNames() );
    }
    else if( aImplementationName == OMyPersistObject_getImplementationName() ) {
        xRet = createSingleFactory( xSMgr , implementationName,
                                    OMyPersistObject_CreateInstance,
                                    OMyPersistObject_getSupportedServiceNames() );
    }
    if (xRet.is())
    {
        smart2uno(xRet, xUnoRet);
    }

    return xUnoRet;
}

#ifdef __cplusplus
}
#endif

Sequence<BYTE> createSeq( char * p )
{
    Sequence<BYTE> seq( strlen( p )+1 );
    strcpy( (char * ) seq.getArray() , p );
    return seq;
}

Sequence<BYTE> createIntSeq( INT32 i )
{
    char pcCount[20];
    sprintf( pcCount , "%d" , i );
    return createSeq( pcCount );
}

