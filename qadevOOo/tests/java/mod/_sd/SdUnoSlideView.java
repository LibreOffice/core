/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package mod._sd;

import com.sun.star.drawing.XDrawPages;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DesktopTools;
import util.DrawTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.awt.XWindow;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;

public class SdUnoSlideView extends TestCase {
    XDesktop the_Desk;
    XComponent xImpressDoc;
    XComponent xSecondDrawDoc;

    /**
    * Creates the instance of the service <code>com.sun.star.frame.Desktop</code>.
    * @see com.sun.star.frame.Desktop
    */
    protected void initialize(TestParameters Param, PrintWriter log) {
        the_Desk = (XDesktop)
            UnoRuntime.queryInterface(
                XDesktop.class, DesktopTools.createDesktop(
                                    (XMultiServiceFactory)Param.getMSF()) );
    }

    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes Impress documents.
    * @param tParam test parameters
    * @param tEnv the environment to cleanup
    * @param log writer to log information while testing
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing impress documents");
        util.DesktopTools.closeDoc(xImpressDoc);;
        util.DesktopTools.closeDoc(xSecondDrawDoc);;
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates two impress documents. After creating of the documents makes short
    * wait to allow frames to be loaded. Retrieves the collection of the draw pages
    * from the first document and takes one of them. Inserts some shapes to the
    * retrieved draw page. Obtains a current controller from the first document
    * using the interface <code>XModel</code>. The obtained controller is the
    * instance of the service <code>com.sun.star.presentation.OutlineView</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'FirstModel'</code> for
    *      {@link ifc.frame._XController}(the interface <code>XModel</code> of
    *      the first created document) </li>
    *  <li> <code>'Frame'</code> for
    *      {@link ifc.frame._XController}(the frame of the created
    *      document) </li>
    *  <li> <code>'SecondModel'</code> for
    *      {@link ifc.frame._XController}(the interface <code>XModel</code> of
    *      the second created document) </li>
    *  <li> <code>'SecondController'</code> for
    *      {@link ifc.frame._XController}(the current controller of the second
    *      created document) </li>
    * </ul>
    * @see com.sun.star.frame.XModel
    */
    protected synchronized TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {

        log.println( "creating a test environment" );

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());
        XDrawPages xDP = null;
        try {
            log.println( "creating a impress document" );
            xImpressDoc = SOF.createImpressDoc(null);
            shortWait();
            xDP = DrawTools.getDrawPages(xImpressDoc);
            xDP.insertNewByIndex(0);
            xDP.insertNewByIndex(0);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't create document", e);
        }

        XModel aModel = (XModel)
            UnoRuntime.queryInterface(XModel.class, xImpressDoc);

        XInterface oObj = aModel.getCurrentController();

        //Change to Slide view
        try {
            String aSlotID = "slot:27011";
            XDispatchProvider xDispProv = (XDispatchProvider)
                UnoRuntime.queryInterface( XDispatchProvider.class, oObj );
            XURLTransformer xParser = (com.sun.star.util.XURLTransformer)
                UnoRuntime.queryInterface(XURLTransformer.class,
        ((XMultiServiceFactory)Param.getMSF()).createInstance("com.sun.star.util.URLTransformer"));
            // Because it's an in/out parameter we must use an array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = aSlotID;
            xParser.parseStrict(aParseURL);
            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch( aURL,"",0);
            if( xDispatcher != null )
                    xDispatcher.dispatch( aURL, null );
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't change to slide view");
        }

        try {
            log.println( "creating a second impress document" );
            xSecondDrawDoc = SOF.createImpressDoc(null);
            shortWait();
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't create document", e);
        }

        XModel aModel2 = (XModel)
            UnoRuntime.queryInterface(XModel.class, xSecondDrawDoc);

        XWindow anotherWindow = (XWindow) UnoRuntime.queryInterface(
                                XWindow.class,aModel2.getCurrentController());

        oObj = aModel.getCurrentController();

        log.println( "creating a new environment for slide view object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        if (anotherWindow != null) {
            tEnv.addObjRelation("XWindow.AnotherWindow", anotherWindow);
        }

         //Adding ObjRelations for XController
        tEnv.addObjRelation("FirstModel", aModel);

        XFrame the_frame = the_Desk.getCurrentFrame();
        tEnv.addObjRelation("Frame", the_frame);

        //Adding ObjRelations for XController
        tEnv.addObjRelation("SecondModel", aModel2);

        XController secondController = aModel2.getCurrentController();
        tEnv.addObjRelation("SecondController", secondController);
        tEnv.addObjRelation("XDispatchProvider.URL",
                                    "slot:27069");

        tEnv.addObjRelation("XUserInputInterception.XModel", aModel);

        //creating obj-relation for the XSelectionSupplier
        try {
            Object[] selections =
                    new Object[]{xDP.getByIndex(0),xDP.getByIndex(1),xDP.getByIndex(2)};
            tEnv.addObjRelation("Selections", selections);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
        }  catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
        }
        log.println("Implementation Name: " + utils.getImplName(oObj));

        return tEnv;

    } // finish method getTestEnvironment

    private void shortWait() {
        try {
            Thread.sleep(1000) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }
    }


} // finish class SdUnoOutlineView

