// Lotus Notes Domino API
import lotus.domino.NotesThread;
import lotus.domino.Session;
import lotus.domino.Database;
import lotus.domino.DocumentCollection;
import lotus.domino.Document;
import lotus.domino.NotesFactory;

// OpenOffice.org API
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.table.XCell;

/** This class creates an OpenOffice.org Calc spreadsheet document and fills it
 * with existing values of documents from a Lotus Notes database.
 */
public class NotesAccess implements Runnable {
  /** Connection to the office.
   */
  static String sOfficeConnection = "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager";

  /** Host server of the Domino Directory.
   */
  static String stringHost = "";

  /** User in the host's Domino Directory.
   */
  static String stringUser = "";

  /** Password for the user in the host's Domino Directory.
   */
  static String stringPassword = "";

  /** Database with documents to get data from.
   */
  static String stringDatabase = "";

  /** Reading the arguments and constructing the thread.
   * @param argv Holding values for the host, user, and the password of the user.
   */
  public static void main( String args[] ) {
    Thread thread;

    if ( args.length < 4 ) {
      System.out.println(
      "usage: java -classpath .;<Office path>/program/classes/jurt.jar;" +
      "<Office path>/program/classes/ridl.jar;" +
      "<Office path>/program/classes/sandbox.jar;" +
      "<Office path>/program/classes/unoil.jar;" +
      "<Office path>/program/classes/juh.jar " +
      "NotesAccess \"<Domino Host>\" \"<User>\" " +
      "\"<Password>\" \"<Database>\" [\"<Connection>\"]" );
      System.out.println( "\ne.g.:" );
      System.out.println(
      "java -classpath .;d:/office60/program/classes/jurt.jar;" +
      "d:/office60/program/classes/ridl.jar;" +
      "d:/office60/program/classes/sandbox.jar;" +
      "d:/office60/program/classes/unoil.jar; " +
      "d:/office60/program/classes/juh.jar " +
      "NotesAccess \"\" \"\" \"\" \"Stocks.nsf\"" );
      System.exit( 1 );
    }

    // It is possible to use a different connection string, passed as argument
    if ( args.length == 2 ) {
        sOfficeConnection = args[1];
    }

    if ( !args[ 1 ].trim().equals( "" ) ) {
      stringHost = args[ 1 ].trim();
    }
    if ( !args[ 2 ].trim().equals( "" ) ) {
      stringUser = args[ 2 ].trim();
    }
    stringPassword = args[ 3 ].trim();
    stringDatabase = args[ 4 ].trim();

    if ( stringHost.equals( "" ) ) {
      // Initializing.
      NotesAccess notesaccess = new NotesAccess();

      // Allowing only local calls to the Domino classes.
      thread = new NotesThread( ( Runnable ) notesaccess );
    }
    else {
      // Extracting the host, user, and password.
      NotesAccess notesaccess = new NotesAccess();

      // Allowing remote calls to the Domino classes.
      thread = new Thread( ( Runnable ) notesaccess );
    }

    // Starting the thread.
    thread.start();
  }

  /** This is the default constructor without arguments.
   */
  public NotesAccess() {
  }

  /** Reading all documents from the given database and writing the data to
   * an OpenOffice.org Calc spreadsheet document.
   */
  public void run() {
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
      Object objectInitial = xurlresolver.resolve( sOfficeConnection );

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
      "private:factory/scalc", "_blank", 0,
      new PropertyValue[0] );

      // Querying for the interface XSpreadsheetDocument
      XSpreadsheetDocument xspreadsheetdocument =
      ( XSpreadsheetDocument ) UnoRuntime.queryInterface(
      XSpreadsheetDocument.class, xcomponent );

      // Getting all sheets from the spreadsheet document.
      XSpreadsheets xspreadsheets = xspreadsheetdocument.getSheets() ;

      // Querying for the interface XIndexAccess.
      XIndexAccess xindexaccess = ( XIndexAccess ) UnoRuntime.queryInterface(
      XIndexAccess.class, xspreadsheets );

      // Getting the first spreadsheet.
      XSpreadsheet xspreadsheet = ( XSpreadsheet ) UnoRuntime.queryInterface(
          XSpreadsheet.class, xindexaccess.getByIndex( 0 ));

      Session session;
      if ( !stringHost.equals( "" ) ) {
        // Creating a Notes session for remote calls to the Domino classes.
        session = NotesFactory.createSession( stringHost, stringUser,
        stringPassword );
      }
      else {
        // Creating a Notes session for only local calls to the Domino classes.
        session = NotesFactory.createSession();
      }

      // Getting the specified Notes database.
      Database database = session.getDatabase( "", stringDatabase );

      // Getting a collection of all documents from the database.
      DocumentCollection documentcollection = database.getAllDocuments();

      // Getting the first document from the database
      Document document = documentcollection.getFirstDocument();

      // Start to write to cells at this row.
      int intRowToStart = 0;

      // The current row.
      int intRow = intRowToStart;

      // The current column.
      int intColumn = 0;

      // Process all documents
      while ( document != null ) {
        // Getting the name of the stock.
        String stringName = document.getItemValueString( "Name" );

        // Inserting the name to a specified cell.
        insertIntoCell( intColumn, intRow, stringName, xspreadsheet, "" );

        // Getting the number of stocks.
        double intNumber = document.getItemValueInteger( "Number" );

        // Inserting the number of stocks to a specified cell.
        insertIntoCell( intColumn + 1, intRow, String.valueOf( intNumber ),
        xspreadsheet, "V" );

        // Getting current share price.
        double doubleSharePrice = document.getItemValueDouble( "SharePrice" );

        // Inserting the current share price to a specified cell.
        insertIntoCell( intColumn + 2, intRow, String.valueOf( doubleSharePrice ),
        xspreadsheet, "V" );

        // Inserting the total value.
        insertIntoCell( intColumn + 3, intRow, "=B"
        + String.valueOf( intRow + 1 ) + "*C" + String.valueOf( intRow + 1 ),
        xspreadsheet, "" );

        // Increasing the current row.
        intRow++;

        // Getting the next document from the collection.
        document = documentcollection.getNextDocument();
      }

      // Summing all specific amounts.
      insertIntoCell( intColumn + 3, intRow, "=sum(D"
      + String.valueOf( intRowToStart + 1 ) + ":D"
      + String.valueOf( intRow ),
      xspreadsheet, "" );

      xcomponentcontext = null;

      // Leaving the program.
      System.exit(0);
    }
    catch (Exception e) {
      e.printStackTrace();
    }
  }

  /** Inserting a value or formula to a cell defined by the row and column.
   * @param intCellX Row.
   * @param intCellY Column.
   * @param stringValue This value will be written to the cell.
   * @param xspreadsheet Write the value to the cells of this spreadsheet.
   * @param stringFlag If this string contains "V", the value will be written, otherwise the formula.
   */
  public static void insertIntoCell( int intCellX, int intCellY,
  String stringValue, XSpreadsheet xspreadsheet, String stringFlag ) {
    XCell xcell = null;

    try {
      xcell = xspreadsheet.getCellByPosition( intCellX, intCellY );
    } catch ( com.sun.star.lang.IndexOutOfBoundsException exception ) {
      System.out.println( "Could not get Cell" );
    }
    if ( stringFlag.equals( "V" )) {
      xcell.setValue( ( new Float( stringValue ) ).floatValue());
    }
    else {
      xcell.setFormula( stringValue );
    }
  }
}
