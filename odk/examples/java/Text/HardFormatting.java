//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
// Name:              hardformating
//***************************************************************************
//
// comment:           Step 1: connect to the office an get the MSF
//                    Step 2: open an empty text document
//                    Step 3: enter a example text
//                    Step 4: get some text attributes
//                    Step 5: check the PropertyState from the selection
//
//                    Chapter 4.1.4 Hard formatting
//
//
//***************************************************************************

import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;

// access the implementations via names
import com.sun.star.comp.servicemanager.ServiceManager;

import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import com.sun.star.uno.XComponentContext;

// access the implementations via names
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.text.ControlCharacter.*;

import com.sun.star.text.XText;


public class HardFormatting {

    public static void main(String args[]) {
        String sConnectionString = "uno:socket,host=localhost,port=8100;urp;StarOffice.NamingService";

        // It is possible to use a different connection string, passed as argument
        if ( args.length == 1 ) {
            sConnectionString = args[0];
        }

        XMultiServiceFactory xMSF = null;
        try {
            xMSF = connect( sConnectionString );
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
            System.exit( 0 );
        }

        if( xMSF != null )  System.out.println("Connecting to " + sConnectionString );

        // You need the desktop to create a document
        com.sun.star.frame.XDesktop xDesktop = null;
        xDesktop = getDesktop( xMSF );

        try {
            // BEGIN: 'Style basics' Section from the Cookbook

            // create text document
            com.sun.star.text.XTextDocument xTextDocument = null;
            xTextDocument = createTextdocument( xDesktop );

            // the text interface contains all methods and properties to
            // manipulate the content from a text document
            com.sun.star.text.XText xText = null;
            xText = xTextDocument.getText();

            String sMyText = "A very short paragraph for illustration only";

            // you can travel with the cursor throught the text document.
            // you travel only at the model, not at the view. The cursor that you can
            // see on the document doesn't change the position
            com.sun.star.text.XTextCursor xTextCursor = null;
            xTextCursor = (com.sun.star.text.XTextCursor) xTextDocument.getText().createTextCursor();

            xText.insertString( xTextCursor, "Headline", false );
            xText.insertControlCharacter( xTextCursor, com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, false );

            xText.insertString( xTextCursor, sMyText, false );

            com.sun.star.text.XTextRange xTextRange = null;
            com.sun.star.beans.XPropertySet xPropertySet = null;

            // BEGIN: 'Hard formating' Section from the Cookbook

            // the text range not the cursor contains the 'parastyle' property
            xTextRange = xText.getEnd();
            xPropertySet = (com.sun.star.beans.XPropertySet) UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xTextRange );

            // create a paragraph cursor to travel throught the paragraphs
            com.sun.star.text.XParagraphCursor xParagraphCursor = null;
            xParagraphCursor = (com.sun.star.text.XParagraphCursor) UnoRuntime.queryInterface(
                com.sun.star.text.XParagraphCursor.class, xTextRange );

            xParagraphCursor.gotoStart( false );
            xParagraphCursor.gotoEndOfParagraph( true );
            xTextRange = xParagraphCursor.getText().getStart();

            // create a WordCursor to travel into the paragraph
            com.sun.star.text.XWordCursor xWordCursor = null;
            xWordCursor = (com.sun.star.text.XWordCursor) UnoRuntime.queryInterface(
                com.sun.star.text.XWordCursor.class, xTextRange );

            // the PropertySet from the cursor contains the text attributes
            xPropertySet = (com.sun.star.beans.XPropertySet) UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xWordCursor );
            System.out.println( "Parastyle : " + xPropertySet.getPropertyValue("ParaStyleName").toString()
            + "\nFontname : " + xPropertySet.getPropertyValue("CharFontName").toString()
            + "\nWeight : " + xPropertySet.getPropertyValue("CharWeight").toString() );

            xWordCursor.gotoNextWord(false);
            xWordCursor.gotoNextWord(false);
            xWordCursor.gotoEndOfWord(true);

            xPropertySet = (com.sun.star.beans.XPropertySet) UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xWordCursor );
            xPropertySet.setPropertyValue("CharWeight", new Float( com.sun.star.awt.FontWeight.BOLD ) );
            xPropertySet.setPropertyValue("CharColor", new Integer( 255 ) );

            System.out.println( "Parastyle : " + xPropertySet.getPropertyValue("ParaStyleName").toString()
            + "\nFontname : " + xPropertySet.getPropertyValue("CharFontName").toString()
            + "\nWeight : " + xPropertySet.getPropertyValue("CharWeight").toString() );

            // the PropertyState contains information where the attribute is set,
            // is a text part hard formated or not.
            com.sun.star.beans.XPropertyState xPropertyState = null;
            xPropertyState = (com.sun.star.beans.XPropertyState) UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertyState.class, xWordCursor );

            com.sun.star.beans.PropertyState xPropertyStateValue = xPropertyState.getPropertyState( "CharWeight" );

            checkPropertyState( xWordCursor,  xPropertyStateValue );

            xWordCursor.goRight( (short) 3 , true );
            xPropertyStateValue = xPropertyState.getPropertyState( "CharWeight" );

            System.out.println( "Increase the selection with three characters" );
            checkPropertyState( xWordCursor,  xPropertyStateValue );

            xPropertyState.setPropertyToDefault( "CharWeight" );

            System.out.println( "Set the default value on the selection" );
            xPropertyStateValue = xPropertyState.getPropertyState( "CharWeight" );
            checkPropertyState( xWordCursor,  xPropertyStateValue );

            // END: 'Hard formating' Section from the Cookbook
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
        }


        System.out.println("Done");

        System.exit(0);

    }


    public static void checkPropertyState( com.sun.star.text.XWordCursor xWordCursor, com.sun.star.beans.PropertyState xPropertyStateValue ) {
        switch( xPropertyStateValue.getValue() ) {
            case com.sun.star.beans.PropertyState.DIRECT_VALUE_value:  {
                System.out.println( "-> The selection '" + xWordCursor.getString() + "' completly hard formated" );
                break;
            }

            case com.sun.star.beans.PropertyState.DEFAULT_VALUE_value: {
                System.out.println( "-> The selection '" + xWordCursor.getString() + "' isn't hard formated" );
                break;
            }

            case com.sun.star.beans.PropertyState.AMBIGUOUS_VALUE_value: {
                System.out.println( "-> The selection '" + xWordCursor.getString() + "' isn't completly hard formated" );
                break;
            }

            default:
                System.out.println( "No PropertyState found" );
        }
    }

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


    public static com.sun.star.frame.XDesktop getDesktop( XMultiServiceFactory xMSF ) {
        XInterface xInterface = null;
        com.sun.star.frame.XDesktop xDesktop = null;

        if( xMSF != null ) {
            try {
                xInterface = (XInterface) xMSF.createInstance("com.sun.star.frame.Desktop");
                xDesktop = (com.sun.star.frame.XDesktop) UnoRuntime.queryInterface(
                    com.sun.star.frame.XDesktop.class, xInterface);
            }
            catch( Exception e) {
                e.printStackTrace(System.out);
            }
        }
        else
            System.out.println( "Can't create a desktop. null pointer !" );

        return xDesktop;
    }

    public static com.sun.star.text.XTextDocument createTextdocument( com.sun.star.frame.XDesktop xDesktop ) {
        com.sun.star.text.XTextDocument aTextDocument = null;

        try {
            com.sun.star.lang.XComponent xComponent = null;
            xComponent = CreateNewDocument( xDesktop, "swriter" );

            aTextDocument = (com.sun.star.text.XTextDocument) UnoRuntime.queryInterface(
                com.sun.star.text.XTextDocument.class, xComponent );
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
        }

        return aTextDocument;
    }


    protected static com.sun.star.lang.XComponent CreateNewDocument( com.sun.star.frame.XDesktop xDesktop, String sDocumentType ) {
        String sURL = "private:factory/" + sDocumentType;

        com.sun.star.lang.XComponent xComponent = null;
        PropertyValue xValues[] = new PropertyValue[1];

        com.sun.star.frame.XComponentLoader xComponentLoader = null;
        XInterface xInterface = null;

        PropertyValue[] xEmptyArgs = new PropertyValue[0];

        xComponentLoader = (com.sun.star.frame.XComponentLoader) UnoRuntime.queryInterface(
            com.sun.star.frame.XComponentLoader.class, xDesktop );

        try {
            xComponent  = xComponentLoader.loadComponentFromURL( sURL, "_blank", 0, xEmptyArgs);
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
        }

        return xComponent ;
    }
}
