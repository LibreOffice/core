import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.container.XSet;
import com.sun.star.lang.XServiceInfo;

import MinimalComponent;


public class TestMinimalComponent {
  public static void main(String args[]) {
    try {
      /* Bootstraps a servicemanager with the jurt base components
         registered */
      XMultiServiceFactory xmultiservicefactory =
      com.sun.star.comp.helper.Bootstrap.createSimpleServiceManager();

      // Querying for the interface XSet on XMultiServiceFactory
      XSet xsetMultiServiceFactory = ( XSet ) UnoRuntime.queryInterface(
      XSet.class, xmultiservicefactory );

      // Getting the XSingleServiceFactory for the minimal component
      XSingleServiceFactory xsingleservicefactoryMinimalComponent =
      MinimalComponent.__getServiceFactory(
      "MinimalComponent$MinimalComponentImplementation", xmultiservicefactory,
      null );

      // Inserting the XSingleServiceFactory into the container
      xsetMultiServiceFactory.insert( xsingleservicefactoryMinimalComponent );

      // Creating an instance of the minimal component
      Object objectMinimalComponent = xmultiservicefactory.createInstance(
      "org.OpenOffice.MinimalComponent" );

      XServiceInfo xserviceinfoMinimalComponent = ( XServiceInfo )
      UnoRuntime.queryInterface( XServiceInfo.class,
      objectMinimalComponent );

      System.out.println(
      xserviceinfoMinimalComponent.getImplementationName() );

      // Removing the XSingleServiceFactory of the minimal Component from the container
      xsetMultiServiceFactory.remove( xsingleservicefactoryMinimalComponent );

      System.exit(0);
    }
    catch( Exception exception ) {
      System.err.println( exception );
    }
  }
}
