import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.beans.PropertyValue;
import com.sun.star.chart.XChartDocument;
import com.sun.star.document.XEmbeddedObjectSupplier;
import com.sun.star.uno.XInterface;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.XTableChart;
import com.sun.star.table.XTableCharts;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.table.XTableChartsSupplier;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.chart.XDiagram;
import com.sun.star.beans.XPropertySet;
import com.sun.star.awt.Rectangle;


/** This class loads an OpenOffice.org Calc document and changes the type of the
 * embedded chart.
 * @author Bertram Nolte
 */
public class ChartTypeChange {

    /** Default connection string
     */
    private String sConnectionString = "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager";

    /** Table chart, which type will be changed.
     */
    private XTableChart xtablechart = null;

    /** Service factory
     */
    private XMultiComponentFactory xmulticomponentfactory = null;

    /** Component context
     */
    private XComponentContext xcomponentcontext = null;

    /** Beginning of the program.
     * @param args No arguments will be passed to the class.
     */
    public static void main(String args[]) {
        try {
            ChartTypeChange charttypechange = new ChartTypeChange( args );

            // Double array holding all values the chart should be based on.
            String[][] stringValues = {
                { "", "Jan", "Feb", "Mar", "Apr", "Mai" },
                { "Profit", "12.3", "43.2", "5.1", "76", "56.8" },
                { "Rival in business", "12.2", "12.6", "17.7", "20.4", "100" },
            };

            // Create the chart with
            charttypechange.getChart( stringValues );

            String[] stringChartType = {
                "com.sun.star.chart.LineDiagram",
                "com.sun.star.chart.BarDiagram",
                "com.sun.star.chart.PieDiagram",
                "com.sun.star.chart.NetDiagram",
                "com.sun.star.chart.XYDiagram",
                "com.sun.star.chart.StockDiagram",
                "com.sun.star.chart.AreaDiagram"
            };

            for ( int intCounter = 0; intCounter < stringChartType.length;
            intCounter++ ) {
                charttypechange.changeChartType( stringChartType[ intCounter ],
                false );
                Thread.sleep( 1200 );
            }

            System.exit(0);
        }
        catch( Exception exception ) {
            System.err.println( exception );
        }
    }

    /** The constructor connects to the OpenOffice.org.
     * @param args Parameters for this constructor (connection string).
     * @throws Exception All exceptions are thrown from this method.
     */
    public ChartTypeChange( String[] args )
    throws Exception {
        // It is possible to use a different connection string, passed as argument
        if ( args.length == 1 ) {
            sConnectionString = args[0];
        }

      /* Bootstraps a component context with the jurt base components
         registered. Component context to be granted to a component for running.
         Arbitrary values can be retrieved from the context. */
        xcomponentcontext =
        com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(
        null );

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
        UnoRuntime.queryInterface( XMultiComponentFactory.class,
        objectInitial );

        // Query for the XPropertySet interface.
        XPropertySet xpropertysetMultiComponentFactory = ( XPropertySet )
        UnoRuntime.queryInterface( XPropertySet.class, xmulticomponentfactory );

        // Get the default context from the office server.
        Object objectDefaultContext =
        xpropertysetMultiComponentFactory.getPropertyValue( "DefaultContext" );

        // Query for the interface XComponentContext.
        xcomponentcontext = ( XComponentContext ) UnoRuntime.queryInterface(
        XComponentContext.class, objectDefaultContext );
    }

    /** This method will change the type of a specified chart.
     * @param stringType The chart will be converted to this type.
     * @param booleanIs3D If the chart should be displayed in 3D this parameter should be set to true.
     * @throws Exception All exceptions are thrown from this method.
     */
    public void changeChartType( String stringType, boolean booleanIs3D )
    throws Exception {
        XEmbeddedObjectSupplier xembeddedobjectsupplier = (XEmbeddedObjectSupplier)
            UnoRuntime.queryInterface(XEmbeddedObjectSupplier.class, xtablechart);
        XInterface xinterface = xembeddedobjectsupplier.getEmbeddedObject();

        XChartDocument xchartdocument = (XChartDocument)UnoRuntime.queryInterface(
            XChartDocument.class, xinterface);
        XDiagram xdiagram = (XDiagram) xchartdocument.getDiagram();
        XMultiServiceFactory xmultiservicefactory = (XMultiServiceFactory)
            UnoRuntime.queryInterface( XMultiServiceFactory.class, xchartdocument );
        Object object = xmultiservicefactory.createInstance( stringType );
        xdiagram = (XDiagram) UnoRuntime.queryInterface(XDiagram.class,object);

        XPropertySet xpropertyset = ( XPropertySet ) UnoRuntime.queryInterface(
            XPropertySet.class, xdiagram );
        xpropertyset.setPropertyValue( "Dim3D", new Boolean( booleanIs3D ) );

        xchartdocument.setDiagram(xdiagram);
    }

    /** Loading an OpenOffice.org Calc document and getting a chart by name.
     * @param stringFileName Name of the OpenOffice.org Calc document which should be loaded.
     * @param stringChartName Name of the chart which should get a new chart type.
     */
    public void getChart( String stringFileName, String stringChartName ) {
        try {
            XMultiComponentFactory xmulticomponentfactory =
            xcomponentcontext.getServiceManager();

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
            "file:///" + stringFileName, "_blank", 0,
            new PropertyValue[0] );

            // Query for the interface XSpreadsheetDocument
            XSpreadsheetDocument xspreadsheetdocument = ( XSpreadsheetDocument )
                UnoRuntime.queryInterface( XSpreadsheetDocument.class, xcomponent );

            XSpreadsheets xspreadsheets = xspreadsheetdocument.getSheets() ;

            XIndexAccess xindexaccess = (XIndexAccess)
                UnoRuntime.queryInterface(XIndexAccess.class, xspreadsheets );

            XSpreadsheet xspreadsheet = (XSpreadsheet) UnoRuntime.queryInterface(
                XSpreadsheet.class, xindexaccess.getByIndex(0));

            XTableChartsSupplier xtablechartssupplier = ( XTableChartsSupplier )
                UnoRuntime.queryInterface( XTableChartsSupplier.class, xspreadsheet );

            xindexaccess = (XIndexAccess) UnoRuntime.queryInterface(
                XIndexAccess.class, xtablechartssupplier.getCharts() );

            this.xtablechart = (XTableChart) UnoRuntime.queryInterface(
                XTableChart.class,  xindexaccess.getByIndex( 0 ) );
        }
        catch( Exception exception ) {
            System.err.println( exception );
        }
    }

    /** Creating an empty OpenOffice.org Calc document, inserting data, and getting a
     * chart by name.
     * @param stringValues Double array with the values for the chart.
     */
    public void getChart( String[][] stringValues ) {
        try {
            XMultiComponentFactory xmulticomponentfactory =
            xcomponentcontext.getServiceManager();
      /* A desktop environment contains tasks with one or more
         frames in which components can be loaded. Desktop is the
         environment for components which can instanciate within
         frames. */
            XComponentLoader xcomponentloader = ( XComponentLoader )
                UnoRuntime.queryInterface( XComponentLoader.class,
                                           xmulticomponentfactory.createInstanceWithContext(
                                               "com.sun.star.frame.Desktop", xcomponentcontext ) );

            // Create an empty calc document, which will be automaticly displayed
            XComponent xcomponent = xcomponentloader.loadComponentFromURL(
            "private:factory/scalc", "_blank", 0,
            new PropertyValue[0] );

            // Query for the interface XSpreadsheetDocument
            XSpreadsheetDocument xspreadsheetdocument = ( XSpreadsheetDocument )
                UnoRuntime.queryInterface( XSpreadsheetDocument.class, xcomponent );

            // Get all sheets of the spreadsheet document.
            XSpreadsheets xspreadsheets = xspreadsheetdocument.getSheets() ;

            // Get the index of the spreadsheet document.
            XIndexAccess xindexaccess = (XIndexAccess) UnoRuntime.queryInterface(
                XIndexAccess.class, xspreadsheets );

            // Get the first spreadsheet.
            XSpreadsheet xspreadsheet = (XSpreadsheet) UnoRuntime.queryInterface(
                XSpreadsheet.class, xindexaccess.getByIndex(0));

            // The double array will written to the spreadsheet
            for ( int intY = 0; intY < stringValues.length; intY++ ) {
                for ( int intX = 0; intX < stringValues[ intY ].length;
                intX++ ) {
                    // Insert the value to the cell, specified by intY and intX.
                    this.insertIntoCell( intY, intX,
                    stringValues[ intY ][ intX ], xspreadsheet, "" );
                }
            }

            // Create a rectangle, which holds the size of the chart.
            Rectangle rectangle = new Rectangle();
            rectangle.X = 500;
            rectangle.Y = 3000;
            rectangle.Width = 25000;
            rectangle.Height = 11000;

            // Get the cell range of the spreadsheet.
            XCellRange xcellrange = ( XCellRange ) UnoRuntime.queryInterface(
                XCellRange.class, xspreadsheet );

            // Create the Unicode of the character for the column name.
            char charRectangle = ( char ) ( 65 + stringValues.length - 1 );

            // Get maximum length all rows in the double array.
            int intMaximumWidthRow = 0;
            for ( int intRow = 0; intRow < stringValues.length; intRow++ ) {
                if ( stringValues[ intRow ].length > intMaximumWidthRow ) {
                    intMaximumWidthRow = stringValues[ intRow ].length;
                }
            }

            // Get the cell range of the written values.
            XCellRange xcellrangeChart = xcellrange.getCellRangeByName( "A1:" +
            charRectangle + intMaximumWidthRow );

            // Get the addressable cell range.
            XCellRangeAddressable xcellrangeaddressable =
            ( XCellRangeAddressable ) UnoRuntime.queryInterface(
                XCellRangeAddressable.class, xcellrangeChart );

            // Get the cell range address.
            CellRangeAddress cellrangeaddress = xcellrangeaddressable.getRangeAddress();

            // Create the cell range address for the chart.
            CellRangeAddress[] cellrangeaddressChart =
            new CellRangeAddress[ 1 ];
            cellrangeaddressChart[ 0 ] = cellrangeaddress;

            // Get the table charts supplier of the spreadsheet.
            XTableChartsSupplier xtablechartssupplier = ( XTableChartsSupplier )
                UnoRuntime.queryInterface( XTableChartsSupplier.class, xspreadsheet );

            // Get all table charts of the spreadsheet.
            XTableCharts xtablecharts = xtablechartssupplier.getCharts();

            // Create a table chart with all written values.
            xtablecharts.addNewByName( "Example", rectangle,
            cellrangeaddressChart, true, true );

            // Get the created table chart.
            this.xtablechart = ( XTableChart ) UnoRuntime.queryInterface(
                XTableChart.class, (( XNameAccess ) UnoRuntime.queryInterface(
                    XNameAccess.class, xtablecharts ) ).getByName( "Example" ));
        }
        catch( Exception exception ) {
            System.err.println( exception );
        }
    }

    /** Inserting a given value to a cell, that is specified by the parameters intX
     * and intY.
     * @param intX Column on the spreadsheet.
     * @param intY Row on the spreadsheet.
     * @param stringValue Value to be inserted to a cell.
     * @param xspreadsheet Spreadsheet of the cell, which will be changed.
     * @param stringFlag If the value of stringFlag is "V", the stringValue will be converted to the
     * float type. Otherwise the stringValue will be written as a formula.
     */
    public static void insertIntoCell( int intX, int intY, String stringValue,
    XSpreadsheet xspreadsheet, String stringFlag ) {
        XCell xcell = null;

        try {
            xcell = xspreadsheet.getCellByPosition( intX, intY );
        }
        catch ( com.sun.star.lang.IndexOutOfBoundsException exception ) {
            System.out.println( "Could not get cell." );
        }
        if ( stringFlag.equals( "V" ) ) {
            xcell.setValue( ( new Float( stringValue ) ).floatValue() );
        }
        else {
            xcell.setFormula( stringValue );
        }
    }
}
