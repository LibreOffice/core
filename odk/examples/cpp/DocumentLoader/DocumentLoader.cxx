/*****************************************************************************
 *****************************************************************************
 *
 * Simple client application using the UnoUrlResolver service.
 *
 *****************************************************************************
 *****************************************************************************/

#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>

#include <string.h>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::bridge;
using namespace com::sun::star::frame;
using namespace com::sun::star::registry;


//============================================================================
int SAL_CALL main( int argc, char **argv ) {
  // Creates a simple registry service instance.
  Reference< XSimpleRegistry > xsimpleregistry(
    ::cppu::createSimpleRegistry() );

  // Connects the registry to a persistent data source represented by an URL.
  xsimpleregistry->open( OUString( RTL_CONSTASCII_USTRINGPARAM(
    "DocumentLoader.rdb") ), sal_True, sal_False );

  /* Bootstraps an initial component context with service manager upon a given
     registry. This includes insertion of initial services:
      - (registry) service manager, shared lib loader,
      - simple registry, nested registry,
      - implementation registration
      - registry typedescription provider, typedescription manager (also
        installs it into cppu core)
  */
  Reference< XComponentContext > xcomponentcontext(
    ::cppu::bootstrap_InitialComponentContext( xsimpleregistry ) );

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
  Reference< XComponentContext > xcomponentcontext(
    ::cppu::defaultBootstrap_InitialComponentContext() );
  OSL_ASSERT( xcomponentcontext.is() );
  */

  /* Gets the service manager instance to be used (or null). This method has
     been added for convenience, because the service manager is a often used
     object.
  */
  /*
  Reference< XMultiComponentFactory > xmulticomponentfactoryClient(
    xcomponentcontext->getServiceManager() );
  */

  Reference< XMultiComponentFactory > xmulticomponentfactoryClient;

  // Gets the service manager from the context.
  OSL_VERIFY( xcomponentcontext->getValueByName( OUString(
    RTL_CONSTASCII_USTRINGPARAM(
    "/singletons/com.sun.star.lang.theServiceManager") ) ) >>=
    xmulticomponentfactoryClient );

  /* Creates an instance of a component which supports the services specified
     by the factory.
  */
  Reference< XInterface > xinterface =
    xmulticomponentfactoryClient->createInstanceWithContext(
    OUString::createFromAscii( "com.sun.star.bridge.UnoUrlResolver" ),
    xcomponentcontext );

  Reference< XUnoUrlResolver > resolver( xinterface, UNO_QUERY );

  // Create a UNO URL for connecting the Office server
  char pcharConnection[160];
  strcpy( pcharConnection, "uno:" );
  strcat( pcharConnection, argv[ 1 ] );
  strcat( pcharConnection, ";StarOffice.ServiceManager" );

  // Resolves an object, on the uno URL.
  Reference< XMultiComponentFactory > xmulticomponentfactoryServer(
    resolver->resolve( OUString::createFromAscii(
      pcharConnection ) ),
    UNO_QUERY );

  /* Creates an instance of a component which supports the services specified
     by the factory.
  */
  Reference < XComponentLoader > xcomponentloader(
    xmulticomponentfactoryServer->createInstanceWithContext(
      OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop" ) ),
      xcomponentcontext ), UNO_QUERY );

  /* Loads a component specified by an URL into the specified new or existing
     frame.
  */
  Reference< XComponent > xcomponent = xcomponentloader->loadComponentFromURL(
    OUString::createFromAscii( argv[ 2 ] ),
    OUString( RTL_CONSTASCII_USTRINGPARAM("_blank") ),
    0,
    Sequence < ::com::sun::star::beans::PropertyValue >() );

  Reference< XComponent >::query( xmulticomponentfactoryClient )->dispose();

  return 0;
}
