/***************************************************************************************************
 ***************************************************************************************************
 *
 * simple client application using the UnoUrlResolver service.
 *
 ***************************************************************************************************
 **************************************************************************************************/

#include <rtl/ustring.hxx>

#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>

#include <string.h>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::bridge;
using namespace com::sun::star::frame;


//==================================================================================================
int SAL_CALL main( int argc, char **argv ) {
  Reference< XMultiServiceFactory > xmultiservicefactoryClient(
    ::cppu::createRegistryServiceFactory(
      OUString::createFromAscii( "DocumentLoader.rdb") ) );

  Reference< XInterface > xinterface ;
  xinterface = xmultiservicefactoryClient->createInstance(
    OUString::createFromAscii( "com.sun.star.bridge.UnoUrlResolver"));
  Reference< XUnoUrlResolver > resolver( xinterface, UNO_QUERY );

  char pcharConnection[160];
  strcpy( pcharConnection, "uno:" );
  strcat( pcharConnection, argv[ 1 ] );
  strcat( pcharConnection, ";StarOffice.ServiceManager" );

  Reference< XMultiServiceFactory > xmultiservicefactoryServer(
    resolver->resolve( OUString::createFromAscii(
      pcharConnection ) ),
    UNO_QUERY );

  Reference < XComponentLoader > xcomponentloader(
    xmultiservicefactoryServer->createInstance(
      OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop" ))),
    UNO_QUERY );

  Reference< XComponent > xcomponent = xcomponentloader->loadComponentFromURL(
    OUString::createFromAscii( argv[ 2 ] ),
    OUString( RTL_CONSTASCII_USTRINGPARAM("_blank") ),
    0,
    Sequence < ::com::sun::star::beans::PropertyValue >() );

  Reference< XComponent >::query( xmultiservicefactoryClient )->dispose();

  return 0;
}
