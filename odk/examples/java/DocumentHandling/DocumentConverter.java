import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XStorable;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;

import java.io.File;
import java.io.FileFilter;


/** The class <CODE>DocumentConverter</CODE> allows you to convert all documents in
 * a given directory and in its subdirectories to a given type. A converted
 * document will be created in the same directory as the origin document.
 *
 */
public class DocumentConverter {
  /** Containing the loaded documents
   */
  static XComponentLoader xcomponentloader = null;
  /** Containing the given type to convert to
   */
  static String stringConvertType = "";
  /** Containing the given extension
   */
  static String stringExtension = "";
  /** Containing the current file or directory
   */
  static String indent = "";

  /** Traversing the given directory recursively and converting their files to the
   * favoured type if possible
   * @param fileDirectory Containing the directory
   */
  static void traverse( File fileDirectory ) {
    // Testing, if the file is a directory, and if so, it throws an exception
    if ( !fileDirectory.isDirectory() ) {
      throw new IllegalArgumentException(
      "not a directory: " + fileDirectory.getName()
      );
    }

    System.out.println(indent + "[" + fileDirectory.getName() + "]");
    indent += "  ";

    // Getting all files and directories in the current directory
    File[] entries = fileDirectory.listFiles(
    new FileFilter() {
      public boolean accept( File pathname ) {
        return true;
      }
    }
    );

    // Iterating for each file and directory
    for ( int i = 0; i < entries.length; ++i ) {
      // Testing, if the entry in the list is a directory
      if ( entries[ i ].isDirectory() ) {
        // Recursive call for the new directory
        traverse( entries[ i ] );
      } else {
        // Converting the document to the favoured type
        try {
          // Composing the URL by replacing all backslashs
          String stringUrl = "file:///"
          + entries[ i ].getAbsolutePath().replace( '\\', '/' );

          // Loading the wanted document
          Object objectDocumentToStore =
          DocumentConverter.xcomponentloader.loadComponentFromURL(
          stringUrl, "_blank", 0, new PropertyValue[0] );

          // Getting an object that will offer a simple way to store a document to a URL.
          XStorable xstorable =
          ( XStorable ) UnoRuntime.queryInterface( XStorable.class,
          objectDocumentToStore );

          // Preparing properties for converting the document
          PropertyValue propertyvalue[] = new PropertyValue[ 2 ];
          // Setting the flag for overwriting
          propertyvalue[ 0 ] = new PropertyValue();
          propertyvalue[ 0 ].Name = "Overwrite";
          propertyvalue[ 0 ].Value = new Boolean(true);
          // Setting the filter name
          propertyvalue[ 1 ] = new PropertyValue();
          propertyvalue[ 1 ].Name = "FilterName";
          propertyvalue[ 1 ].Value = DocumentConverter.stringConvertType;

          // Appending the favoured extension to the origin document name
          int index = stringUrl.lastIndexOf('.');
          if (index >= 0)
              stringUrl = stringUrl.substring(0, index + 1);
          stringUrl = stringUrl + DocumentConverter.stringExtension;

          // Storing and converting the document
          xstorable.storeAsURL( stringUrl, propertyvalue );

          // Getting the method dispose() for closing the document
          XComponent xcomponent =
          ( XComponent ) UnoRuntime.queryInterface( XComponent.class,
          xstorable );

          // Closing the converted document
          xcomponent.dispose();
        }
        catch( Exception exception ) {
          exception.printStackTrace();
        }

        System.out.println(indent + entries[ i ].getName());
      }
    }

    indent = indent.substring(2);
  }

  /** Connecting to the office with the component UnoUrlResolver and calling the
   * static method traverse
   * @param args The array of the type String contains the directory, in which all files should be
   * converted, the favoured converting type and the wanted extension
   */
  public static void main( String args[] ) {
    try {
      String sConnectionString = "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager";

      if ( args.length < 3 ) {
        System.out.println(
        "usage: java -classpath <see make> " +
        "DocumentConverter \"<directory to convert>\"" +
        " \"<type to convert to>\" \"<extension>\" [\"<connection>\"]" );
        System.out.println( "\ne.g.:" );
        System.out.println(
        "java -classpath <see make> " +
        "DocumentConverter" + " \"c:/myoffice\" \"swriter: MS Word 97\" \"doc\"" );
        System.exit(1);
      }

      // It is possible to use a different connection string, passed as argument
      if ( args.length == 4 ) {
            sConnectionString = args[3];
      }

      /* Bootstraps a component context with the jurt base components
         registered. Component context to be granted to a component for running.
         Arbitrary values can be retrieved from the context. */
      XComponentContext xComponentContext =
      com.sun.star.comp.helper.Bootstrap.createInitialComponentContext( null );

      /* Gets the service manager instance to be used (or null). This method has
         been added for convenience, because the service manager is a often used
         object. */
      XMultiComponentFactory xMultiComponentFactory =
      xComponentContext.getServiceManager();

      /* Creates an instance of the component UnoUrlResolver which
         supports the services specified by the factory. */
      Object objectUrlResolver = xMultiComponentFactory.createInstanceWithContext(
      "com.sun.star.bridge.UnoUrlResolver", xComponentContext );

      // Create a new url resolver
      XUnoUrlResolver xurlresolver = ( XUnoUrlResolver )
      UnoRuntime.queryInterface( XUnoUrlResolver.class,
      objectUrlResolver );

      // Resolves an object that is specified as follow:
      // uno:<connection description>;<protocol description>;<initial object name>
      Object objectInitial = xurlresolver.resolve( sConnectionString );

      // Create a service manager from the initial object
      xMultiComponentFactory = ( XMultiComponentFactory )
      UnoRuntime.queryInterface( XMultiComponentFactory.class, objectInitial );

      // Query for the XPropertySet interface.
      XPropertySet xpropertysetMultiComponentFactory = ( XPropertySet )
      UnoRuntime.queryInterface( XPropertySet.class, xMultiComponentFactory );

      // Get the default context from the office server.
      Object objectDefaultContext =
      xpropertysetMultiComponentFactory.getPropertyValue( "DefaultContext" );

      // Query for the interface XComponentContext.
      xComponentContext = ( XComponentContext ) UnoRuntime.queryInterface(
      XComponentContext.class, objectDefaultContext );

      /* A desktop environment contains tasks with one or more
         frames in which components can be loaded. Desktop is the
         environment for components which can instanciate within
         frames. */
      xcomponentloader = ( XComponentLoader )
      UnoRuntime.queryInterface( XComponentLoader.class,
      xMultiComponentFactory.createInstanceWithContext(
      "com.sun.star.frame.Desktop", xComponentContext ) );

      // Getting the given starting directory
      File file = new File(args[ 0 ]);

      // Getting the given type to convert to
      stringConvertType = args[ 1 ];

      // Getting the given extension that should be appended to the origin document
      stringExtension = args[ 2 ];

      // Starting the conversion of documents in the given directory and subdirectories
      traverse( file );

      System.exit(0);
    }
    catch( Exception exception ) {
      System.err.println( exception );
    }
  }
}
