import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.text.XText;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextContent;
import com.sun.star.text.TextContentAnchorType;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import java.io.PrintWriter;
import java.io.BufferedWriter;
import java.io.FileWriter;


public class GraphicsInserter {
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
        "GraphicsInserter \"<Graphic URL|path>\" [\"<Connection>\"]" );
        System.out.println( "\ne.g.:" );
        System.out.println(
        "java -classpath .;d:/office60/program/classes/jurt.jar;" +
        "d:/office60/program/classes/ridl.jar;" +
        "d:/office60/program/classes/sandbox.jar;" +
        "d:/office60/program/classes/unoil.jar; " +
        "d:/office60/program/classes/juh.jar " +
        "GraphicsInserter \"file:///f:/TestGraphics.gif\"" );
        System.exit( 1 );
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

      // Load a Writer document, which will be automaticly displayed
      XComponent xcomponent = xcomponentloader.loadComponentFromURL(
      "private:factory/swriter", "_blank", 0,
      new PropertyValue[0] );

      // Querying for the interface XTextDocument on the xcomponent
      XTextDocument xtextdocument = ( XTextDocument ) UnoRuntime.queryInterface(
      XTextDocument.class, xcomponent );

      // Querying for the interface XMultiServiceFactory on the xtextdocument
      XMultiServiceFactory xmultiservicefactoryDocument =
      ( XMultiServiceFactory ) UnoRuntime.queryInterface(
      XMultiServiceFactory.class, xtextdocument );

      // Providing a log file for output
      PrintWriter printwriterLog = new PrintWriter( new BufferedWriter(
      new FileWriter( "log.txt" ) ) );

      Object objectGraphic = null;
      try {
        // Creating the service GraphicObject
        objectGraphic =
        xmultiservicefactoryDocument.createInstance(
        "com.sun.star.text.GraphicObject" );
      }
      catch ( Exception exception ) {
        System.out.println( "Could not create instance" );
        exception.printStackTrace( printwriterLog );
      }

      // Getting the text
      XText xtext = xtextdocument.getText();

      // Getting the cursor on the document
      XTextCursor xtextcursor = xtext.createTextCursor();

      // Querying for the interface XTextContent on the GraphicObject
      XTextContent xtextcontent = ( XTextContent )
      UnoRuntime.queryInterface( XTextContent.class, objectGraphic );

      // Printing information to the log file
      printwriterLog.println( "inserting graphic" );
      try {
        // Inserting the content
        xtext.insertTextContent( xtextcursor, xtextcontent, true );
      } catch ( Exception exception ) {
        System.out.println( "Could not insert Content" );
        exception.printStackTrace();
      }

      // Printing information to the log file
      printwriterLog.println( "adding graphic" );

      // Querying for the interface XPropertySet on GraphicObject
      XPropertySet xpropertyset = (XPropertySet)
      UnoRuntime.queryInterface(XPropertySet.class,objectGraphic);
      try {
        // Creating a string for the graphic url
        java.io.File sourceFile = new java.io.File(args[0]);
        StringBuffer sUrl = new StringBuffer("file:///");
        sUrl.append(sourceFile.getCanonicalPath().replace('\\', '/'));
        System.out.println( "insert graphic \"" + sUrl + "\"");

        // Setting the anchor type
        xpropertyset.setPropertyValue( "AnchorType",
        TextContentAnchorType.AT_PARAGRAPH );

        // Setting the graphic url
        xpropertyset.setPropertyValue( "GraphicURL", sUrl.toString() );

        // Setting the horizontal position
        xpropertyset.setPropertyValue( "HoriOrientPosition",
        new Integer( 5500 ) );

        // Setting the vertical position
        xpropertyset.setPropertyValue( "VertOrientPosition",
        new Integer( 4200 ) );

        // Setting the width
        xpropertyset.setPropertyValue( "Width", new Integer( 4400 ) );

        // Setting the height
        xpropertyset.setPropertyValue( "Height", new Integer( 4000 ) );
      } catch ( Exception exception ) {
        System.out.println( "Couldn't set property 'GraphicURL'" );
        exception.printStackTrace( printwriterLog );
      }

      xcomponentcontext = null;

      System.exit(0);
    }
    catch( Exception exception ) {
      System.err.println( exception );
    }
  }
}
