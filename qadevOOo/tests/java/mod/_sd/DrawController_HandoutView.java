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

import java.io.PrintWriter;
import java.util.Comparator;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DesktopTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.awt.XWindow;
import com.sun.star.container.XIndexAccess;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XModifiable;

/**
* Test for object which is represented by service
* <code>com.sun.star.drawing.DrawingDocumentDrawView</code>. <p>
* The view was switched to <b>HandoutMode</b>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::drawing::DrawingDocumentDrawView</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
*  <li> <code>com::sun::star::frame::XController</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::view::XSelectionSupplier</code></li>
*  <li> <code>com::sun::star::drawing::XDrawView</code></li>
* </ul>
* @see com.sun.star.drawing.DrawingDocumentDrawView
* @see com.sun.star.lang.XComponent
* @see com.sun.star.lang.XServiceInfo
* @see com.sun.star.frame.XController
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.view.XSelectionSupplier
* @see com.sun.star.drawing.XDrawView
* @see ifc.drawing._DrawingDocumentDrawView
* @see ifc.lang._XComponent
* @see ifc.lang._XServiceInfo
* @see ifc.frame._XController
* @see ifc.beans._XPropertySet
* @see ifc.view._XSelectionSupplier
* @see ifc.drawing._XDrawView
*/
public class DrawController_HandoutView extends TestCase {
    static XDesktop the_Desk;
    static XComponent xDrawDoc;
    static XComponent xSecondDrawDoc;

    /**
    * Creates the instance of the service
    * <code>com.sun.star.frame.Desktop</code>.
    * @see com.sun.star.frame.Desktop
    */
    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        the_Desk = UnoRuntime.queryInterface(
            XDesktop.class, DesktopTools.createDesktop(
                                Param.getMSF()) );
    }

    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes Impress documents.
    * @param Param test parameters
    * @param log writer to log information while testing
    */
    @Override
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing impress documents");
        util.DesktopTools.closeDoc(xDrawDoc);
        util.DesktopTools.closeDoc(xSecondDrawDoc);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates two impress documents. After creating
    * of the documents makes short
    * wait to allow frames to be loaded. Retrieves
    * the collection of the draw pages
    * from the first document and takes one of them. Inserts some shapes to the
    * retrieved draw page.
    * Switch to the <b>HandoutMode</b>.
    * Obtains a current controller from the first document
    * using the interface <code>XModel</code>. The obtained controller is the
    * instance of the service
    * <code>com.sun.star.drawing.DrawingDocumentDrawView</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'Pages'</code> for
    *      {@link ifc.drawing._XDrawView}(the retrieved collection of the draw
    *      pages) </li>
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
    *  <li> <code>'DrawPage'</code> for
    *      {@link ifc.drawing._DrawingDocumentDrawView}(the draw page which will
    *      be new current page) </li>
    * </ul>
    * @see com.sun.star.frame.XModel
    * @see com.sun.star.drawing.DrawingDocumentDrawView
    */
    @Override
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) throws Exception {

        log.println( "creating a test environment" );
        XMultiServiceFactory xMSF = Param.getMSF();
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(xMSF);

        log.println( "creating two impress documents" );
        xSecondDrawDoc = SOF.createImpressDoc(null);
        util.utils.waitForEventIdle(Param.getMSF());
        xDrawDoc = SOF.createImpressDoc(null);
        util.utils.waitForEventIdle(Param.getMSF());

        // get the drawpage of drawing here
        log.println( "getting Drawpage" );
        XDrawPagesSupplier oDPS = UnoRuntime.queryInterface(XDrawPagesSupplier.class, xDrawDoc);
        XDrawPages the_pages = oDPS.getDrawPages();
        for (int i = 1; i < 10; i++){
            the_pages.insertNewByIndex(i);
        }
        XIndexAccess oDPi = UnoRuntime.queryInterface(XIndexAccess.class,the_pages);

        XDrawPage oDrawPage = (XDrawPage) AnyConverter.toObject(
                    new Type(XDrawPage.class),oDPi.getByIndex(0));

        //put something on the drawpage
        log.println( "inserting some Shapes" );
        XShapes oShapes = UnoRuntime.queryInterface(XShapes.class, oDrawPage);
        XShape shape1 = SOF.createShape(
            xDrawDoc, 3000, 4500, 15000, 1000, "Ellipse");
        XShape shape2 = SOF.createShape(
            xDrawDoc, 5000, 3500, 7500, 5000, "Rectangle");
        XShape shape3 = SOF.createShape(
            xDrawDoc, 3000, 500, 5000, 1000, "Line");
        oShapes.add(shape1);
        oShapes.add(shape2);
        oShapes.add(shape3);
        util.utils.waitForEventIdle(Param.getMSF());

        log.println("switch to HandoutView...");
        utils.dispatchURL(xMSF, xDrawDoc, ".uno:HandoutMode");

        util.utils.waitForEventIdle(Param.getMSF());

        XModel aModel = UnoRuntime.queryInterface(XModel.class, xDrawDoc);

        XInterface oObj = aModel.getCurrentController();

        log.println("bring first document to front...");
        DesktopTools.bringWindowToFront(aModel);

        XModel aModel2 = UnoRuntime.queryInterface(XModel.class, xSecondDrawDoc);

        XWindow anotherWindow = UnoRuntime.queryInterface(
                                XWindow.class,aModel2.getCurrentController());

        log.println( "creating a new environment for impress view object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        if (anotherWindow != null) {
            tEnv.addObjRelation("XWindow.AnotherWindow",anotherWindow);
        }

        Object oShapeCol1 = null;
        Object oShapeCol2 = null;
        oShapeCol1 = xMSF.
            createInstance("com.sun.star.drawing.ShapeCollection");
        oShapeCol2 = xMSF.
            createInstance("com.sun.star.drawing.ShapeCollection");

        XShapes xShapes1 = UnoRuntime.queryInterface(XShapes.class, oShapeCol1);
        XShapes xShapes2 = UnoRuntime.queryInterface(XShapes.class, oShapeCol2);
        xShapes1.add(shape2);
        xShapes1.add(shape3);
        xShapes2.add(shape1);
        util.utils.waitForEventIdle(Param.getMSF());

        tEnv.addObjRelation("Selections", new Object[] {
            oDrawPage, oShapeCol1, oShapeCol2});
        tEnv.addObjRelation("Comparer", new Comparator<Object>() {
            public int compare(Object o1, Object o2) {
                XIndexAccess indAc1 = UnoRuntime.queryInterface(XIndexAccess.class, o1);
                XIndexAccess indAc2 = UnoRuntime.queryInterface(XIndexAccess.class, o2);
                if (indAc1 == null || indAc2 == null) return -1;
                if (indAc1.getCount() == indAc2.getCount()) {
                    return 0;
                }
                return 1;
            }});



        tEnv.addObjRelation("Pages", the_pages);

        //Adding ObjRelations for XController
        tEnv.addObjRelation("FirstModel", aModel);

        tEnv.addObjRelation("XUserInputInterception.XModel", aModel);

        XFrame the_frame = the_Desk.getCurrentFrame();
        tEnv.addObjRelation("Frame", the_frame);

         aModel = UnoRuntime.queryInterface(XModel.class, xSecondDrawDoc);
        //Adding ObjRelations for XController
        tEnv.addObjRelation("SecondModel", aModel);

        XController secondController = aModel.getCurrentController();
        tEnv.addObjRelation("SecondController", secondController);
        tEnv.addObjRelation("XDispatchProvider.URL",
                                    "slot:27009");

        //Adding relations for DrawingDocumentDrawView
        XDrawPage new_page = the_pages.insertNewByIndex(1);
        tEnv.addObjRelation("DrawPage", new_page);

        log.println("Implementation Name: "+utils.getImplName(oObj));

        XModifiable modify = UnoRuntime.queryInterface(XModifiable.class,xDrawDoc);

        tEnv.addObjRelation("Modifiable",modify);

        tEnv.addObjRelation("XComponent.DisposeThis", xDrawDoc);


        return tEnv;

    } // finish method getTestEnvironment


}

