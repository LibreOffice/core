/*****************************************************************************
 *****************************************************************************
 *
 * Simple client application using the UnoUrlResolver service.
 *
 *****************************************************************************
 *****************************************************************************/
#include <stdio.h>

#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <string.h>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::bridge;
using namespace com::sun::star::frame;
using namespace com::sun::star::registry;


//============================================================================
int SAL_CALL main( int argc, char **argv ) {

     if (argc != 3)
    {
        printf("using: DocumentLoader <uno_connection_url> <file_url>\n\n"
               "example: DocumentLoader \"uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager\" \"file://e:/temp/test.sxw\"\n");
        exit(1);
    }

    // Creates a simple registry service instance.
    Reference< XSimpleRegistry > xSimpleRegistry(
        ::cppu::createSimpleRegistry() );

    // Connects the registry to a persistent data source represented by an URL.
    xSimpleRegistry->open( OUString( RTL_CONSTASCII_USTRINGPARAM(
        "DocumentLoader.rdb") ), sal_True, sal_False );

    /* Bootstraps an initial component context with service manager upon a given
       registry. This includes insertion of initial services:
       - (registry) service manager, shared lib loader,
       - simple registry, nested registry,
       - implementation registration
       - registry typedescription provider, typedescription manager (also
         installs it into cppu core)
    */
    Reference< XComponentContext > xComponentContext(
        ::cppu::bootstrap_InitialComponentContext( xSimpleRegistry ) );

    /* Bootstraps an initial component context with service manager upon default
       types and services registry. This includes insertion of initial services:
       - (registry) service manager, shared lib loader,
       - simple registry, nested registry,
       - implementation registration
       - registry typedescription provider, typedescription manager (also
         installs it into cppu core)

       This function tries to find its parameters via these bootstrap variables:
       - UNO_TYPES        -- a space separated list of file urls of type rdbs
       - UNO_SERVICES     -- a space separated list of file urls of service rdbs
       - UNO_WRITERDB     -- a file url of a write rdb (e.g. user.rdb)

       For further info, please look at:
       http://udk.openoffice.org/common/man/concept/uno_default_bootstrapping.html
    */
    /*
    Reference< XComponentContext > xComponentContext(
        ::cppu::defaultBootstrap_InitialComponentContext() );
    OSL_ASSERT( xcomponentcontext.is() );
    */

    /* Gets the service manager instance to be used (or null). This method has
       been added for convenience, because the service manager is a often used
       object.
    */
    Reference< XMultiComponentFactory > xMultiComponentFactoryClient(
        xComponentContext->getServiceManager() );

    /* Creates an instance of a component which supports the services specified
       by the factory.
    */
    Reference< XInterface > xInterface =
        xMultiComponentFactoryClient->createInstanceWithContext(
            OUString::createFromAscii( "com.sun.star.bridge.UnoUrlResolver" ),
            xComponentContext );

    Reference< XUnoUrlResolver > resolver( xInterface, UNO_QUERY );

    // Resolves the component context from the office, on the uno URL given by argv[1].
    xInterface = Reference< XInterface >(
        resolver->resolve( OUString::createFromAscii( argv[1] ) ), UNO_QUERY );

    // gets the server component context as property of the office component factory
    Reference< XPropertySet > xPropSet( xInterface, UNO_QUERY );
    xPropSet->getPropertyValue( OUString::createFromAscii("DefaultContext") ) >>= xComponentContext;

    // gets the service manager from the office
    Reference< XMultiComponentFactory > xMultiComponentFactoryServer(
        xComponentContext->getServiceManager() );

    /* Creates an instance of a component which supports the services specified
       by the factory. Important: using the office component context.
    */
    Reference < XComponentLoader > xComponentLoader(
        xMultiComponentFactoryServer->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop" ) ),
            xComponentContext ), UNO_QUERY );

    /* Loads a component specified by an URL into the specified new or existing
       frame.
    */
    Reference< XComponent > xComponent = xComponentLoader->loadComponentFromURL(
        OUString::createFromAscii( argv[ 2 ] ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("_blank") ),
        0,
        Sequence < ::com::sun::star::beans::PropertyValue >() );

    // dispose the local service manager
    Reference< XComponent >::query( xMultiComponentFactoryClient )->dispose();

    return 0;
}
