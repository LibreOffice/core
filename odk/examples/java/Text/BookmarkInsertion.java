//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
// Name:              BookmarkInsertion
//***************************************************************************
//
// comment:           Step 1: connect to the office an get the MSF
//                    Step 2: open an empty text document
//                    Step 3: enter a example text
//                    Step 4: insert some bookmarks
//
//                    Chapter 5.1.1.4 Inserting bookmarks
//
//***************************************************************************
// date                 : Thu August 18  2000
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


public class BookmarkInsertion {

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

        if( xMSF != null )  System.out.println("Connected to " + sConnectionString );

        // You need the desktop to create a document
        com.sun.star.frame.XDesktop xDesktop = null;
        xDesktop = getDesktop( xMSF );

        // create text document
        com.sun.star.text.XTextDocument xTextDocument = null;
        xTextDocument = createTextdocument( xDesktop );

        // put example text in document
        createExampleData( xTextDocument );


        String mOffending[] = { "negro(e|es)?","bor(ed|ing)?", "bloody?", "bleed(ing)?" };
        String mBad[] = { "possib(le|ilit(y|ies))", "real(ly)+", "brilliant" };

        String sOffendPrefix = "Offending";
        String sBadPrefix = "BadStyle";

        markList( xTextDocument, mOffending, sOffendPrefix );
        markList( xTextDocument, mBad, sBadPrefix );

        System.out.println("Done");

        System.exit(0);

    }

    public static void markList( com.sun.star.text.XTextDocument xTextDocument, String mList[], String sPrefix ) {
        int iCounter=0;
        com.sun.star.uno.XInterface xSearchInterface = null;
        com.sun.star.text.XTextRange xSearchTextRange = null;

        try {
            for( iCounter = 0; iCounter < mList.length; iCounter++ ) {
                // the findfirst returns a XInterface
                xSearchInterface = ( com.sun.star.uno.XInterface ) FindFirst( xTextDocument, mList[ iCounter ] );

                if( xSearchInterface != null ) {
                    // get the TextRange form the XInterface
                    xSearchTextRange = (com.sun.star.text.XTextRange) UnoRuntime.queryInterface(
                        com.sun.star.text.XTextRange.class, xSearchInterface );

                    InsertBookmark( xTextDocument, xSearchTextRange, sPrefix + iCounter );
                }
            }
        }
        catch( Exception e) {
            e.printStackTrace(System.out);
        }


    }


    public static void InsertBookmark( com.sun.star.text.XTextDocument xTextDocument, com.sun.star.text.XTextRange xTextRange, String sBookName ) {
        // create a bookmark on a TextRange
        try {
            // get the MultiServiceFactory from the text document
            XMultiServiceFactory xDocMSF;
            xDocMSF = (XMultiServiceFactory) UnoRuntime.queryInterface( XMultiServiceFactory.class, xTextDocument );

            // the bookmark service is a context dependend service, you need the MultiServiceFactory from the document
            Object xObject = xDocMSF.createInstance( "com.sun.star.text.Bookmark" );

            // set the name from the bookmark
            com.sun.star.container.XNamed xNameAccess = null;
            xNameAccess = (com.sun.star.container.XNamed) UnoRuntime.queryInterface( com.sun.star.container.XNamed.class, xObject );

            xNameAccess.setName( sBookName );

            // create a XTextContent, for the method 'insertTextContent'
            com.sun.star.text.XTextContent xTextContent = null;
            xTextContent = ( com.sun.star.text.XTextContent ) UnoRuntime.queryInterface(
                com.sun.star.text.XTextContent.class, xNameAccess );

            // insertTextContent need a TextRange not a cursor to specify the position from the bookmark
            xTextDocument.getText().insertTextContent( xTextRange, xTextContent, true );

        }
        catch( Exception e) {
            e.printStackTrace(System.out);
        }
    }

    protected static com.sun.star.uno.XInterface FindFirst( com.sun.star.text.XTextDocument xTextDocument, String sSearchString ) {
        com.sun.star.util.XSearchDescriptor xSearchDescriptor = null;
        com.sun.star.util.XSearchable xSearchable = null;
        com.sun.star.uno.XInterface xSearchInterface = null;

        try {
            xSearchable = ( com.sun.star.util.XSearchable ) UnoRuntime.queryInterface(
                com.sun.star.util.XSearchable.class, xTextDocument );
            xSearchDescriptor = ( com.sun.star.util.XSearchDescriptor) xSearchable.createSearchDescriptor();

            xSearchDescriptor.setSearchString( sSearchString );

            com.sun.star.beans.XPropertySet xPropertySet = null;
            xPropertySet = (com.sun.star.beans.XPropertySet) UnoRuntime.queryInterface(
                com.sun.star.beans.XPropertySet.class, xSearchDescriptor );

            xPropertySet.setPropertyValue("SearchRegularExpression", new Boolean( true ) );

            xSearchInterface = ( com.sun.star.uno.XInterface ) xSearchable.findFirst( xSearchDescriptor );
        }
        catch( Exception e) {
            e.printStackTrace();
        }

        return xSearchInterface;
    }

    protected static void createExampleData( com.sun.star.text.XTextDocument xTextDocument ) {
        com.sun.star.text.XTextCursor xTextCursor = null;

        try {
            xTextCursor = (com.sun.star.text.XTextCursor) xTextDocument.getText().createTextCursor();

            xTextCursor.setString( "He heard quiet steps behind him. That didn't bode well. Who could be following him this late at night and in this deadbeat part of town? And at this particular moment, just after he pulled off the big time and was making off with the greenbacks. Was there another crook who'd had the same idea, and was now watching him and waiting for a chance to grab the fruit of his labor?" );
            xTextCursor.collapseToEnd();
            xTextCursor.setString( "Or did the steps behind him mean that one of many bloody officers in town was on to him and just waiting to pounce and snap those cuffs on his wrists? He nervously looked all around. Suddenly he saw the alley. Like lightening he darted off to the left and disappeared between the two warehouses almost falling over the trash can lying in the middle of the sidewalk. He tried to nervously tap his way along in the inky darkness and suddenly stiffened: it was a dead-end, he would have to go back the way he had come" );
            xTextCursor.collapseToEnd();
            xTextCursor.setString( "The steps got louder and louder, he saw the black outline of a figure coming around the corner. Is this the end of the line? he thought pressing himself back against the wall trying to make himself invisible in the dark, was all that planning and energy wasted? He was dripping with sweat now, cold and wet, he could smell the brilliant fear coming off his clothes. Suddenly next to him, with a barely noticeable squeak, a door swung quietly to and fro in the night's breeze." );

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
