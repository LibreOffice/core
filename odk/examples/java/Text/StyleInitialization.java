//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
// Name:              StyleInitialization
//***************************************************************************
//
// comment:           Step 1: connect to the office an get the MSF
//                    Step 2: open an empty text document
//                    Step 3: enter a example text
//                    Step 4: use the paragraph collection
//                    Step 5: apply a different paragraph style on the paragraphs
//
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

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;

import com.sun.star.uno.IBridge;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import com.sun.star.uno.XComponentContext;

// access the implementations via names
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.text.ControlCharacter.*;

import com.sun.star.text.XText;


public class StyleInitialization {

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
            // BEGIN: 'Style basics' Section from the Tutorial

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

            XPropertySet oCPS = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, xTextCursor );
            try {
                oCPS.setPropertyValue("CharFontName","Helvetica");
            }
            catch (Exception ex) {

            }

            xText.insertString( xTextCursor, "Headline", false );

            try {
                oCPS.setPropertyValue("CharFontName","Times");
            }
            catch (Exception ex) {

            }
            xText.insertControlCharacter( xTextCursor, com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, false );

            xText.insertString( xTextCursor, sMyText, false );

            com.sun.star.text.XTextRange xTextRange = null;
            com.sun.star.beans.XPropertySet xPropertySet = null;

            // the text range not the cursor contains the 'parastyle' property
            xTextRange = xText.getEnd();
            xPropertySet = (com.sun.star.beans.XPropertySet) UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xTextRange );

            // To run the sample with StarOffice 5.2 you'll have to change 'ParaStyleName' to 'ParaStyle' in the next line
            System.out.println( "Current Parastyle : " + xPropertySet.getPropertyValue("ParaStyleName") );

            // END: 'Style basics' Section from the Tutorial

            // There are two way to travel throught the paragraphs, with a paragraph cursor, or a enumeration.
            // You find both ways in this example

            // The first way, with the paragraph cursor
            com.sun.star.text.XParagraphCursor xParagraphCursor = null;
            xParagraphCursor = (com.sun.star.text.XParagraphCursor) UnoRuntime.queryInterface(
                com.sun.star.text.XParagraphCursor.class, xTextRange );

            xParagraphCursor.gotoStart( false );
            xParagraphCursor.gotoEndOfParagraph( true );

            // The second way, with the paragraph enumeration
            com.sun.star.container.XEnumerationAccess xEnumerationAccess = null;
            xEnumerationAccess = (com.sun.star.container.XEnumerationAccess) UnoRuntime.queryInterface(
                com.sun.star.container.XEnumerationAccess.class, xText );

            // the enumeration contains all paragraph form the document
            com.sun.star.container.XEnumeration xParagraphEnumeration = null;
            xParagraphEnumeration = xEnumerationAccess.createEnumeration();

            com.sun.star.text.XTextContent xParagraph = null;
            com.sun.star.text.XTextRange xWord = null;

            com.sun.star.container.XEnumerationAccess xParaEnumerationAccess = null;
            com.sun.star.container.XEnumeration xPortionEnumeration = null;

            // check if a paragraph is available
            while ( xParagraphEnumeration.hasMoreElements() ) {
                // get the next paragraph
                xParagraph = (com.sun.star.text.XTextContent) UnoRuntime.queryInterface(
                    com.sun.star.text.XTextContent.class, xParagraphEnumeration.nextElement());

                // you need the method getAnchor to a TextRange -> to manipulate the paragraph
                String sText = xParagraph.getAnchor().getString();

                // create a cursor from this paragraph
                com.sun.star.text.XTextCursor xParaCursor = null;
                xParaCursor = xParagraph.getAnchor().getText().createTextCursor();

                // goto the start and end of the paragraph
                xParaCursor.gotoStart( false );
                xParaCursor.gotoEnd( true );

                // The enumeration from the paragraphs contain parts from the paragraph with a
                // different attributes.
                xParaEnumerationAccess = (com.sun.star.container.XEnumerationAccess)
                    UnoRuntime.queryInterface(com.sun.star.container.XEnumerationAccess.class, xParagraph);
                xPortionEnumeration = xParaEnumerationAccess.createEnumeration();

                while ( xPortionEnumeration.hasMoreElements() ) {
                    // output of all parts from the paragraph with different attributes
                    xWord = (com.sun.star.text.XTextRange) UnoRuntime.queryInterface(
                        com.sun.star.text.XTextRange.class, xPortionEnumeration.nextElement());
                    String sWordString = xWord.getString();
                    System.out.println( "Content of the paragraph : " + sWordString );
                }
            }

            // BEGIN: 'Finding a suitable style' Section from the Tutorial

            // craete a supplier to get the styles-collection
            com.sun.star.style.XStyleFamiliesSupplier xSupplier = null;
            xSupplier = ( com.sun.star.style.XStyleFamiliesSupplier ) UnoRuntime.queryInterface(
                com.sun.star.style.XStyleFamiliesSupplier.class, xTextDocument );

            // use the name access from the collection
            com.sun.star.container.XNameAccess xNameAccess = null;
            xNameAccess = xSupplier.getStyleFamilies();

            com.sun.star.container.XNameContainer xParaStyleCollection = null;
            xParaStyleCollection = (com.sun.star.container.XNameContainer) UnoRuntime.queryInterface(
                com.sun.star.container.XNameContainer.class, xNameAccess.getByName( "ParagraphStyles" ));

            // create a array from strings with the name of all paragraph styles from the text document
            String[] sElementNames = xParaStyleCollection.getElementNames();
            int iElementCount = sElementNames.length;

            for( int iCounter = 0;  iCounter < iElementCount; iCounter++ ) {
                // specify one paragraph style
                com.sun.star.style.XStyle xStyle = null;
                xStyle = (com.sun.star.style.XStyle) UnoRuntime.queryInterface(
                    com.sun.star.style.XStyle.class,
                    xParaStyleCollection.getByName( sElementNames[iCounter] ));

                // create a property set of all properties from the style
                xPropertySet = (com.sun.star.beans.XPropertySet) UnoRuntime.queryInterface(
                    com.sun.star.beans.XPropertySet.class, xStyle );

                AnyConverter aAnyConv = new AnyConverter();
                String sFontname = aAnyConv.toString(xPropertySet.getPropertyValue("CharFontName"));
                sFontname = sFontname.toLowerCase();

                // if the style use the font 'Albany', apply it to the current paragraph
                if( sFontname.compareTo("albany") == 0 ) {
                    // create a property set from the current paragraph, to change the paragraph style
                    xPropertySet = (com.sun.star.beans.XPropertySet) UnoRuntime.queryInterface(
                        com.sun.star.beans.XPropertySet.class, xTextRange );

                    // To run the sample with StarOffice 5.2 you'll have to change 'ParaStyleName'
                    // to 'ParaStyle' in the next line
                    xPropertySet.setPropertyValue("ParaStyleName", new String( sElementNames[iCounter] ) );
                    System.out.println( "Apply the paragraph style : " + sElementNames[iCounter] );
                    break;
                }
            }
            // END: 'Finding a suitable style' Section from the Tutorial
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
        }


        System.out.println("Done");

        System.exit(0);

    }

    public static XMultiServiceFactory connect( String connectStr )
    throws com.sun.star.uno.Exception, com.sun.star.uno.RuntimeException, Exception {
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
