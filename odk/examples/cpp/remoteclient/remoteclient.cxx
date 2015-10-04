/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include <stdio.h>
#include <osl/mutex.hxx>
#include <uno/lbnames.h>
#include <cppuhelper/factory.hxx>

#include <com/sun/star/uno/XNamingService.hpp>

#include <com/sun/star/connection/XConnector.hpp>

#include <com/sun/star/bridge/XUnoUrlResolver.hpp>

#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include <cppuhelper/implbase.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::connection;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::io;


namespace remotebridges_officeclient {

class PipeClientMain : public WeakImplHelper < XMain >
{
public:
    PipeClientMain( const Reference< XMultiServiceFactory > &r ) :
        m_xSMgr( r )
        {}
public:     // Methods


    virtual sal_Int32 SAL_CALL run( const Sequence< OUString >& aArguments )
        throw(RuntimeException);


private: // helper methods
    void testPipe( const Reference < XInterface > & rComponent );
    Reference< XMultiServiceFactory > m_xSMgr;
};

void PipeClientMain::testPipe( const Reference< XInterface > & rxInterface )
{
    // ask for the input stream
    Reference< XInputStream > rInputStream( rxInterface, UNO_QUERY );

    // ask for the output stream
    Reference< XOutputStream > rOutputStream( rxInterface, UNO_QUERY );

    if( rInputStream.is() && rOutputStream.is() )
    {
        printf( "got inputstream and outputstream from remote process\n" );

        sal_Int8 a[] = { 5,4,3,2,1,0 };
        Sequence< sal_Int8 > seq( a , 6 );
        rOutputStream->writeBytes( seq );
        rOutputStream->closeOutput();

        Sequence< sal_Int8> seqRead;
        if( rInputStream->readBytes( seqRead ,3 ) != 3 )
        {
            printf( "error : Couldn't read the expected number of bytes\n" );
            return;
        }

        if( seqRead.getConstArray()[0] != 5 ||
            seqRead.getConstArray()[1] != 4 ||
            seqRead.getConstArray()[2] != 3 )
        {
            printf( "error : The array doesn't contain the expected values\n" );
            return;
        }

        // try to read more bytes than written
        if( rInputStream->readBytes( seqRead , 4 ) != 3 )
        {
            printf( "error : Got an unexpected number of bytes\n" );
            return;
        }

        if( seqRead.getConstArray()[0] != 2 ||
            seqRead.getConstArray()[1] != 1 ||
            seqRead.getConstArray()[2] != 0 )
        {
            printf( "error : The array doesn't contain the expected values\n" );
            return;
        }

        printf( "pipe test worked perfect\n" );
    }
    else
    {
        printf( "Couldn't get inputstream and outputstream\n" );
    }
}


sal_Int32 PipeClientMain::run( const Sequence< OUString > & aArguments ) throw ( RuntimeException )
{
    printf( "Connecting ....\n" );

    if( aArguments.getLength() == 1 )
    {
        try {
            Reference < XInterface > r =
                m_xSMgr->createInstance("com.sun.star.bridge.UnoUrlResolver");
            Reference < XUnoUrlResolver > rResolver( r , UNO_QUERY );

            // connect to the remote process and retrieve the initial object
            r = rResolver->resolve( aArguments.getConstArray()[0] );

            if( r.is() )
            {
                printf( "got the remote initial object\n" );
                testPipe( r );
            }
            else
            {
                printf( "error : didn't get the initial object\n" );
            }
        }
        catch( ConnectionSetupException &e )
        {
            OString o = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
            printf( "%s\n", o.pData->buffer );
            printf( "couldn't access local resource ( possible security resons )\n" );
        }
        catch( NoConnectException &e )
        {
            OString o = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
            printf( "%s\n", o.pData->buffer );
            printf( "no server listening on the resource\n" );
        }
        catch( IllegalArgumentException &e )
        {
            OString o = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
            printf( "%s\n", o.pData->buffer );
            printf( "uno url invalid\n" );
        }
        catch( RuntimeException & e )
        {
            OString o = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
            printf( "%s\n", o.pData->buffer );
            printf( "a remote call was aborted\n" );
        }
    }
    else
    {
        printf( "usage: (uno remoteclient-component --) uno-url\n"
                "e.g.:  uno:socket,host=localhost,port=2083;urp;MyPipe\n" );
        return 1;
    }
    return 0;
}

Reference< XInterface > SAL_CALL CreateInstance( const Reference< XMultiServiceFactory > &r)
{
    return Reference< XInterface > ( ( OWeakObject * ) new PipeClientMain(r) );
}

Sequence< OUString > getSupportedServiceNames()
{
    static Sequence < OUString > *pNames = 0;
    if( ! pNames )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( !pNames )
        {
            static Sequence< OUString > seqNames(1);
            seqNames[0] = "com.sun.star.bridge.example.RemoteClientSample";
            pNames = &seqNames;
        }
    }
    return *pNames;
}

}

using namespace remotebridges_officeclient;
#define IMPLEMENTATION_NAME "com.sun.star.comp.product.example.RemoteClientSample"


extern "C"
{
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

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    char const ** ppEnvTypeName, uno_Environment **)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
