/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
        the_Desk = UnoRuntime.queryInterface(
            XDesktop.class, DesktopTools.createDesktop(
                                (XMultiServiceFactory)Param.getMSF()) );
    }

    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes Impress documents.
    * @param Param test parameters
    * @param log writer to log information while testing
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing impress documents");
        util.DesktopTools.closeDoc(xImpressDoc);
        util.DesktopTools.closeDoc(xSecondDrawDoc);
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

        XModel aModel = UnoRuntime.queryInterface(XModel.class, xImpressDoc);

        XInterface oObj = aModel.getCurrentController();

        //Change to Slide view
        try {
            String aSlotID = "slot:27011";
            XDispatchProvider xDispProv = UnoRuntime.queryInterface( XDispatchProvider.class, oObj );
            XURLTransformer xParser = UnoRuntime.queryInterface(XURLTransformer.class,
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

        XModel aModel2 = UnoRuntime.queryInterface(XModel.class, xSecondDrawDoc);

        XWindow anotherWindow = UnoRuntime.queryInterface(
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

