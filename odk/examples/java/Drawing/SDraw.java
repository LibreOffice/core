//ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
// Name             : SDraw
//***************************************************************************
//
// comment:           Step 1: connect to the office an get the MSF
//                    Step 2: open an empty text document
//                    Step 3: get the drawpage an insert some shapes
//
//***************************************************************************
// date                 : Tue August 22  2000
//
//***************************************************************************


import java.lang.Math;

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
import com.sun.star.drawing.*;
import com.sun.star.text.XText;
import com.sun.star.awt.*;

public class SDraw  {


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
        XComponent xDrawDoc = null;
        XDrawPage oObj = null;
        XShape oShape = null;
        XShape oShape1 = null;
        XShapes oShapes = null;


        // create connection(s) and get multiservicefactory

        // create connection(s) and get multiservicefactory
        System.out.println( "getting MultiServiceFactory" );

        try {
            xMSF = connect( sConnectionString );
        } catch( Exception Ex ) {
            System.out.println( "Couldn't get MSF"+ Ex );
        }

        //***************************************************************************

        //oooooooooooooooooooooooooooStep 2oooooooooooooooooooooooooooooooooooooooooo
        // open an empty document. In this case it's a draw document.
        // For this purpose an instance of com.sun.star.frame.Desktop
        // is created. It's interface XDesktop provides the XComponentLoader,
        // which is used to open the document via loadComponentFromURL
        //***************************************************************************

        //Open document

        //Draw

        System.out.println("Opening an empty Draw document");
        xDrawDoc = openDraw(xMSF);


        //***************************************************************************

        //oooooooooooooooooooooooooooStep 3oooooooooooooooooooooooooooooooooooooooooo
        // get the drawpage an insert some shapes.
        // the documents DrawPageSupplier supplies the DrawPage vi IndexAccess
        // To add a shape get the MultiServiceFaktory of the document, create an
        // instance of the ShapeType and add it to the Shapes-container
        // provided by the drawpage
        //***************************************************************************


        // get the drawpage of drawing here
        try {
            System.out.println( "getting Drawpage" );
            XDrawPagesSupplier oDPS = (XDrawPagesSupplier) UnoRuntime.queryInterface(
                XDrawPagesSupplier.class,xDrawDoc);
            XDrawPages oDPn = oDPS.getDrawPages();
            XIndexAccess oDPi = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class,oDPn);
            oObj = (XDrawPage) UnoRuntime.queryInterface(XDrawPage.class, oDPi.getByIndex(0));
        } catch ( Exception e ) {
            System.out.println( "Couldn't create document"+ e );
        }

        createSequence(xDrawDoc, oObj);

        //put something on the drawpage
        System.out.println( "inserting some Shapes" );
        oShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class,oObj);
        oShapes.add(createShape(xDrawDoc,2000,1500,1000,1000,"Line",0));
        oShapes.add(createShape(xDrawDoc,3000,4500,15000,1000,"Ellipse",16711680));
        oShapes.add(createShape(xDrawDoc,5000,3500,7500,5000,"Rectangle",6710932));

        //***************************************************************************



        System.out.println("done");

        System.exit(0);


    } // finish method main

    public static XMultiServiceFactory connect( String connectStr )
    throws com.sun.star.uno.Exception,
    com.sun.star.uno.RuntimeException, Exception {
      /* Bootstraps a component context with the jurt base components
         registered. Component context to be granted to a component for running.
         Arbitrary values can be retrieved from the context. */
        XComponentContext xcomponentcontext =
        com.sun.star.comp.helper.Bootstrap.createInitialComponentContext( null );

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

        Object rInitialObject = xurlresolver.resolve( connectStr );

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



    public static XComponent openDraw(XMultiServiceFactory oMSF) {


        //define variables
        XInterface oInterface;
        XDesktop oDesktop;
        XComponentLoader oCLoader;
        XComponent aDoc = null;

        try {

            oInterface = (XInterface) oMSF.createInstance( "com.sun.star.frame.Desktop" );
            oDesktop = ( XDesktop ) UnoRuntime.queryInterface( XDesktop.class, oInterface );
            oCLoader = ( XComponentLoader ) UnoRuntime.queryInterface( XComponentLoader.class, oDesktop );
            PropertyValue [] szEmptyArgs = new PropertyValue [0];
            String doc = "private:factory/sdraw";
            aDoc = oCLoader.loadComponentFromURL(doc, "_blank", 0, szEmptyArgs );

        } // end of try

        catch(Exception e){

            System.out.println(" Exception " + e);

        } // end of catch


        return aDoc;
    }//end of openDraw

    public static XShape createShape(XComponent oDoc, int height, int width, int x, int y, String kind, int col) {
        //possible values for kind are 'Ellipse', 'Line' and 'Rectangle'
        Size size = new Size();
        Point position = new Point();
        XShape oShape = null;

        //get MSF
        XMultiServiceFactory oDocMSF = (XMultiServiceFactory) UnoRuntime.queryInterface( XMultiServiceFactory.class, oDoc );

        try{
            Object oInt = oDocMSF.createInstance("com.sun.star.drawing."+kind+"Shape");
            oShape = (XShape)UnoRuntime.queryInterface( XShape.class, oInt );
            size.Height = height;
            size.Width = width;
            position.X = x;
            position.Y = y;
            oShape.setSize(size);
            oShape.setPosition(position);

        } catch ( Exception e ) {
            // Some exception occures.FAILED
            System.out.println( "Couldn't create instance "+ e );
        }

        XPropertySet oSPS = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, oShape);

        try {

            oSPS.setPropertyValue("FillColor",new Integer(col));


        } catch (Exception e) {

            System.out.println("Can't change colors " + e);

        }


        return oShape;

    }

    public static XShape createSequence(XComponent oDoc, XDrawPage oDP) {
        Size size = new Size();
        Point position = new Point();
        XShape oShape = null;
        XShapes oShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class,oDP);
        int height = 3000;
        int width = 3500;
        int x = 1900;
        int y = 20000;
        Object oInt = null;
        int r = 40;
        int g = 0;
        int b = 80;

        //get MSF
        XMultiServiceFactory oDocMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
            XMultiServiceFactory.class, oDoc );

        for (int i=0; i<370; i=i+25) {
            try{
                oInt = oDocMSF.createInstance("com.sun.star.drawing.EllipseShape");
                oShape = (XShape)UnoRuntime.queryInterface( XShape.class, oInt );
                size.Height = height;
                size.Width = width;
                position.X = (x+(i*40));
                position.Y = (new Float(y+(Math.sin((i*Math.PI)/180))*5000)).intValue();
                oShape.setSize(size);
                oShape.setPosition(position);

            } catch ( Exception e ) {
                // Some exception occures.FAILED
                System.out.println( "Couldn't get Shape "+ e );
            }

            b=b+8;

            XPropertySet oSPS = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, oShape);

            try {

                oSPS.setPropertyValue("FillColor",new Integer(getCol(r,g,b)));
                oSPS.setPropertyValue("Shadow",new Boolean(true));

            } catch (Exception e) {

                System.out.println("Can't change colors " + e);

            }
            oShapes.add(oShape);
        }

        XShapeGrouper gr = (XShapeGrouper)UnoRuntime.queryInterface( XShapeGrouper.class, oDP );

        oShape = gr.group(oShapes);


        return oShape;
    }

    public static int getCol(int r, int g, int b) {

        return r*65536+g*256+b;

    }


} // finish class SDraw



