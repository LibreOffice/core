import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.container.XSet;
import com.sun.star.lang.XServiceInfo;

public class TestMinimalComponent {
  public static void main(String args[]) {
    try {
      /* Bootstraps a component context with the jurt base components
         registered. Component context to be granted to a component for running.
         Arbitrary values can be retrieved from the context. */
      XComponentContext xcomponentcontext =
      com.sun.star.comp.helper.Bootstrap.createInitialComponentContext( null );

      /* Gets the service manager instance to be used (or null). This method has
         been added for convenience, because the service manager is a often used
         object. */
      XMultiComponentFactory xmulticomponentfactory =
      xcomponentcontext.getServiceManager();

      // Querying for the interface XSet on XMultiComponentFactory
      XSet xsetMultiServiceFactory = ( XSet ) UnoRuntime.queryInterface(
      XSet.class, xmulticomponentfactory );

      // Getting the XSingleServiceFactory for the minimal component
      XSingleServiceFactory xsingleservicefactoryMinimalComponent =
      MinimalComponent.__getServiceFactory(
      "MinimalComponent$MinimalComponentImplementation",
      ( XMultiServiceFactory ) UnoRuntime.queryInterface(
      XMultiServiceFactory.class, xmulticomponentfactory ),
      null );

      // Inserting the XSingleServiceFactory into the container
      xsetMultiServiceFactory.insert( xsingleservicefactoryMinimalComponent );

      // Creating an instance of the minimal component
      Object objectMinimalComponent =
      xmulticomponentfactory.createInstanceWithContext(
      "org.OpenOffice.MinimalComponent", xcomponentcontext );

      XServiceInfo xserviceinfoMinimalComponent = ( XServiceInfo )
      UnoRuntime.queryInterface( XServiceInfo.class,
      objectMinimalComponent );

      System.out.println( "\nXServiceInfo is used to get the implementation name:\n    -> " +
                          xserviceinfoMinimalComponent.getImplementationName() +
                          "\nOk");

      // Removing the XSingleServiceFactory of the minimal Component from the container
      xsetMultiServiceFactory.remove( xsingleservicefactoryMinimalComponent );

      xcomponentcontext = null;

      System.exit(0);
    }
    catch( Exception exception ) {
      System.err.println( exception );
    }
  }
}
