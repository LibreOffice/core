#include <stdio.h>

#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::bridge;
using namespace rtl;
using namespace cppu;

int main( )
{
    // create the initial component context
    Reference< XComponentContext > rComponentContext =
        defaultBootstrap_InitialComponentContext();

    // retrieve the servicemanager from the context
    Reference< XMultiComponentFactory > rServiceManager =
        rComponentContext->getServiceManager();

    // instantiate a sample service with the servicemanager.
    Reference< XInterface > rInstance =
        rServiceManager->createInstanceWithContext(
            OUString::createFromAscii("com.sun.star.bridge.UnoUrlResolver" ),
            rComponentContext );

    // Query for the XUnoUrlResolver interface
    Reference< XUnoUrlResolver > rResolver( rInstance, UNO_QUERY );

    if( ! rResolver.is() )
    {
        printf( "Error: Couldn't instantiate com.sun.star.bridge.UnoUrlResolver service\n" );
        return 1;
    }
    try
    {
        // resolve the uno-url
        rInstance = rResolver->resolve( OUString::createFromAscii(
            "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager" ) );

        if( ! rInstance.is() )
        {
            printf( "StarOffice.ServiceManager is not exported from remote counterpart\n" );
            return 1;
        }

        // query for the simpler XMultiServiceFactory interface, sufficient for scripting
        Reference< XMultiServiceFactory > rOfficeServiceManager (rInstance, UNO_QUERY);

        if( ! rInstance.is() )
        {
            printf( "XMultiServiceFactory interface is not exported for StarOffice.ServiceManager\n" );
            return 1;
        }

        printf( "Connected sucessfully to the office\n" );
    }
    catch( Exception &e )
    {
        OString o = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
        printf( "Error: %s\n", o.pData->buffer );
        return 1;
    }
    return 0;
}
