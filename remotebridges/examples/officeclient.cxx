/*************************************************************************
 *
 *  $RCSfile: officeclient.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jbu $ $Date: 2000-10-26 14:12:07 $
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


#ifdef SOLARIS
extern "C" void ChangeGlobalInit();
#endif

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

    rText->insertString( rRange, OUString::createFromAscii( "This text has been posted by the officeclient component" ), sal_False );
}

/********************
 * does necessary service registration ( this could be done also by a setup tool )
 *********************/
void OfficeClientMain::registerServices( )
{
    // register services.
    // Note : this needs to be done only once and is in general done by the setup
    Reference < XImplementationRegistration > rImplementationRegistration(

        m_xSMgr->createInstance(
            OUString::createFromAscii( "com.sun.star.registry.ImplementationRegistration" )),
        UNO_QUERY );

    if( ! rImplementationRegistration.is() )
    {
        printf( "Couldn't create registration component\n" );
        exit(1);
    }

    OUString aSharedLibrary[4];
    aSharedLibrary[0] = OUString::createFromAscii( "connectr" );
    aSharedLibrary[1] = OUString::createFromAscii( "remotebridge" );
    aSharedLibrary[2] = OUString::createFromAscii( "brdgfctr" );
    aSharedLibrary[3] = OUString::createFromAscii( "uuresolver" );

    sal_Int32 i;
    for( i = 0 ; i < 4 ; i ++ )
    {

        // build the system specific library name
#ifdef SAL_W32
        OUString aDllName = aSharedLibrary[i];
        aDllName += OUString::createFromAscii(".dll");
#else
        OUString aDllName = OUString::createFromAscii("lib");
        aDllName += aSharedLibrary[i];
        aDllName += OUString::createFromAscii(".so");
#endif

        try
        {
            // register the needed services in the servicemanager
            rImplementationRegistration->registerImplementation(
                OUString::createFromAscii( "com.sun.star.loader.SharedLibrary" ),
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

sal_Int32 OfficeClientMain::run( const Sequence< OUString > & aArguments )
{
    printf( "Connecting ....\n" );

    if( aArguments.getLength() == 1 )
    {
        try {
            registerServices();
            Reference < XInterface > r =
                m_xSMgr->createInstance( OUString::createFromAscii( "com.sun.star.bridge.UnoUrlResolver" ) );
            Reference < XUnoUrlResolver > rResolver( r , UNO_QUERY );
            r = rResolver->resolve( aArguments.getConstArray()[0] );

            Reference< XNamingService > rNamingService( r, UNO_QUERY );
            if( rNamingService.is() )
            {
                printf( "got the remote NamingService\n" );

                r = rNamingService->getRegisteredObject(OUString::createFromAscii("StarOffice.ServiceManager"));

                Reference< XMultiServiceFactory > rRemoteSMgr( r , UNO_QUERY );

                Reference < XComponentLoader > rLoader(
                    rRemoteSMgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop" ))),
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
                                OUString( RTL_CONSTASCII_USTRINGPARAM("_blank")),
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
            seqNames.getArray()[0] = OUString::createFromAscii( "com.sun.star.bridge.example.OfficeClientExample" );
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
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
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
