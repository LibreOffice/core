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

package mod._sc;

import com.sun.star.container.NoSuchElementException;

import java.io.PrintWriter;

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
    private static XSpreadsheetDocument xSpreadsheetDoc;
    private static SOfficeFactory SOF;
    private static XInterface oObj;

    /**
     * Creates Spreadsheet document.
     */
    @Override
    public void initialize( TestParameters Param, PrintWriter log ) throws Exception {
        // get a soffice factory object
        SOF = SOfficeFactory.getFactory( Param.getMSF());
        log.println("creating a spreadsheetdocument");
        xSpreadsheetDoc = SOF.createCalcDoc(null);
    }

    /**
     * Disposes Spreadsheet document.
     */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println("disposing xSpreadsheetDocument");
        XComponent oComp = UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
     * Creating a TestEnvironment for the interfaces to be tested.
     * Retrieves the current controller of the spreadsheet document using the
     * interface <code>XModel</code>. The controller contains the collection
     * of the view panes so retrieves the view pane with index 0 from
     * the collection. The retrieved view pane is the instance of the service
     * <code>com.sun.star.sheet.SpreadsheetViewPane</code>. Retrieves the address
     * of the cell range that consists of the cells which are visible in the pane.
     * Object relations created :
     * <ul>
     *  <li> <code>'DATAAREA'</code> for
     *      {@link ifc.sheet._XViewPane}(the retrieved address)</li>
     * </ul>
     * @see com.sun.star.frame.XModel
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) throws Exception {
        XDrawPage oDrawPage;

        XModel xm = UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc);
        XController xc = xm.getCurrentController();
        XIndexAccess xIA = UnoRuntime.queryInterface(XIndexAccess.class, xc);
        oObj = (XInterface) AnyConverter.toObject(
            new Type(XInterface.class),xIA.getByIndex(0));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        //Relation for XControlAccess
        tEnv.addObjRelation("DOCUMENT", UnoRuntime.queryInterface(XComponent.class,xSpreadsheetDoc));
        tEnv.addObjRelation("XControlAccess.isSheet", Boolean.TRUE);

        XViewPane VP = UnoRuntime.queryInterface(XViewPane.class, oObj);
        CellRangeAddress dataArea = VP.getVisibleRange();
        tEnv.addObjRelation("DATAAREA", dataArea);

        // XForm for com.sun.star.view.XFormLayerAccess
        log.println("adding relation for com.sun.star.view.XFormLayerAccess: XForm");

        XForm myForm = null;
        String kindOfControl="CommandButton";
        XShape aShape = null;
        log.println("adding control shape '" + kindOfControl + "'");
        XComponent oComp = UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc) ;

        aShape = FormTools.createControlShape(oComp, 3000, 4500, 15000, 10000, kindOfControl);

        try {
            log.println( "getting Drawpages" );
            XDrawPagesSupplier oDPS = UnoRuntime.queryInterface(XDrawPagesSupplier.class,xSpreadsheetDoc);
            XDrawPages oDP = oDPS.getDrawPages();
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

