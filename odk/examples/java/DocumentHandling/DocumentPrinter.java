import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.view.XPrintable;


public class DocumentPrinter {
  public static void main(String args[]) {
    try {
      String sConnectionString = "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager";

      if ( args.length < 3 ) {
        System.out.println(
        "usage: java -classpath .;<Office path>/program/classes/jurt.jar;" +
        "<Office path>/program/classes/ridl.jar;" +
        "<Office path>/program/classes/sandbox.jar;" +
        "<Office path>/program/classes/unoil.jar;" +
        "<Office path>/program/classes/juh.jar " +
        "DocumentLoader \"<Favoured printer>\" \"<URL|path>\" " +
        "\"<Pages>\" [\"<Connection>\"]" );
        System.out.println( "\ne.g.:" );
        System.out.println(
        "java -classpath .;d:/office60/program/classes/jurt.jar;" +
        "d:/office60/program/classes/ridl.jar;" +
        "d:/office60/program/classes/sandbox.jar;" +
        "d:/office60/program/classes/unoil.jar; " +
        "d:/office60/program/classes/juh.jar " +
        "DocumentLoader \"amadeus\" \"file:///f:/TestPrint.sxw\" \"1-3;7;9\"" );
        System.exit(1);
      }

      // It is possible to use a different connection string, passed as argument
      if ( args.length == 4 ) {
            sConnectionString = args[3];
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

      java.io.File sourceFile = new java.io.File(args[1]);
      StringBuffer sUrl = new StringBuffer("file:///");
      sUrl.append(sourceFile.getCanonicalPath().replace('\\', '/'));

      // Load a Writer document, which will be automaticly displayed
      XComponent xcomponent = xcomponentloader.loadComponentFromURL(
      sUrl.toString(), "_blank", 0,
      new PropertyValue[0] );

      // Querying for the interface XPrintable on the loaded document
      XPrintable xprintable =
      ( XPrintable ) UnoRuntime.queryInterface( XPrintable.class, xcomponent );

      // Setting the property "Name" for the favoured printer (name of IP address)
      PropertyValue []propertyvalue = new PropertyValue[ 1 ];
      propertyvalue[ 0 ] = new PropertyValue();
      propertyvalue[ 0 ].Name = "Name";
      propertyvalue[ 0 ].Value = args[ 0 ];

      // Setting the name of the printer
      xprintable.setPrinter( propertyvalue );

      // Setting the property "Pages" so that only the desired pages will be printed.
      propertyvalue[ 0 ] = new PropertyValue();
      propertyvalue[ 0 ].Name = "Pages";
      propertyvalue[ 0 ].Value = args[ 2 ];

      // Printing the loaded document
      xprintable.print( propertyvalue );

      System.exit(0);
    }
    catch( Exception exception ) {
      System.err.println( exception );
    }
  }
}
