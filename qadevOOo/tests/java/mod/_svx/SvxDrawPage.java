/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SvxDrawPage.java,v $
 * $Revision: 1.8.8.1 $
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
package mod._svx;

import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.DrawTools;
import util.FormTools;
import util.InstCreator;
import util.SOfficeFactory;
import util.ShapeDsc;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.drawing.DrawPage</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::drawing::GenericDrawPage</code></li>
 *  <li> <code>com::sun::star::drawing::XShapeBinder</code></li>
 *  <li> <code>com::sun::star::drawing::XShapeGrouper</code></li>
 *  <li> <code>com::sun::star::drawing::XShapes</code></li>
 *  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::container::XIndexAccess</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::drawing::XMasterPageTarget</code></li>
 *  <li> <code>com::sun::star::container::XNamed</code></li>
 *  <li> <code>com::sun::star::drawing::XShapeCombiner</code></li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.drawing.GenericDrawPage
 * @see com.sun.star.drawing.XShapeBinder
 * @see com.sun.star.drawing.XShapeGrouper
 * @see com.sun.star.drawing.XShapes
 * @see com.sun.star.lang.XServiceInfo
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.container.XIndexAccess
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.drawing.XMasterPageTarget
 * @see com.sun.star.container.XNamed
 * @see com.sun.star.drawing.XShapeCombiner
 * @see ifc.drawing._GenericDrawPage
 * @see ifc.drawing._XShapeBinder
 * @see ifc.drawing._XShapeGrouper
 * @see ifc.drawing._XShapes
 * @see ifc.lang._XServiceInfo
 * @see ifc.beans._XPropertySet
 * @see ifc.container._XIndexAccess
 * @see ifc.container._XElementAccess
 * @see ifc.drawing._XMasterPageTarget
 * @see ifc.container._XNamed
 * @see ifc.drawing._XShapeCombiner
 */
public class SvxDrawPage extends TestCase {
    static XSpreadsheetDocument xDoc = null;

    /**
     * Creates a new Draw document.
     */
    protected void initialize(TestParameters tParam, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     (XMultiServiceFactory) tParam.getMSF());

        try {
            log.println("creating a sheetdocument");
            xDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    /**
     * Disposes the Draw document created before
     */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xSheetDoc ");
        util.DesktopTools.closeDoc(xDoc);
        ;
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * From the Draw document created a collection of its draw
     * pages is obtained. Two new pages are inserted. And one
     * page is obtained as a testing component. A shape is added
     * to this page. <p>
     *
     *     Object relations created :
     * <ul>
     *  <li> <code>'DrawPage'</code> for
     *      {@link ifc.drawing._XShapeGrouper} :
     *      the draw page tested. </li>
     *  <li> <code>'Shape'</code> for
     *      {@link ifc.drawing._XShapes} :
     *      the creator which can create instances of
     *      <code>com.sun.star.drawing.Line</code> service </li>
     * </ul>
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        XInterface oObj = null;
        XShape oShape = null;
        XDrawPages oDP = null;

        XComponent xComp = (XComponent) UnoRuntime.queryInterface(
                                   XComponent.class, xDoc);


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        try {
            log.println("getting Drawpages");

            XDrawPagesSupplier oDPS = (XDrawPagesSupplier) UnoRuntime.queryInterface(
                                              XDrawPagesSupplier.class, xDoc);
            oDP = (XDrawPages) oDPS.getDrawPages();
            oDP.insertNewByIndex(1);
            oDP.insertNewByIndex(2);
            oObj = (XDrawPage) AnyConverter.toObject(new Type(XDrawPage.class),
                                                     oDP.getByIndex(0));

            SOfficeFactory SOF = SOfficeFactory.getFactory(
                                         (XMultiServiceFactory) tParam.getMSF());

            oShape = SOF.createShape(xComp, 5000, 3500, 7500, 5000,
                                     "Rectangle");
            DrawTools.getShapes((XDrawPage) oObj).add(oShape);

            XShape oShape1 = SOF.createShape(xComp, 5000, 5500, 5000, 5000,
                                             "Rectangle");
            DrawTools.getShapes((XDrawPage) oObj).add(oShape1);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't create insance");
            e.printStackTrace(log);
            throw new StatusException("Can't create enviroment", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't create insance");
            e.printStackTrace(log);
            throw new StatusException("Can't create enviroment", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't create insance");
            e.printStackTrace(log);
            throw new StatusException("Can't create enviroment", e);
        }

        // create test environment here
        TestEnvironment tEnv = new TestEnvironment(oObj);


        //adding a controlButton to have a Form
        FormTools.insertControlShape(
                (XComponent) UnoRuntime.queryInterface(XComponent.class, xDoc),
                3000, 4500, 15000, 1000, "CommandButton");

        // relation for XShapes interface
        ShapeDsc sDsc = new ShapeDsc(5000, 3500, 7500, 10000, "Line");
        tEnv.addObjRelation("Shape", new InstCreator(xDoc, sDsc));

        log.println("ImplementationName: " + util.utils.getImplName(oObj));


        // adding relation for XShapeGrouper
        tEnv.addObjRelation("DrawPage", oObj);

        // adding relation for XMasterPageTarget

        /*XMasterPagesSupplier oMPS = (XMasterPagesSupplier)
            UnoRuntime.queryInterface(XMasterPagesSupplier.class, xDoc);
        XDrawPages oGroup = oMPS.getMasterPages();
        tEnv.addObjRelation("MasterPageSupplier",oGroup); */
        return tEnv;
    } // finish method getTestEnvironment
} // finish class SvxDrawPage
