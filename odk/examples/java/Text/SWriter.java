//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
// Name             : SWriter
//***************************************************************************
//
// comment:           Step 1: connect to the office an get the MSF
//                    Step 2: open an empty text document
//                    Step 3: enter some text
//                    Step 4: insert a text table
//                    Step 5: insert colored text
//                    Step 6: insert a text frame
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
import com.sun.star.drawing.*;
import com.sun.star.awt.Size;

public class SWriter  {



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
        XTextDocument myDoc = null;


        // create connection(s) and get multiservicefactory
        System.out.println( "getting MultiServiceFactory" );

        try {
            xMSF = connect( sConnectionString );
        } catch( Exception Ex ) {
            System.out.println( "Couldn't get MSF"+ Ex );
            return;
        }

        //***************************************************************************


        //oooooooooooooooooooooooooooStep 2oooooooooooooooooooooooooooooooooooooooooo
        // open an empty document. In this case it's a writer document.
        // For this purpose an instance of com.sun.star.frame.Desktop
        // is created. It's interface XDesktop provides the XComponentLoader,
        // which is used to open the document via loadComponentFromURL
        //***************************************************************************


        //Open Writer document

        System.out.println("Opening an empty Writer document");
        myDoc = openWriter(xMSF);

        //***************************************************************************


        //oooooooooooooooooooooooooooStep 3oooooooooooooooooooooooooooooooooooooooooo
        // insert some text.
        // For this purpose get the Text-Object of the document an create the
        // cursor. Now it is possible to insert a text at the cursor-position
        // via insertString
        //***************************************************************************


        //getting the text object
        XText oText = myDoc.getText();

        //create a cursor object
        XTextCursor oCursor = oText.createTextCursor();

        //inserting some Text
        oText.insertString( oCursor, "The first line in the newly created text document.\n", false );


        //inserting a second line
        oText.insertString( oCursor, "Now we're in the second line\n", false );

        //***************************************************************************


        //oooooooooooooooooooooooooooStep 4oooooooooooooooooooooooooooooooooooooooooo
        // insert a text table.
        // For this purpose get MultiServiceFactory of the document, create an
        // instance of com.sun.star.text.TextTable and initialize it. Now it can
        // be inserted at the cursor position via insertTextContent.
        // After that some properties are changed and some data is inserted.
        //***************************************************************************

        //inserting a text table

        System.out.println("inserting a text table");

        //getting MSF of the document
        XMultiServiceFactory oDocMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
            XMultiServiceFactory.class, myDoc );

        //create instance of a text table
        XTextTable TT = null;

        try {
            Object oInt = oDocMSF.createInstance("com.sun.star.text.TextTable");
            TT = (XTextTable) UnoRuntime.queryInterface(XTextTable.class,oInt);
        } catch (Exception e) {
            System.out.println("Couldn't create instance "+ e);
        }

        //initialize the text table with 4 columns an 4 rows
        TT.initialize(4,4);

        XPropertySet aRow = null;

        //insert the table
        try {
            oText.insertTextContent(oCursor, TT, false);
            // get first Row
            XIndexAccess theRows = TT.getRows();
            aRow = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, theRows.getByIndex(0));

        } catch (Exception e) {
            System.out.println("Couldn't insert the table " + e);
        }


        // get the property set of the text table

        XPropertySet oTPS = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, TT );

        // Change the BackColor
        try {

            oTPS.setPropertyValue("BackTransparent", new Boolean(false));
            oTPS.setPropertyValue("BackColor",new Integer(13421823));
            aRow.setPropertyValue("BackTransparent", new Boolean(false));
            aRow.setPropertyValue("BackColor",new Integer(6710932));

        } catch (Exception e) {
            System.out.println("Couldn't change the color " + e);
        }


        // write Text in the Table headers
        System.out.println("write text in the table headers");

        insertIntoCell("A1","FirstColumn", TT);
        insertIntoCell("B1","SecondColumn", TT) ;
        insertIntoCell("C1","ThirdColumn", TT) ;
        insertIntoCell("D1","SUM", TT) ;


        //Insert Something in the text table
        System.out.println("Insert something in the text table");

        (TT.getCellByName("A2")).setValue(22.5);
        (TT.getCellByName("B2")).setValue(5615.3);
        (TT.getCellByName("C2")).setValue(-2315.7);
        (TT.getCellByName("D2")).setFormula("sum <A2:C2>");

        (TT.getCellByName("A3")).setValue(21.5);
        (TT.getCellByName("B3")).setValue(615.3);
        (TT.getCellByName("C3")).setValue(-315.7);
        (TT.getCellByName("D3")).setFormula("sum <A3:C3>");

        (TT.getCellByName("A4")).setValue(121.5);
        (TT.getCellByName("B4")).setValue(-615.3);
        (TT.getCellByName("C4")).setValue(415.7);
        (TT.getCellByName("D4")).setFormula("sum <A4:C4>");


        //***************************************************************************


        //oooooooooooooooooooooooooooStep 5oooooooooooooooooooooooooooooooooooooooooo
        // insert a colored text.
        // Get the propertySet of the cursor, change the CharColor and add a
        // shadow. Then insert the Text via InsertString at the cursor position.
        //***************************************************************************


        // get the property set of the cursor

        XPropertySet oCPS = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, oCursor );

        Object oldValue = null;

        // Change the CharColor and add a Shadow
        try {
            oCPS.setPropertyValue("CharColor",new Integer(255));
            oCPS.setPropertyValue("CharShadowed", new Boolean(true));
        } catch (Exception e) {
            System.out.println("Couldn't change the color " + e);
        }

        //create a paragraph break
        try {

            oText.insertControlCharacter( oCursor, ControlCharacter.PARAGRAPH_BREAK, false );

        } catch (Exception e) {
            System.out.println("Couldn't insert break "+ e);
        }



        //inserting colored Text
        System.out.println("inserting colored Text");

        oText.insertString( oCursor, " This is a colored Text - blue with shadow\n", false );

        //create a paragraph break
        try {

            oText.insertControlCharacter( oCursor, ControlCharacter.PARAGRAPH_BREAK, false );

        } catch (Exception e) {
            System.out.println("Couldn't insert break "+ e);
        }

        //***************************************************************************


        //oooooooooooooooooooooooooooStep 6oooooooooooooooooooooooooooooooooooooooooo
        // insert a text frame.
        // create an instance of com.sun.star.text.TextFrame using the MSF of the
        // document. Change some properties an insert it.
        // Now get the text-Object of the frame an the corresponding cursor.
        // Insert some text via insertString.
        //***************************************************************************


        // Create a TextFrame

        XTextFrame TF = null;
        XShape TFS = null;

        try {
            Object oInt = oDocMSF.createInstance("com.sun.star.text.TextFrame");
            TF = (XTextFrame) UnoRuntime.queryInterface(XTextFrame.class,oInt);
            TFS = (XShape) UnoRuntime.queryInterface(XShape.class,oInt);

            Size aSize = new Size();
            aSize.Height = 400;
            aSize.Width = 15000;

            TFS.setSize(aSize);

        } catch (Exception e) {
            System.out.println("Couldn't create instance "+ e);
        }


        // get the property set of the text frame

        XPropertySet oTFPS = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, TF );

        // Change the AnchorType
        try {

            oTFPS.setPropertyValue("AnchorType", TextContentAnchorType.AS_CHARACTER);

        } catch (Exception e) {
            System.out.println("Couldn't change the color " + e);
        }


        //insert the frame
        System.out.println("insert the text frame");

        try {
            oText.insertTextContent(oCursor, TF, false);
        } catch (Exception e) {
            System.out.println("Couldn't insert the frame " + e);
        }


        //getting the text object of Frame
        XText oTex = TF.getText();

        //create a cursor object
        XTextCursor oCurso = oTex.createTextCursor();

        //inserting some Text
        oTex.insertString( oCurso, "The first line in the newly created text frame.", false );


        oTex.insertString( oCurso, "\nWith this second line the height of the frame raises.", false );

        //insert a paragraph break
        try {

            oText.insertControlCharacter( oCursor, ControlCharacter.PARAGRAPH_BREAK, false );

        } catch (Exception e) {
            System.out.println("Couldn't insert break "+ e);
        }

        //***************************************************************************



        // Change the CharColor and add a Shadow
        try {
            oCPS.setPropertyValue("CharColor",new Integer(65536));
            oCPS.setPropertyValue("CharShadowed", new Boolean(false));
        } catch (Exception e) {
            System.out.println("Couldn't change the color " + e);
        }


        oText.insertString( oCursor, " That's all for now !!", false );


        //if the document should be disposed remove the slashes in front of the next line
        //myDoc.dispose();


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


    public static XTextDocument openWriter(XMultiServiceFactory oMSF) {


        //define variables
        XInterface oInterface;
        XDesktop oDesktop;
        XComponentLoader oCLoader;
        XTextDocument oDoc = null;
        XComponent aDoc = null;

        try {

            oInterface = (XInterface) oMSF.createInstance( "com.sun.star.frame.Desktop" );
            oDesktop = ( XDesktop ) UnoRuntime.queryInterface( XDesktop.class, oInterface );
            oCLoader = ( XComponentLoader ) UnoRuntime.queryInterface( XComponentLoader.class, oDesktop );
            PropertyValue [] szEmptyArgs = new PropertyValue [0];
            String doc = "private:factory/swriter";
            aDoc = oCLoader.loadComponentFromURL(doc, "_blank", 0, szEmptyArgs );
            oDoc = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, aDoc);

        } // end of try

        catch(Exception e){

            System.out.println(" Exception " + e);

        } // end of catch


        return oDoc;
    }//end of openWriter

    public static void insertIntoCell(String CellName, String theText, XTextTable TT1) {

        XText oTableText = (XText) UnoRuntime.queryInterface(XText.class, TT1.getCellByName(CellName));

        //create a cursor object
        XTextCursor oC = oTableText.createTextCursor();

        XPropertySet oTPS = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, oC );

        try {
            oTPS.setPropertyValue("CharColor",new Integer(16777215));
        } catch (Exception e) {
        }


        //inserting some Text
        oTableText.setString( theText );

    } // end of insertIntoCell

} // finish class SWriter
