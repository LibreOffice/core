//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
// Name:              TextDocumentStructure
//***************************************************************************
//
// comment:           Step 1: connect to the office an get the MSF
//                    Step 2: open an empty text document
//                    Step 3: create an enumeration of all paragraphs
//                    Step 4: create an enumeration of all text portions
//
//***************************************************************************
// date                 : Tue August 22  2000
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
import com.sun.star.text.XTextContent;

import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XEnumeration;

// access the implementations via names
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.text.ControlCharacter.*;

import com.sun.star.lang.XComponent;
import com.sun.star.text.XText;
import com.sun.star.text.XTextDocument;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XWordCursor;
import com.sun.star.beans.XPropertyState;
import com.sun.star.beans.PropertyState;

import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.lang.XServiceInfo;

public class TextDocumentStructure {

    public static void main(String args[]) {
        // This connection string to connect the office
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

        if( xMSF != null )
            System.out.println("Connected to " + sConnectionString );

        try {
            System.out.println("create new text document");
            // start document init
            XInterface xInterface = (XInterface) xMSF.createInstance("com.sun.star.frame.Desktop");

            XComponentLoader xComponentLoader = (XComponentLoader) UnoRuntime.queryInterface(
                XComponentLoader.class, xInterface );

            PropertyValue[] xEmptyArgs = new PropertyValue[0];
            XComponent xComponent  = xComponentLoader.loadComponentFromURL( "private:factory/swriter", "_blank", 0, xEmptyArgs);

            XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(
                XTextDocument.class, xComponent );
            XText xText = xTextDocument.getText();
            createExampleData( xText );

            // end document init


            // Begin section 'The structure of text documents' of the Tutorial

            XEnumeration xParagraphEnumeration = null;
            XTextContent xTextElement = null;
            XEnumerationAccess xParaEnumerationAccess = null;
            XEnumeration xPortionEnumeration = null;
            XEnumeration xTextPortionEnum;

            System.out.println("create an enumeration of all paragraphs");
            // create an enumeration access of all paragraphs of a document
            XEnumerationAccess xEnumerationAccess = (XEnumerationAccess) UnoRuntime.queryInterface(
                XEnumerationAccess.class, xText );
            xParagraphEnumeration = xEnumerationAccess.createEnumeration();

            // Loop through all paragraphs of the document
            while ( xParagraphEnumeration.hasMoreElements() ) {
                xTextElement = (XTextContent) UnoRuntime.queryInterface(
                    XTextContent.class, xParagraphEnumeration.nextElement());
                XServiceInfo xServiceInfo = (XServiceInfo) UnoRuntime.queryInterface(
                    XServiceInfo.class, xTextElement );

                // check ifs the current paragraph really a paragraph or an anchor of a frame or picture
                if( xServiceInfo.supportsService("com.sun.star.text.Paragraph") ) {
                    XTextRange xTextRange = xTextElement.getAnchor();
                    System.out.println( "This is a Paragraph" );

                    // create another enumeration to get all text portions of the paragraph
                    xParaEnumerationAccess = (XEnumerationAccess) UnoRuntime.queryInterface(
                        XEnumerationAccess.class, xTextElement );
                    xTextPortionEnum = xParaEnumerationAccess.createEnumeration();

                    while ( xTextPortionEnum.hasMoreElements() ) {
                        XTextRange xTextPortion = (XTextRange) UnoRuntime.queryInterface(
                            XTextRange.class, xTextPortionEnum.nextElement());
                        System.out.println( "Text from the portion : " + xTextPortion.getString() );

                        XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(
                            XPropertySet.class, xTextPortion );
                        System.out.println( "Name of the font : " + xPropertySet.getPropertyValue( "CharFontName" ) );

                        // PropertyState status of each text portion.
                        XPropertyState xPropertyState = (XPropertyState) UnoRuntime.queryInterface(
                            XPropertyState.class, xTextPortion );

                        if( xPropertyState.getPropertyState( "CharWeight" ).equals(PropertyState.AMBIGUOUS_VALUE) )
                            System.out.println( "-  The text range contains more than one different attributes" );

                        if( xPropertyState.getPropertyState( "CharWeight" ).equals(PropertyState.DIRECT_VALUE ) )
                            System.out.println( " - The text range contains hard formats" );

                        if( xPropertyState.getPropertyState( "CharWeight" ).equals(PropertyState.DEFAULT_VALUE ) )
                            System.out.println( " - The text range doesn't contains hard formats" );
                    }
                }
                else
                    System.out.println( "The text portion isn't a text paragraph" );
                // End section 'The structure of text documents' of the Tutorial
            }
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
        }

        System.out.println("done");
        System.exit(0);
    }

    public static void createExampleData( XText xText ) {

        try {
            xText.setString( "This is an example sentence" );

            XWordCursor xWordCursor = (XWordCursor) UnoRuntime.queryInterface(
                XWordCursor.class, xText.getStart() );

            xWordCursor.gotoNextWord(false);
            xWordCursor.gotoNextWord(false);
            xWordCursor.gotoEndOfWord(true);

            XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xWordCursor );
            xPropertySet.setPropertyValue("CharWeight", new Float( com.sun.star.awt.FontWeight.BOLD ) );

            System.out.println("create example data");
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
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

}
