import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;


/** This class opens a new or an existing office document.
 */
public class DocumentLoader {
  public static void main(String args[]) {
    try {
      String sConnectionString = "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager";

      if ( args.length < 1 ) {
        System.out.println(
        "usage: java -classpath .;<Office path>/program/classes/jurt.jar;" +
        "<Office path>/program/classes/ridl.jar;" +
        "<Office path>/program/classes/sandbox.jar;" +
        "<Office path>/program/classes/unoil.jar;" +
        "<Office path>/program/classes/juh.jar " +
        "DocumentLoader \"<URL|path>\" [\"<connection>\"]" );
        System.out.println( "\ne.g.:" );
        System.out.println(
        "java -classpath .;d:/office60/program/classes/jurt.jar;" +
        "d:/office60/program/classes/ridl.jar;" +
        "d:/office60/program/classes/sandbox.jar;" +
        "d:/office60/program/classes/unoil.jar; " +
        "d:/office60/program/classes/juh.jar " +
        "DocumentLoader \"private:factory/swriter\"" );
        System.exit(1);
      }

      // It is possible to use a different connection string, passed as argument
      if ( args.length == 2 ) {
            sConnectionString = args[1];
      }

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

      /* Creates an instance of the component UnoUrlResolver which
         supports the services specified by the factory. */
      Object objectUrlResolver =
      xmulticomponentfactory.createInstanceWithContext(
      "com.sun.star.bridge.UnoUrlResolver", xcomponentcontext );

      // Create a new url resolver
      XUnoUrlResolver xurlresolver = ( XUnoUrlResolver )
      UnoRuntime.queryInterface( XUnoUrlResolver.class,
      objectUrlResolver );

      // Resolves an object that is specified as follow:
      // uno:<connection description>;<protocol description>;<initial object name>
      Object objectInitial = xurlresolver.resolve( sConnectionString );

      // Create a service manager from the initial object
      xmulticomponentfactory = ( XMultiComponentFactory )
      UnoRuntime.queryInterface( XMultiComponentFactory.class, objectInitial );

      // Query for the XPropertySet interface.
      XPropertySet xpropertysetMultiComponentFactory = ( XPropertySet )
      UnoRuntime.queryInterface( XPropertySet.class, xmulticomponentfactory );

      // Get the default context from the office server.
      Object objectDefaultContext =
      xpropertysetMultiComponentFactory.getPropertyValue( "DefaultContext" );

      // Query for the interface XComponentContext.
      xcomponentcontext = ( XComponentContext ) UnoRuntime.queryInterface(
      XComponentContext.class, objectDefaultContext );

      /* A desktop environment contains tasks with one or more
         frames in which components can be loaded. Desktop is the
         environment for components which can instanciate within
         frames. */
      XComponentLoader xcomponentloader = ( XComponentLoader )
      UnoRuntime.queryInterface( XComponentLoader.class,
      xmulticomponentfactory.createInstanceWithContext(
      "com.sun.star.frame.Desktop", xcomponentcontext ) );

      String sUrl = args[0];
      if ( sUrl.indexOf("private:") != 0) {
          java.io.File sourceFile = new java.io.File(args[0]);
          StringBuffer sTmp = new StringBuffer("file:///");
          sTmp.append(sourceFile.getCanonicalPath().replace('\\', '/'));
          sUrl = sTmp.toString();
      }

      // Load a Writer document, which will be automaticly displayed
      XComponent xcomponent = xcomponentloader.loadComponentFromURL(
      sUrl, "_blank", 0,
      new PropertyValue[0] );

      System.exit(0);
    }
    catch( Exception exception ) {
      System.err.println( exception );
    }
  }
}
