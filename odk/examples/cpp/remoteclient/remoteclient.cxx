/*************************************************************************
 *
 *  $RCSfile: remoteclient.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 13:39:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <stdio.h>
#include <osl/mutex.hxx>
#include <cppuhelper/factory.hxx>

#include <cppuhelper/servicefactory.hxx>

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
                "e.g.:  uno:socket,host=localhost,port=2002;urp;MyPipe\n" );
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
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
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
                    OUString::createFromAscii( "/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );

            const Sequence< OUString > & rSNL = getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
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
