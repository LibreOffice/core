// Lotus Notes Domino API
import lotus.domino.NotesThread;
import lotus.domino.Session;
import lotus.domino.Database;
import lotus.domino.DocumentCollection;
import lotus.domino.Document;
import lotus.domino.NotesFactory;

// OpenOffice.org API
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.beans.PropertyValue;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.table.XCell;

/** This class creates an OpenOffice.org Calc spreadsheet document and fills it
 * with existing values of documents from a Lotus Notes database.
 */
public class NotesAccess implements Runnable {
  /** Host server of the Domino Directory.
   */
  String stringHost = null;

  /** User in the host's Domino Directory.
   */
  String stringUser = "";

  /** Password for the user in the host's Domino Directory.
   */
  String stringPassword = "";

  /** Reading the arguments and constructing the thread.
   * @param argv Holding values for the host, user, and the password of the user.
   */
  public static void main( String argv[] ) {
    Thread thread;

    if( argv.length < 1 ) {
      // Initializing.
      NotesAccess notesaccess = new NotesAccess();

      // Allowing only local calls to the Domino classes.
      thread = new NotesThread( ( Runnable ) notesaccess );
    }
    else {
      // Extracting the host, user, and password.
      NotesAccess notesaccess = new NotesAccess( argv );

      // Allowing remote calls to the Domino classes.
      thread = new Thread( ( Runnable ) notesaccess );
    }

    // Starting the thread.
    thread.start();
  }

  /** The constructor extracts the values for the host, user, and password given
   * as arguments.
   * @param argv Holding values for the host, user, and the password of the user.
   */
  public NotesAccess( String argv[] ) {
    // Getting the host.
    stringHost = argv[ 0 ];

    if ( argv.length >= 2 ) {
      // Getting the user.
      stringUser = argv[ 1 ];
    }
    if ( argv.length >= 3 ) {
      // Getting the password for the user.
      stringPassword = argv[ 2 ];
    }
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
      /* Bootstraps a servicemanager with the jurt base components
         registered */
      XMultiServiceFactory xmultiservicefactory =
      com.sun.star.comp.helper.Bootstrap.createSimpleServiceManager();

      /* Creates an instance of the component UnoUrlResolver which
         supports the services specified by the factory. */
      Object objectUrlResolver = xmultiservicefactory.createInstance(
      "com.sun.star.bridge.UnoUrlResolver" );

      // Create a new url resolver
      XUnoUrlResolver xurlresolver = ( XUnoUrlResolver )
      UnoRuntime.queryInterface( XUnoUrlResolver.class,
      objectUrlResolver );

      // Resolves an object that is specified as follow:
      // uno:<connection description>;<protocol description>;<initial object name>
      Object objectInitial = xurlresolver.resolve(
      "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager" );

      // Create a service manager from the initial object
      xmultiservicefactory = ( XMultiServiceFactory )
      UnoRuntime.queryInterface( XMultiServiceFactory.class,
      objectInitial );

      /* A desktop environment contains tasks with one or more
         frames in which components can be loaded. Desktop is the
         environment for components which can instanciate within
         frames. */
      XComponentLoader xcomponentloader = ( XComponentLoader )
      UnoRuntime.queryInterface( XComponentLoader.class,
      xmultiservicefactory.createInstance(
      "com.sun.star.frame.Desktop" ) );

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
      XSpreadsheet xspreadsheet = ( XSpreadsheet ) xindexaccess.getByIndex( 0 );

      Session session;
      if ( stringHost != null ) {
        // Creating a Notes session for remote calls to the Domino classes.
        session = NotesFactory.createSession( stringHost, stringUser,
        stringPassword );
      }
      else {
        // Creating a Notes session for only local calls to the Domino classes.
        session = NotesFactory.createSession();
      }

      // Getting the specified Notes database.
      Database database = session.getDatabase( "",
      "F:\\odk3.0.0\\examples\\java\\NotesAccess\\Stocks.nsf" );

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
