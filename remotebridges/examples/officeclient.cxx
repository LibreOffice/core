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
#include <osl/mutex.hxx>
#include <cppuhelper/factory.hxx>

#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/uno/XNamingService.hpp>

#include <com/sun/star/registry/ImplementationRegistration.hpp>

#include <com/sun/star/connection/XConnector.hpp>

#include <com/sun/star/bridge/XUnoUrlResolver.hpp>

#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/frame/XComponentLoader.hpp>

#include <com/sun/star/text/XTextDocument.hpp>

#include <cppuhelper/implbase1.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::connection;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;



namespace remotebridges_officeclient {

class OfficeClientMain : public WeakImplHelper1< XMain >
{
public:
    OfficeClientMain( const Reference< XMultiServiceFactory > &r ) :
        m_xSMgr( r )
        {}
public:     // Methods


    virtual sal_Int32 SAL_CALL run( const Sequence< OUString >& aArguments )
        throw(RuntimeException);


private: // helper methods
    void testWriter( const Reference < XComponent > & rComponent );
    void registerServices();
    Reference< XMultiServiceFactory > m_xSMgr;
};

void OfficeClientMain::testWriter( const Reference< XComponent > & rComponent )
{
    printf( "pasting some text into the writer document\n" );

    Reference< XTextDocument > rTextDoc( rComponent , UNO_QUERY );
    Reference< XText > rText = rTextDoc->getText();
    Reference< XTextCursor > rCursor = rText->createTextCursor();
    Reference< XTextRange > rRange ( rCursor , UNO_QUERY );

    rText->insertString( rRange, OUString("This text has been posted by the officeclient component"), sal_False );
}

/********************
 * does necessary service registration ( this could be done also by a setup tool )
 *********************/
void OfficeClientMain::registerServices( )
{
    // register services.
    // Note : this needs to be done only once and is in general done by the setup
    Reference < XImplementationRegistration > rImplementationRegistration(
        ImplementationRegistration::create(m_xSMgr) );

    if( ! rImplementationRegistration.is() )
    {
        printf( "Couldn't create registration component\n" );
        exit(1);
    }

    OUString aSharedLibrary[4];
    aSharedLibrary[0] =
        OUString( "connector.uno" SAL_DLLEXTENSION );
    aSharedLibrary[1] =
        OUString( "remotebridge.uno" SAL_DLLEXTENSION );
    aSharedLibrary[2] =
        OUString( "bridgefac.uno" SAL_DLLEXTENSION );
    aSharedLibrary[3] =
        OUString( "uuresolver.uno" SAL_DLLEXTENSION );

    sal_Int32 i;
    for( i = 0 ; i < 4 ; i ++ )
    {

        // build the system specific library name
        OUString aDllName = aSharedLibrary[i];

        try
        {
            // register the needed services in the servicemanager
            rImplementationRegistration->registerImplementation(
                OUString("com.sun.star.loader.SharedLibrary"),
                aDllName,
                Reference< XSimpleRegistry > () );
        }
        catch( Exception & )
        {
            printf( "couldn't register dll %s\n" ,
                    OUStringToOString( aDllName, RTL_TEXTENCODING_ASCII_US ).getStr() );
        }
    }
}

sal_Int32 OfficeClientMain::run( const Sequence< OUString > & aArguments ) throw ( RuntimeException )
{
    printf( "Connecting ....\n" );

    if( aArguments.getLength() == 1 )
    {
        try {
            registerServices();
            Reference < XInterface > r =
                m_xSMgr->createInstance( OUString("com.sun.star.bridge.UnoUrlResolver") );
            Reference < XUnoUrlResolver > rResolver( r , UNO_QUERY );
            r = rResolver->resolve( aArguments.getConstArray()[0] );

            Reference< XNamingService > rNamingService( r, UNO_QUERY );
            if( rNamingService.is() )
            {
                printf( "got the remote NamingService\n" );

                r = rNamingService->getRegisteredObject(OUString("StarOffice.ServiceManager"));

                Reference< XMultiServiceFactory > rRemoteSMgr( r , UNO_QUERY );

                Reference < XComponentLoader > rLoader(
                    rRemoteSMgr->createInstance( OUString( "com.sun.star.frame.Desktop" )),
                    UNO_QUERY );

                if( rLoader.is() )
                {

                    sal_Char *urls[] = {
                        "private:factory/swriter",
                        "private:factory/sdraw",
                        "private:factory/simpress",
                        "private:factory/scalc"
                    };

                    sal_Char *docu[]= {
                        "a new writer document ...\n",
                        "a new draw document ...\n",
                        "a new schedule document ...\n" ,
                        "a new calc document ...\n"
                    };
                    sal_Int32 i;
                    for( i = 0 ; i < 4 ; i ++ )
                    {
                        printf( "press any key to open %s\n" , docu[i] );
                        getchar();

                        Reference< XComponent > rComponent =
                            rLoader->loadComponentFromURL(
                                OUString::createFromAscii( urls[i] ) ,
                                OUString( "_blank"),
                                0 ,
                                Sequence < ::com::sun::star::beans::PropertyValue >() );

                        if( 0 == i )
                        {
                            testWriter( rComponent );
                        }
                        printf( "press any key to close the document\n" );
                        getchar();
                        rComponent->dispose();
                    }
                }
            }

        }
        catch( const ConnectionSetupException &e )
        {
            OString o = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
            printf( "%s\n", o.pData->buffer );
            printf( "couldn't access local resource ( possible security resons )\n" );
        }
        catch( const NoConnectException &e )
        {
            OString o = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
            printf( "%s\n", o.pData->buffer );
            printf( "no server listening on the resource\n" );
        }
        catch( const IllegalArgumentException &e )
        {
            OString o = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
            printf( "%s\n", o.pData->buffer );
            printf( "uno url invalid\n" );
        }
        catch( const RuntimeException & e )
        {
            OString o = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
            printf( "%s\n", o.pData->buffer );
            printf( "a remote call was aborted\n" );
        }
    }
    else
    {
        printf( "usage: (uno officeclient-component --) uno-url\n"
                "e.g.:  uno:socket,host=localhost,port=2002;urp;StarOffice.NamingService\n" );
        return 1;
    }
    return 0;
}

Reference< XInterface > SAL_CALL CreateInstance( const Reference< XMultiServiceFactory > &r)
{
    return Reference< XInterface > ( ( OWeakObject * ) new OfficeClientMain(r) );
}

Sequence< OUString > getSupportedServiceNames()
{
    static Sequence < OUString > *pNames = 0;
    if( ! pNames )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pNames )
        {
            static Sequence< OUString > seqNames(2);
            seqNames.getArray()[0] = OUString("com.sun.star.bridge.example.OfficeClientExample");
            pNames = &seqNames;
        }
    }
    return *pNames;
}

}

using namespace remotebridges_officeclient;
#define IMPLEMENTATION_NAME "com.sun.star.comp.remotebridges.example.OfficeClientSample"


extern "C"
{
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( "/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );

            const Sequence< OUString > & rSNL = getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_FAIL( "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLEMENTATION_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            CreateInstance, getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
