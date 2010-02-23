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

import com.sun.star.container.NoSuchElementException;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.form.XForm;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XViewPane;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import util.FormTools;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.sheet.SpreadsheetViewPane</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::sheet::XViewPane</code></li>
 *  <li> <code>com::sun::star::sheet::XCellRangeReferrer</code></li>
 * </ul>
 * @see com.sun.star.sheet.SpreadsheetViewPane
 * @see com.sun.star.sheet.XViewPane
 * @see com.sun.star.sheet.XCellRangeReferrer
 * @see ifc.sheet._XViewPane
 * @see ifc.sheet._XCellRangeReferrer
 */
public class ScViewPaneObj extends TestCase {
    static private XSpreadsheetDocument xSpreadsheetDoc;
    static private SOfficeFactory SOF;
    static private XInterface oObj;

    /**
     * Creates Spreadsheet document.
     */
    public void initialize( TestParameters Param, PrintWriter log ) {
        // get a soffice factory object
        SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

        try {
            log.println("creating a spreadsheetdocument");
            xSpreadsheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document ", e );
        }
    }

    /**
     * Disposes Spreadsheet document.
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println("disposing xSpreadsheetDocument");
        XComponent oComp = (XComponent)
        UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Retieves the current controller of the spreadsheet document using the
     * interface <code>XModel</code>. The controller contains the collection
     * of the view panes so retrieves the view pane with index 0 from
     * the collection. The retrived view pane is the instance of the service
     * <code>com.sun.star.sheet.SpreadsheetViewPane</code>. Retrieves the address
     * of the cell range that consists of the cells which are visible in the pane.
     * Object relations created :
     * <ul>
     *  <li> <code>'DATAAREA'</code> for
     *      {@link ifc.sheet._XViewPane}(the retrieved address)</li>
     * </ul>
     * @see com.sun.star.frame.XModel
     */
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {
        XDrawPage oDrawPage;

        XModel xm = (XModel)
        UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc);
        XController xc = xm.getCurrentController();
        XIndexAccess xIA = (XIndexAccess)
        UnoRuntime.queryInterface(XIndexAccess.class, xc);
        try {
            oObj = (XInterface) AnyConverter.toObject(
                new Type(XInterface.class),xIA.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by index", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oObj);

        //Relation for XControlAccess
        tEnv.addObjRelation("DOCUMENT", UnoRuntime.queryInterface(XComponent.class,xSpreadsheetDoc));
        tEnv.addObjRelation("XControlAccess.isSheet", Boolean.TRUE);

        XViewPane VP = (XViewPane)
        UnoRuntime.queryInterface(XViewPane.class, oObj);
        CellRangeAddress dataArea = VP.getVisibleRange();
        tEnv.addObjRelation("DATAAREA", dataArea);

        // XForm for com.sun.star.view.XFormLayerAccess
        log.println("adding relation for com.sun.star.view.XFormLayerAccess: XForm");

        XForm myForm = null;
        String kindOfControl="CommandButton";
        XShape aShape = null;
        try{
            log.println("adding contol shape '" + kindOfControl + "'");
            XComponent oComp = (XComponent) UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc) ;

            aShape = FormTools.createControlShape(oComp, 3000, 4500, 15000, 10000, kindOfControl);

        } catch (Exception e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't create following control shape : '" +
                kindOfControl + "': ", e);
        }

        try {
            log.println( "getting Drawpages" );
            XDrawPagesSupplier oDPS = (XDrawPagesSupplier)
            UnoRuntime.queryInterface(XDrawPagesSupplier.class,xSpreadsheetDoc);
            XDrawPages oDP = (XDrawPages) oDPS.getDrawPages();
            oDP.insertNewByIndex(1);
            oDP.insertNewByIndex(2);
            oDrawPage = (XDrawPage) AnyConverter.toObject(
                new Type(XDrawPage.class),oDP.getByIndex(0));
            if (oDrawPage == null)
                log.println("ERROR: could not get DrawPage: null");

            oDrawPage.add(aShape);
            log.println("getting XForm");
            XNameContainer xForm = FormTools.getForms(oDrawPage);
            try {
                myForm = (XForm) AnyConverter.toObject(new Type(XForm.class),xForm.getByName("Standard"));
            } catch (WrappedTargetException ex) {
                log.println("ERROR: could not XFormm 'Standard': " + ex.toString());
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                log.println("ERROR: could not XFormm 'Standard': " + ex.toString());
            } catch (NoSuchElementException ex) {
                log.println("ERROR: could not XFormm 'Standard': " + ex.toString());
            }
            if (myForm == null)
                log.println("ERROR: could not get XForm: null");
            tEnv.addObjRelation("XFormLayerAccess.XForm", myForm);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ex) {
            log.println("ERROR: could not add ObjectRelation 'XFormLayerAccess.XForm': " + ex.toString());
        } catch (WrappedTargetException ex) {
            log.println("ERROR: could not add ObjectRelation 'XFormLayerAccess.XForm': " + ex.toString());
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("ERROR: could not add ObjectRelation 'XFormLayerAccess.XForm': " + ex.toString());
        }

        return tEnv;
    }
}

