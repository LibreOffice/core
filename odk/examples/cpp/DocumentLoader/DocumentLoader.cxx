/*****************************************************************************
 *****************************************************************************
 *
 * Simple client application using the UnoUrlResolver service.
 *
 *****************************************************************************
 *****************************************************************************/
#include <stdio.h>

#include <cppuhelper/bootstrap.hxx>

#include <osl/file.hxx>
#include <osl/process.h>

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
int SAL_CALL main( int argc, char **argv )
{
    OUString sConnectionString(RTL_CONSTASCII_USTRINGPARAM("uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager"));
     if (argc < 2)
    {
        printf("using: DocumentLoader <file_url> [<uno_connection_url>]\n\n"
               "example: DocumentLoader  \"file:///e:/temp/test.sxw\" \"uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager\"\n");
        exit(1);
    }
     if (argc == 3)
    {
        sConnectionString = OUString::createFromAscii(argv[2]);
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
    try
    {
        xInterface = Reference< XInterface >(
            resolver->resolve( sConnectionString ), UNO_QUERY );
    }
    catch ( Exception& e )
    {
        printf("Error: cannot establish a connection using '%s':\n       %s\n",
               OUStringToOString(sConnectionString, RTL_TEXTENCODING_ASCII_US).getStr(),
               OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        exit(1);
    }

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
    OUString sDocUrl, sWorkingDir;
    osl_getProcessWorkingDir(&sWorkingDir.pData);
    osl::FileBase::getAbsoluteFileURL( sWorkingDir, OUString::createFromAscii(argv[1]), sDocUrl);

    Reference< XComponent > xComponent = xComponentLoader->loadComponentFromURL(
        sDocUrl, OUString( RTL_CONSTASCII_USTRINGPARAM("_blank") ), 0,
        Sequence < ::com::sun::star::beans::PropertyValue >() );

    // dispose the local service manager
    Reference< XComponent >::query( xMultiComponentFactoryClient )->dispose();

    return 0;
}
