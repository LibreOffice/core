/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XDocumentAuditing.java,v $
 * $Revision: 1.4 $
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
package ifc.sheet;

import com.sun.star.awt.Point;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNamed;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XDispatchHelper;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XDocumentAuditing;
import com.sun.star.sheet.XSheetAuditing;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.uno.UnoRuntime;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

/**
 *
 */
public class _XDocumentAuditing extends MultiMethodTest {
    public XDocumentAuditing oObj = null;
    XDrawPage xDrawPage = null;
    XSpreadsheet[] xSheet = null;
    int elementCount = 0;
    String sheetName = null;
    Point pos = null;

    public void before() {
        Exception ex = null;
        // get two sheets
        xSheet = new XSpreadsheet[2];
        try {
            XSpreadsheetDocument xSpreadsheetDocument = (XSpreadsheetDocument)
                    UnoRuntime.queryInterface(XSpreadsheetDocument.class, oObj);
            XSpreadsheets oSheets = xSpreadsheetDocument.getSheets();
            XIndexAccess oIndexSheets = (XIndexAccess) UnoRuntime.queryInterface(
                                                XIndexAccess.class, oSheets);
            XSpreadsheet oSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                                      XSpreadsheet.class, oIndexSheets.getByIndex(0));
            xSheet[0] = oSheet;
            oSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                                      XSpreadsheet.class, oIndexSheets.getByIndex(1));
            xSheet[1] = oSheet;
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            ex = e;
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            ex = e;
        }
        catch(java.lang.NullPointerException e) {
            ex = e;
        }
        if (ex != null) {
            throw new StatusException("Could not get two sheets.", ex);
        }

        // get the draw page for checking the shapes
        xDrawPage = (XDrawPage)tEnv.getObjRelation("XDocumentAuditing.DrawPage");
        if (xDrawPage == null) { // get from object
            try {
                XDrawPagesSupplier oDPS = (XDrawPagesSupplier)
                    UnoRuntime.queryInterface(XDrawPagesSupplier.class, oObj);
                Object o = oDPS.getDrawPages().getByIndex(1);
                xDrawPage = (XDrawPage)UnoRuntime.queryInterface(XDrawPage.class, o);
            }
            catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            } // ignore exceptions, we'll run into next if statement anyway
            catch(com.sun.star.lang.WrappedTargetException e) {
            }
        }
        if (xDrawPage == null) {
            throw new StatusException(Status.failed("'XSheetAuditing.DrawPage' object relation not found."));
        }
        if (xDrawPage.hasElements()) {
            elementCount = xDrawPage.getCount();
        }

        // switch off the automatic refresh
        PropertyValue[] props = new PropertyValue[1];
        props[0] = new PropertyValue();
        props[0].Name = "AutoRefreshArrows";
        props[0].Value = Boolean.FALSE;
        XModel xModel = (XModel)UnoRuntime.queryInterface(XModel.class, oObj);
        dispatch(xModel.getCurrentController().getFrame(), (XMultiServiceFactory)tParam.getMSF(), ".uno:AutoRefreshArrows", props);

        // prepare the sheets
        try {
            xSheet[0].getCellByPosition(6, 6).setValue(9);
            XNamed xNamed = (XNamed)UnoRuntime.queryInterface(XNamed.class, xSheet[0]);
            sheetName = xNamed.getName();
            xSheet[1].getCellByPosition(6, 6).setValue(16);
            xSheet[1].getCellByPosition(6, 7).setFormula("= SQRT(G7)");
            XSheetAuditing xSheetAuditing = (XSheetAuditing)UnoRuntime.queryInterface(XSheetAuditing.class, xSheet[1]);
            CellAddress add = new CellAddress((short)1, 6, 7);
            xSheetAuditing.showPrecedents(add);
            boolean ok = hasRightAmountOfShapes(1);
            if (!ok)
                throw new StatusException(Status.failed("Wrong amount of shapes on page."));
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            throw new StatusException("Could not set formulas on sheets.", e);
        }
    }

    public void after() {
        // switch the automatic refresh back on
        PropertyValue[] props = new PropertyValue[1];
        props[0] = new PropertyValue();
        props[0].Name = "AutoRefreshArrows";
        props[0].Value = Boolean.TRUE;
        XModel xModel = (XModel)UnoRuntime.queryInterface(XModel.class, oObj);
        dispatch(xModel.getCurrentController().getFrame(), (XMultiServiceFactory)tParam.getMSF(), ".uno:AutoRefreshArrows", props);
    }

    public void _refreshArrows() {
        boolean result = true;

        Point p0 = pos;

        try {
            result &= xSheet[1].getCellByPosition(6, 7).getValue() == 4;
            xSheet[1].getCellByPosition(6, 7).setFormula("= SQRT(" + sheetName + ".G7)");
            result &= xSheet[1].getCellByPosition(6, 7).getValue() == 3;
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            throw new StatusException("Could not set formulas on sheets.", e);
        }

        result &= hasRightAmountOfShapes(1);
        Point p1 = pos;

        // points have to be the same: if not we have an auto update
        boolean res = (p0.X == p1.X && p0.Y == p1.Y);
        result &= res;
        if (!res)
            log.println("Arrow has been refreshed, but this should have been switched off.");

        oObj.refreshArrows();

        result &= hasRightAmountOfShapes(1);
        Point p2 = pos;

        // points have to differ
        res = (p1.X != p2.X || p1.Y != p2.Y);
        result &= res;
        if (!res)
            log.println("Arrow has not been refreshed.");

        tRes.tested("refreshArrows()", result);
    }

    /**
     * Check if the amount of shapes is the right one after displaying that stuff
     * 2do improve this: check taht the shapes are the correct ones -> convwatch
     * @desiredValue That's the amount of shapes that have to be here.
     * @return True, if the actual count of shapes is the same
     */
    private boolean hasRightAmountOfShapes(int desiredValue) {
        int newCount = xDrawPage.getCount();
        if (newCount != elementCount + desiredValue) {
            return false;
        }
        else {
            if (desiredValue >= 0) {
                for (int i=elementCount; i<newCount; i++) {
                    try {
                        Object o = xDrawPage.getByIndex(i);
                        XShape xShape = (XShape)UnoRuntime.queryInterface(XShape.class, o);
                        pos = xShape.getPosition();
                        System.out.println("Shape Type: " + xShape.getShapeType());
                    }
                    catch(com.sun.star.uno.Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        }
        return true;
    }

    private void dispatch(Object oProvider, XMultiServiceFactory xMSF, String url, PropertyValue[] prop) {
        XDispatchProvider xDispatchProvider = (XDispatchProvider)UnoRuntime.queryInterface(XDispatchProvider.class, oProvider);
        Object dispatcher = null;
        try {
            dispatcher = xMSF.createInstance("com.sun.star.frame.DispatchHelper");
        }
        catch(com.sun.star.uno.Exception e) {
        }

        XDispatchHelper xDispatchHelper = (XDispatchHelper)UnoRuntime.queryInterface(XDispatchHelper.class, dispatcher);
        xDispatchHelper.executeDispatch(xDispatchProvider, url, "", 0, prop);
    }
}
