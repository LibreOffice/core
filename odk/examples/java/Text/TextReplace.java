//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
// Name:              TextReplace
//***************************************************************************
//
// comment:           Step 1: connect to the office an get the MSF
//                    Step 2: open an empty text document
//                Step 3: enter a example text
//                    Step 4: replace some english spelled words with US spelled
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

// access the implementations via names
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.text.ControlCharacter.*;

import com.sun.star.text.XText;

public class TextReplace {

    public static void main(String args[]) {
        //the connection string to connect the office
        String sConnectionString = "uno:socket,host=localhost,port=8100;urp;StarOffice.NamingService";

        // It is possible to use a different connection string, passed as argument
        if ( args.length == 1 ) {
            sConnectionString = args[0];
        }

        String mBritishWords[] = {"colour", "neighbour", "centre", "behaviour", "metre", "through" };
        String mUSWords[] = { "color", "neighbor", "center", "behavior", "meter", "thru" };

        XMultiServiceFactory xMSF = null;
        try {
            xMSF = connect( sConnectionString );
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
            System.exit( 0 );
        }

        if( xMSF != null )  System.out.println("Connected to " + sConnectionString);

        com.sun.star.frame.XDesktop xDesktop = null;
        xDesktop = getDesktop( xMSF );

        com.sun.star.text.XTextDocument xTextDocument = null;
        xTextDocument = createTextdocument( xDesktop );

        createExampleData( xTextDocument );

        try {
            com.sun.star.util.XReplaceDescriptor xReplaceDescriptor = null;
            com.sun.star.util.XSearchDescriptor xSearchDescriptor = null;
            com.sun.star.util.XReplaceable xReplaceable = null;

            xReplaceable = ( com.sun.star.util.XReplaceable ) UnoRuntime.queryInterface(
                com.sun.star.util.XReplaceable.class, xTextDocument );

            // You need a descriptor to set properies for Replace
            xReplaceDescriptor = ( com.sun.star.util.XReplaceDescriptor) xReplaceable.createReplaceDescriptor();

            for( int iArrayCounter = 0; iArrayCounter < mBritishWords.length; iArrayCounter++ ) {
                // Set the properties the replace method need
                xReplaceDescriptor.setSearchString( mBritishWords[ iArrayCounter ] );
                xReplaceDescriptor.setReplaceString( mUSWords[ iArrayCounter ] );

                // Replace all words
                xReplaceable.replaceAll( xReplaceDescriptor );
            }

        }
        catch( Exception e) {
            e.printStackTrace(System.out);
        }


        System.out.println("Done");

        System.exit(0);

    }

    protected static void createExampleData( com.sun.star.text.XTextDocument xTextDocument ) {
        // Create textdocument and insert example text
        com.sun.star.text.XTextCursor xTextCursor = null;

        try {
            xTextCursor = (com.sun.star.text.XTextCursor) xTextDocument.getText().createTextCursor();
            XText xText = (XText) xTextDocument.getText();

            xText.insertString( xTextCursor, "He nervously looked all around. Suddenly he saw his ", false );

            xText.insertString( xTextCursor, "neighbour ", true );
            XPropertySet xCPS = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, xTextCursor );
            xCPS.setPropertyValue( "CharColor", new Integer( 255 ) );// Set the word blue
            xTextCursor.gotoEnd(false); // Go to last character
            xCPS.setPropertyValue( "CharColor", new Integer( 0 ) );

            xText.insertString( xTextCursor, "in the alley. Like lightening he darted off to the left and disappeared between the two warehouses almost falling over the trash can lying in the ", false  );

            xText.insertString( xTextCursor, "centre ", true );
            xCPS = (XPropertySet) UnoRuntime.queryInterface( XPropertySet.class, xTextCursor );
            xCPS.setPropertyValue( "CharColor", new Integer( 255 ) );// Set the word blue
            xTextCursor.gotoEnd(false);  // Go to last character
            xCPS.setPropertyValue( "CharColor", new Integer( 0 ) );

            xText.insertString( xTextCursor, "of the sidewalk.", false );

            xText.insertControlCharacter( xTextCursor, com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, false );
            xText.insertString( xTextCursor, "He tried to nervously tap his way along in the inky darkness and suddenly stiffened: it was a dead-end, he would have to go back the way he had come.", false );

            xTextCursor.gotoStart(false);
        }
        catch( Exception e) {
            e.printStackTrace();
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
