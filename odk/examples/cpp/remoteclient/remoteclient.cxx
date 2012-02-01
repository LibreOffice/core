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



#include <stdio.h>
#include <osl/mutex.hxx>
#include <cppuhelper/factory.hxx>

#include <com/sun/star/uno/XNamingService.hpp>

#include <com/sun/star/registry/XImplementationRegistration.hpp>

#include <com/sun/star/connection/XConnector.hpp>

#include <com/sun/star/bridge/XUnoUrlResolver.hpp>

#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include <cppuhelper/implbase1.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::connection;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::io;


namespace remotebridges_officeclient {

class PipeClientMain : public WeakImplHelper1< XMain >
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
                m_xSMgr->createInstance( OUString::createFromAscii( "com.sun.star.bridge.UnoUrlResolver" ) );
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
            seqNames.getArray()[0] = OUString::createFromAscii( "com.sun.star.bridge.example.RemoteClientSample" );
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
//==================================================================================================
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//==================================================================================================
// SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(
//  void * pServiceManager, void * pRegistryKey )
// {
//  if (pRegistryKey)
//  {
//      try
//      {
//          Reference< XRegistryKey > xNewKey(
//              reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
//                  OUString::createFromAscii( "/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );

//          const Sequence< OUString > & rSNL = getSupportedServiceNames();
//          const OUString * pArray = rSNL.getConstArray();
//          for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
//              xNewKey->createKey( pArray[nPos] );

//          return sal_True;
//      }
//      catch (InvalidRegistryException &)
//      {
//          OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
//      }
//  }
//  return sal_False;
// }

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
