//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
// Name:              StyleCreation
//***************************************************************************
//
// comment:           Step 1: connect to the office an get the MSF
//                    Step 2: open an empty text document
//                    Step 3: create a new Paragraph style
//                    Step 4: apply the Paragraph style
//
//                    Chapter 4.1.3 Defining Your Own Style
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
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNameAccess;

// access the implementations via names
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.text.ControlCharacter.*;

import com.sun.star.text.XTextRange;
import com.sun.star.text.XText;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;


public class StyleCreation {
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

        if( xMSF != null )
            System.out.println( "Connecting to " + sConnectionString );

        // You need the desktop to create a document
        com.sun.star.frame.XDesktop xDesktop = null;
        xDesktop = getDesktop( xMSF );

        try {
            // create text document
            com.sun.star.text.XTextDocument xTextDocument = null;
            xTextDocument = createTextdocument( xDesktop );

            // BEGIN: 'Defining your own style' Section from the Tutorial

            // the service '..ParagraphStyle' is context dependend, you need the multi service factory
            // from the document to use the service
            XMultiServiceFactory xDocMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                XMultiServiceFactory.class, xTextDocument );

            // use the service 'com.sun.star.style.ParagraphStyle'
            XInterface xInterface = (XInterface) xDocMSF.createInstance( "com.sun.star.style.ParagraphStyle" );

            // create a supplier to get the Style family collection
            XStyleFamiliesSupplier xSupplier = ( XStyleFamiliesSupplier ) UnoRuntime.queryInterface(
                XStyleFamiliesSupplier.class, xTextDocument );

            // get the NameAccess interface from the Style family collection
            XNameAccess xNameAccess = xSupplier.getStyleFamilies();

            // select the Paragraph styles, you get the Paragraph style collection
            XNameContainer xParaStyleCollection = (XNameContainer) UnoRuntime.queryInterface(
                XNameContainer.class, xNameAccess.getByName( "ParagraphStyles" ));

            // create a PropertySet to set the properties for the new Paragraphstyle
            XPropertySet xPropertySet = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xInterface );
            System.out.println( "create a PropertySet to set the properties for the new Paragraphstyle" );

            // set some properties from the Paragraph style
            xPropertySet.setPropertyValue("CharFontName", new String( "Helvetica" ) );
            System.out.println( "set name of the font to 'Helvetica'" );

            xPropertySet.setPropertyValue("CharHeight", new Float( 36 ) );
            System.out.println( "Change the height of th font to 36" );

            xPropertySet.setPropertyValue("CharWeight", new Float( com.sun.star.awt.FontWeight.BOLD ) );
            System.out.println( "set the font attribute 'Bold'" );

            xPropertySet.setPropertyValue("CharAutoKerning", new Boolean( true ) );
            System.out.println( "set the paragraph attribute 'AutoKerning'" );

            xPropertySet.setPropertyValue("ParaAdjust", new Integer( com.sun.star.style.ParagraphAdjust.CENTER_value ) );
            System.out.println( "set the paragraph adjust to LEFT" );

            xPropertySet.setPropertyValue("ParaFirstLineIndent", new Integer( 0 ) );
            System.out.println( "set the first line indent to 0 cm" );

            xPropertySet.setPropertyValue("BreakType", com.sun.star.style.BreakType.PAGE_AFTER );
            System.out.println( "set the paragraph attribute Breaktype to PageAfter" );

            // insert the new Paragraph style in the Paragraph style collection
            xParaStyleCollection.insertByName( "myheading", xPropertySet );
            System.out.println( "create new paragraph style, with the values from the Propertyset");

            // get the Textrange from the document
            XTextRange xTextRange = xTextDocument.getText().getStart();

            // get the PropertySet from the current paragraph
            XPropertySet xParagraphPropertySet = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xTextRange );
            // change the value from the property 'ParaStyle' to apply the Paragraph style
            // To run the sample with StarOffice 5.2 you'll have to change 'ParaStyleName' to 'ParaStyle' in the next line
            xParagraphPropertySet.setPropertyValue("ParaStyleName", new String( "myheading" ) );
            System.out.println( "apply the new paragraph style");

            // END: 'Defining your own style' Section from the Tutorial
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
        }

        System.out.println("done");

        System.exit(0);
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

