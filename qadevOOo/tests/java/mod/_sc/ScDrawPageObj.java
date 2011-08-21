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

package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DrawTools;
import util.InstCreator;
import util.SOfficeFactory;
import util.ShapeDsc;

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

public class ScDrawPageObj extends TestCase {

    static XSpreadsheetDocument xDoc = null;

    /**
     * Creates a new Draw document.
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a sheetdocument" );
            xDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
     * Disposes the Draw document created before
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent xComp = (XComponent)
                            UnoRuntime.queryInterface(XComponent.class, xDoc);
        util.DesktopTools.closeDoc(xComp);
    }


    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * From the Calc document created a collection of its draw
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
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XShape oShape = null ;
        XDrawPages oDP = null;

        XComponent xComp = (XComponent)
                            UnoRuntime.queryInterface(XComponent.class, xDoc);

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );
        try {
            log.println( "getting Drawpages" );
            XDrawPagesSupplier oDPS = (XDrawPagesSupplier)
                UnoRuntime.queryInterface(XDrawPagesSupplier.class,xDoc);
            oDP = (XDrawPages) oDPS.getDrawPages();
            oDP.insertNewByIndex(1);
            oDP.insertNewByIndex(2);
            oObj = (XDrawPage) AnyConverter.toObject(
                    new Type(XDrawPage.class),oDP.getByIndex(0));

            SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

            oShape = SOF.createShape(xComp,5000,3500,7500,5000,"Rectangle");
            DrawTools.getShapes((XDrawPage) oObj).add(oShape);
            XShape oShape1 = SOF.createShape(xComp,
                5000,5500,5000,5000,"Rectangle");
            DrawTools.getShapes((XDrawPage) oObj).add(oShape1);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't create insance");
            e.printStackTrace(log);
            throw new StatusException("Can't create enviroment", e) ;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't create insance");
            e.printStackTrace(log);
            throw new StatusException("Can't create enviroment", e) ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't create insance");
            e.printStackTrace(log);
            throw new StatusException("Can't create enviroment", e) ;
        }

        // create test environment here
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // relation for XShapes interface
        ShapeDsc sDsc = new ShapeDsc(5000,3500,7500,10000,"Line");
        tEnv.addObjRelation("Shape", new InstCreator(xDoc, sDsc)) ;

        log.println("ImplementationName: "+util.utils.getImplName(oObj));

        // adding relation for XShapeGrouper
        tEnv.addObjRelation("DrawPage", oObj);

        return tEnv;
    } // finish method getTestEnvironment

}

