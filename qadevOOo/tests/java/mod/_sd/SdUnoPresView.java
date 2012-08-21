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

import lib.StatusException;
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
public class SdUnoPresView extends TestCase {
    static XDesktop the_Desk;
    static XComponent xImpressDoc;
    static XComponent xSecondDrawDoc;

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
    * Creating a Test environment for the interfaces to be tested.
    * Creates two impress documents. After creating of the documents makes short
    * wait to allow frames to be loaded. Retrieves the collection of the draw pages
    * from the first document and takes one of them. Inserts some shapes to the
    * retrieved draw page. Obtains a current controller from the first document
    * using the interface <code>XModel</code>. The obtained controller is the
    * instance of the service
    * <code>com.sun.star.drawing.DrawingDocumentDrawView</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'First'</code> for
    *      {@link ifc.view._XSelectionSupplier}(the first created shape) </li>
    *  <li> <code>'Second'</code> for
    *      {@link ifc.view._XSelectionSupplier}(the second created shape) </li>
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
    * </ul>
    * @see com.sun.star.frame.XModel
    * @see com.sun.star.drawing.DrawingDocumentDrawView
    */
    protected synchronized TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {

        log.println( "creating a test environment" );

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                        (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating two impress documents" );
            xImpressDoc = SOF.createImpressDoc(null);
            xSecondDrawDoc = SOF.createImpressDoc(null);
            shortWait();
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't create document", e);
        }

        // get the drawpage of drawing here
        log.println( "getting Drawpage" );
        XDrawPagesSupplier oDPS = UnoRuntime.queryInterface(XDrawPagesSupplier.class, xImpressDoc);
        XDrawPages the_pages = oDPS.getDrawPages();
        XIndexAccess oDPi = UnoRuntime.queryInterface(XIndexAccess.class,the_pages);

        XDrawPage oDrawPage = null;
        XDrawPage secondDrawPage = null;
        try {
            oDrawPage = (XDrawPage) AnyConverter.toObject(
                    new Type(XDrawPage.class),oDPi.getByIndex(0));
            the_pages.insertNewByIndex(0);
            the_pages.insertNewByIndex(0);
            the_pages.insertNewByIndex(0);
            the_pages.insertNewByIndex(0);
           secondDrawPage = (XDrawPage) AnyConverter.toObject(
                    new Type(XDrawPage.class),oDPi.getByIndex(3));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get DrawPage", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get DrawPage", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get DrawPage", e);
        }

        //put something on the drawpage
        log.println( "inserting some Shapes" );
        XShapes oShapes = UnoRuntime.queryInterface(XShapes.class, oDrawPage);
        XShape shape1 = SOF.createShape(
            xImpressDoc, 5000, 3500, 7500, 5000, "Rectangle");
        oShapes.add(shape1);

        oShapes = UnoRuntime.queryInterface(XShapes.class, secondDrawPage);
        shape1 = SOF.createShape(
            xImpressDoc, 3000, 4500, 15000, 1000, "Ellipse");
        oShapes.add(shape1);

        XModel aModel = UnoRuntime.queryInterface(XModel.class, xImpressDoc);

        XInterface oObj = aModel.getCurrentController();

        XModel aModel2 = UnoRuntime.queryInterface(XModel.class, xSecondDrawDoc);

        XWindow anotherWindow = UnoRuntime.queryInterface(
                                XWindow.class,aModel2.getCurrentController());

        log.println( "creating a new environment for impress view object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        if (anotherWindow != null) {
            tEnv.addObjRelation("XWindow.AnotherWindow",anotherWindow);
        }

        tEnv.addObjRelation("Selections", new Object[] {
            oDrawPage, secondDrawPage});

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

        tEnv.addObjRelation("FirstPage", oDrawPage);
        tEnv.addObjRelation("SecondPage", secondDrawPage);

        //tEnv.addObjRelation("First", shape1);
        //tEnv.addObjRelation("Second", shape2);

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
                                    "slot:27010");

        log.println("Implementation Name: "+utils.getImplName(oObj));

        XModifiable modify = UnoRuntime.queryInterface(XModifiable.class,xImpressDoc);

        tEnv.addObjRelation("Modifiable",modify);


        tEnv.addObjRelation("XComponent.DisposeThis", xImpressDoc);

        return tEnv;

    } // finish method getTestEnvironment

    private void shortWait() {
        try {
            Thread.sleep(5000) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }
    }


} // finish class SdUnoDrawView

