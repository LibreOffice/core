//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
// Name             : SCalc
//***************************************************************************
//
// comment:           Step 1: connect to the office an get the MSF
//                    Step 2: open an empty calc document
//                    Step 3: create cell styles
//                    Step 4: get the sheet an insert some data
//                    Step 5: apply the created cell syles
//                    Step 6: insert a 3D Chart
//
//***************************************************************************
// date                 : Tue August 22  2000
//
//***************************************************************************

// base interface
import com.sun.star.uno.XInterface;

// access the implementations via names
import com.sun.star.comp.servicemanager.ServiceManager;


import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import com.sun.star.uno.XComponentContext;


// staroffice interfaces to provide desktop and componentloader
// and components i.e. spreadsheets, writerdocs etc.
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XComponentLoader;

// additional classes required
import com.sun.star.sheet.*;
import com.sun.star.container.*;
import com.sun.star.table.*;
import com.sun.star.beans.*;
import com.sun.star.style.*;
import com.sun.star.lang.*;
import com.sun.star.text.*;
import com.sun.star.chart.*;
import com.sun.star.document.*;
import com.sun.star.awt.Rectangle;

public class SCalc  {

    public static void main(String args[]) {

        //oooooooooooooooooooooooooooStep 1oooooooooooooooooooooooooooooooooooooooooo
        // connect to the office an get the MultiServiceFactory
        // this is necessary to create instances of Services
        //***************************************************************************
        String sConnectionString = "uno:socket,host=localhost,port=8100;urp;StarOffice.NamingService";

        // It is possible to use a different connection string, passed as argument
        if ( args.length == 1 ) {
            sConnectionString = args[0];
        }

        XMultiServiceFactory xMSF = null;
        XSpreadsheetDocument myDoc = null;
        XCell oCell = null;


        // create connection(s) and get multiservicefactory


        // create connection(s) and get multiservicefactory
        System.out.println( "getting MultiServiceFactory" );

        try {
            xMSF = connect( sConnectionString );
        } catch( com.sun.star.uno.RuntimeException Ex ) {
            System.out.println( "Couldn't get MSF"+ Ex.getMessage() );
            return;
        } catch( Exception Ex ) {
        }

        //***************************************************************************

        //oooooooooooooooooooooooooooStep 2oooooooooooooooooooooooooooooooooooooooooo
        // open an empty document. In this case it's a calc document.
        // For this purpose an instance of com.sun.star.frame.Desktop
        // is created. It's interface XDesktop provides the XComponentLoader,
        // which is used to open the document via loadComponentFromURL
        //***************************************************************************


        //Open document

        //Calc

        System.out.println("Opening an empty Calc document");
        myDoc = openCalc(xMSF);

        //***************************************************************************


        //oooooooooooooooooooooooooooStep 3oooooooooooooooooooooooooooooooooooooooooo
        // create cell styles.
        // For this purpose get the StyleFamiliesSupplier and the the familiy
        // CellStyle. Create an instance of com.sun.star.style.CellStyle and
        // add it to the family. Now change some properties
        //***************************************************************************

        try {
            XStyleFamiliesSupplier xSFS = (XStyleFamiliesSupplier) UnoRuntime.queryInterface(XStyleFamiliesSupplier.class, myDoc);
            XNameAccess xSF = (XNameAccess) xSFS.getStyleFamilies();
            XNameAccess xCS = (XNameAccess) UnoRuntime.queryInterface(
                XNameAccess.class, xSF.getByName("CellStyles"));
            XMultiServiceFactory oDocMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                XMultiServiceFactory.class, myDoc );
            XNameContainer oStyleFamilyNameContainer = (XNameContainer) UnoRuntime.queryInterface(
                XNameContainer.class, xCS);
            XInterface oInt1 = (XInterface) oDocMSF.createInstance("com.sun.star.style.CellStyle");
            oStyleFamilyNameContainer.insertByName("My Style", oInt1);
            XPropertySet oCPS1 = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, oInt1 );
            oCPS1.setPropertyValue("IsCellBackgroundTransparent", new Boolean(false));
            oCPS1.setPropertyValue("CellBackColor",new Integer(6710932));
            oCPS1.setPropertyValue("CharColor",new Integer(16777215));
            XInterface oInt2 = (XInterface) oDocMSF.createInstance("com.sun.star.style.CellStyle");
            oStyleFamilyNameContainer.insertByName("My Style2", oInt2);
            XPropertySet oCPS2 = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, oInt2 );
            oCPS2.setPropertyValue("IsCellBackgroundTransparent", new Boolean(false));
            oCPS2.setPropertyValue("CellBackColor",new Integer(13421823));
        } catch (Exception e) {
        }

        //***************************************************************************

        //oooooooooooooooooooooooooooStep 4oooooooooooooooooooooooooooooooooooooooooo
        // get the sheet an insert some data.
        // Get the sheets from the document and then the first from this container.
        // Now some data can be inserted. For this purpose get a Cell via
        // getCellByPosition and insert into this cell via setValue() (for floats)
        // or setFormula() for formulas and Strings
        //***************************************************************************


        XSpreadsheet oSheet=null;

        try {
            System.out.println("Getting spreadsheet") ;
            XSpreadsheets oSheets = myDoc.getSheets() ;
            XIndexAccess oIndexSheets = (XIndexAccess) UnoRuntime.queryInterface(
                XIndexAccess.class, oSheets);
            oSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                XSpreadsheet.class, oIndexSheets.getByIndex(0));

        } catch (Exception e) {
            System.out.println("Couldn't get Sheet " +e);
        }



        System.out.println("Creating the Header") ;

        insertIntoCell(1,0,"JAN",oSheet,"");
        insertIntoCell(2,0,"FEB",oSheet,"");
        insertIntoCell(3,0,"MAR",oSheet,"");
        insertIntoCell(4,0,"APR",oSheet,"");
        insertIntoCell(5,0,"MAI",oSheet,"");
        insertIntoCell(6,0,"JUN",oSheet,"");
        insertIntoCell(7,0,"JUL",oSheet,"");
        insertIntoCell(8,0,"AUG",oSheet,"");
        insertIntoCell(9,0,"SEP",oSheet,"");
        insertIntoCell(10,0,"OCT",oSheet,"");
        insertIntoCell(11,0,"NOV",oSheet,"");
        insertIntoCell(12,0,"DEC",oSheet,"");
        insertIntoCell(13,0,"SUM",oSheet,"");


        System.out.println("Fill the lines");

        insertIntoCell(0,1,"Smith",oSheet,"");
        insertIntoCell(1,1,"42",oSheet,"V");
        insertIntoCell(2,1,"58.9",oSheet,"V");
        insertIntoCell(3,1,"-66.5",oSheet,"V");
        insertIntoCell(4,1,"43.4",oSheet,"V");
        insertIntoCell(5,1,"44.5",oSheet,"V");
        insertIntoCell(6,1,"45.3",oSheet,"V");
        insertIntoCell(7,1,"-67.3",oSheet,"V");
        insertIntoCell(8,1,"30.5",oSheet,"V");
        insertIntoCell(9,1,"23.2",oSheet,"V");
        insertIntoCell(10,1,"-97.3",oSheet,"V");
        insertIntoCell(11,1,"22.4",oSheet,"V");
        insertIntoCell(12,1,"23.5",oSheet,"V");
        insertIntoCell(13,1,"=SUM(B2:M2)",oSheet,"");


        insertIntoCell(0,2,"Jones",oSheet,"");
        insertIntoCell(1,2,"21",oSheet,"V");
        insertIntoCell(2,2,"40.9",oSheet,"V");
        insertIntoCell(3,2,"-57.5",oSheet,"V");
        insertIntoCell(4,2,"-23.4",oSheet,"V");
        insertIntoCell(5,2,"34.5",oSheet,"V");
        insertIntoCell(6,2,"59.3",oSheet,"V");
        insertIntoCell(7,2,"27.3",oSheet,"V");
        insertIntoCell(8,2,"-38.5",oSheet,"V");
        insertIntoCell(9,2,"43.2",oSheet,"V");
        insertIntoCell(10,2,"57.3",oSheet,"V");
        insertIntoCell(11,2,"25.4",oSheet,"V");
        insertIntoCell(12,2,"28.5",oSheet,"V");
        insertIntoCell(13,2,"=SUM(B3:M3)",oSheet,"");

        insertIntoCell(0,3,"Brown",oSheet,"");
        insertIntoCell(1,3,"31.45",oSheet,"V");
        insertIntoCell(2,3,"-20.9",oSheet,"V");
        insertIntoCell(3,3,"-117.5",oSheet,"V");
        insertIntoCell(4,3,"23.4",oSheet,"V");
        insertIntoCell(5,3,"-114.5",oSheet,"V");
        insertIntoCell(6,3,"115.3",oSheet,"V");
        insertIntoCell(7,3,"-171.3",oSheet,"V");
        insertIntoCell(8,3,"89.5",oSheet,"V");
        insertIntoCell(9,3,"41.2",oSheet,"V");
        insertIntoCell(10,3,"71.3",oSheet,"V");
        insertIntoCell(11,3,"25.4",oSheet,"V");
        insertIntoCell(12,3,"38.5",oSheet,"V");
        insertIntoCell(13,3,"=SUM(A4:L4)",oSheet,"");

        //***************************************************************************

        //oooooooooooooooooooooooooooStep 5oooooooooooooooooooooooooooooooooooooooooo
        // apply the created cell style.
        // For this purpose get the PropertySet of the Cell and change the
        // property CellStyle to the appropriate value.
        //***************************************************************************

        // change backcolor
        chgbColor( 1 , 0, 13, 0, "My Style", oSheet );
        chgbColor( 0 , 1, 0, 3, "My Style", oSheet );
        chgbColor( 1 , 1, 13, 3, "My Style2", oSheet );

        //***************************************************************************

        //oooooooooooooooooooooooooooStep 6oooooooooooooooooooooooooooooooooooooooooo
        // insert a 3D chart.
        // get the CellRange which holds the data for the chart and its RangeAddress
        // get the TableChartSupplier from the sheet and then the TableCharts from it.
        // add a new chart based on the data to the TableCharts.
        // get the ChartDocument, which provide the Diagramm. Change the properties
        // Dim3D (3 dimension) and String (the title) of the diagramm.
        //***************************************************************************

        // insert a chart

        Rectangle oRect = new Rectangle();
        oRect.X = 500;
        oRect.Y = 3000;
        oRect.Width = 25000;
        oRect.Height = 11000;

        XCellRange oRange = (XCellRange)UnoRuntime.queryInterface(XCellRange.class, oSheet);
        XCellRange myRange = oRange.getCellRangeByName("A1:N4");
        XCellRangeAddressable oRangeAddr = (XCellRangeAddressable)UnoRuntime.queryInterface(
            XCellRangeAddressable.class, myRange);
        CellRangeAddress myAddr = oRangeAddr.getRangeAddress();

        CellRangeAddress[] oAddr = new CellRangeAddress[1];
        oAddr[0] = myAddr;
        XTableChartsSupplier oSupp = (XTableChartsSupplier)UnoRuntime.queryInterface(
            XTableChartsSupplier.class, oSheet);

        XTableChart oChart = null;

        System.out.println("Insert Chart");

        XTableCharts oCharts = oSupp.getCharts();
        oCharts.addNewByName("Example", oRect, oAddr, true, true);

        // get the diagramm and Change some of the properties

        try {
            oChart = (XTableChart) (UnoRuntime.queryInterface(
                XTableChart.class, ((XNameAccess) UnoRuntime.queryInterface(
                                        XNameAccess.class, oCharts)).getByName("Example")));
            XEmbeddedObjectSupplier oEOS = (XEmbeddedObjectSupplier) UnoRuntime.queryInterface(
                XEmbeddedObjectSupplier.class, oChart);
            XInterface oInt = oEOS.getEmbeddedObject();
            XChartDocument xChart = (XChartDocument) UnoRuntime.queryInterface(XChartDocument.class,oInt);
            XDiagram oDiag = (XDiagram) xChart.getDiagram();
            System.out.println("Change Diagramm to 3D");
            XPropertySet oCPS = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, oDiag );
            oCPS.setPropertyValue("Dim3D", new Boolean(true));
            System.out.println("Change the title");
            Thread.sleep(200);
            XPropertySet oTPS = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, xChart.getTitle() );
            oTPS.setPropertyValue("String","The new title");
            //oDiag.Dim3D();
        } catch (Exception e){
            System.out.println("Changin Properties failed "+e);
        }



        System.out.println("done");

        System.exit(0);


    } // finish method main

    public static XMultiServiceFactory connect( String connectStr )
    throws com.sun.star.uno.Exception,
    com.sun.star.uno.RuntimeException, Exception {

        // Get component context
        XComponentContext xcomponentcontext =
        com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(
        null );

        // initial serviceManager
        XMultiComponentFactory xLocalServiceManager =
        xcomponentcontext.getServiceManager();

        // create a connector, so that it can contact the office
        Object  xUrlResolver  = xLocalServiceManager.createInstanceWithContext(
        "com.sun.star.bridge.UnoUrlResolver", xcomponentcontext );
        XUnoUrlResolver urlResolver = (XUnoUrlResolver)UnoRuntime.queryInterface(
            XUnoUrlResolver.class, xUrlResolver );

        Object rInitialObject = urlResolver.resolve( connectStr );

        XNamingService rName = (XNamingService)UnoRuntime.queryInterface(
            XNamingService.class, rInitialObject );

        XMultiServiceFactory xMSF = null;
        if( rName != null ) {
            System.err.println( "got the remote naming service !" );
            Object rXsmgr = rName.getRegisteredObject("StarOffice.ServiceManager" );

            xMSF = (XMultiServiceFactory)
            UnoRuntime.queryInterface( XMultiServiceFactory.class, rXsmgr );
        }

        return ( xMSF );
    }


    public static XSpreadsheetDocument openCalc(XMultiServiceFactory oMSF) {

        //define variables
        XInterface oInterface;
        XDesktop oDesktop;
        XComponentLoader oCLoader;
        XSpreadsheetDocument oDoc = null;
        XComponent aDoc = null;

        try {

            oInterface = (XInterface) oMSF.createInstance( "com.sun.star.frame.Desktop" );
            oDesktop = ( XDesktop ) UnoRuntime.queryInterface( XDesktop.class, oInterface );
            oCLoader = ( XComponentLoader ) UnoRuntime.queryInterface( XComponentLoader.class, oDesktop );
            PropertyValue [] szEmptyArgs = new PropertyValue [0];
            String doc = "private:factory/scalc";
            aDoc = oCLoader.loadComponentFromURL(doc, "_blank", 0, szEmptyArgs );
            oDoc = (XSpreadsheetDocument) UnoRuntime.queryInterface(XSpreadsheetDocument.class, aDoc);

        } // end of try

        catch(Exception e){

            System.out.println(" Exception " + e);

        } // end of catch


        return oDoc;
    }//end of openCalc


    public static void insertIntoCell(int CellX, int CellY, String theValue, XSpreadsheet TT1, String flag) {

        XCell oCell = null;

        try {
            oCell = TT1.getCellByPosition(CellX, CellY);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
            System.out.println("Could not get Cell");
        }
        if (flag.equals("V")) {oCell.setValue((new Float(theValue)).floatValue());}
        else {oCell.setFormula(theValue);}

    } // end of insertIntoCell

    public static void chgbColor( int x1, int y1, int x2, int y2, String template, XSpreadsheet TT ) {
        XCellRange xCR = null;
        try {
            xCR = TT.getCellRangeByPosition(x1,y1,x2,y2);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
            System.out.println("Could not get CellRange");
        }
        XPropertySet oCPS = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, xCR );

        try {
            oCPS.setPropertyValue("CellStyle", template);
        } catch (Exception e) {

            System.out.println("Can't change colors chgbColor" + e);

        }


    }

} // finish class SCalc
