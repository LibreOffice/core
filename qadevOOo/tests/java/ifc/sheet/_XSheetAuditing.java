/*************************************************************************
 *
 *  $RCSfile: _XSheetAuditing.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-02 11:57:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
package ifc.sheet;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.sheet.ValidationType;
import com.sun.star.sheet.XSheetAuditing;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.table.CellAddress;
import com.sun.star.table.XCell;
import com.sun.star.text.XText;
import com.sun.star.uno.UnoRuntime;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

/**
 *
 */
public class _XSheetAuditing extends MultiMethodTest {
    public XSheetAuditing oObj = null;
    CellAddress address = null;
    CellAddress precedentAddress = null;
    CellAddress dependentAddress = null;
    XCell xAddress = null;
    XCell xPrecedentAddress = null;
    XCell xDependentAddress = null;
    XDrawPage xDrawPage = null;
    int elementCount = 0;

    public void before() {
        address = (CellAddress)tEnv.getObjRelation("XSheetAuditing.CellAddress");
        precedentAddress = (CellAddress)tEnv.getObjRelation("XSheetAuditing.PrecedentCellAddress");
        dependentAddress= (CellAddress)tEnv.getObjRelation("XSheetAuditing.DependentCellAddress");
        if (address == null || precedentAddress == null || dependentAddress == null) {
            throw new StatusException(Status.failed("Necessary CellAddress object relations not found."));
        }

        // get the draw page for checking the shapes
        xDrawPage = (XDrawPage)tEnv.getObjRelation("XSheetAuditing.DrawPage");
        if (xDrawPage == null) { // get from object
            XDrawPageSupplier oDPS = (XDrawPageSupplier)
                UnoRuntime.queryInterface(XDrawPageSupplier.class, oObj);
            xDrawPage = (XDrawPage) oDPS.getDrawPage();
        }
        if (xDrawPage == null) {
            throw new StatusException(Status.failed("'XSheetAuditing.DrawPage' object relation not found."));
        }
        if (xDrawPage.hasElements()) {
            elementCount = xDrawPage.getCount();
        }

        // get a sheet for changing the cells
        XSpreadsheet xSheet = (XSpreadsheet)tEnv.getObjRelation("XSheetAuditing.Spreadsheet");
        if (xSheet == null) // query on ther object
            xSheet = (XSpreadsheet)UnoRuntime.queryInterface(XSpreadsheet.class, oObj);
        if (xSheet == null)
            throw new StatusException(Status.failed("'XSheetAuditing.Spreadsheet' object relation not found."));
        try {
            xAddress = xSheet.getCellByPosition(address.Column, address.Row);
            xDependentAddress = xSheet.getCellByPosition(dependentAddress.Column, dependentAddress.Row);
            xPrecedentAddress = xSheet.getCellByPosition(precedentAddress.Column, precedentAddress.Row);
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            throw new StatusException(Status.failed("Invalid cell addresses in object relations."));
        }
    }

    public void _clearArrows() {
        requiredMethod("hideDependents()");
        boolean erg = false;
        oObj.showDependents(address);
        oObj.showPrecedents(address);
        erg = hasRightAmountOfShapes(3);
        oObj.clearArrows();
        erg &= hasRightAmountOfShapes(0);
        tRes.tested("clearArrows()", erg);
    }

    public void _hideDependents() {
        requiredMethod("showDependents()");
        oObj.hideDependents(address);
        tRes.tested("hideDependents()", hasRightAmountOfShapes(0));
    }

    public void _hidePrecedents() {
        requiredMethod("showPrecedents()");
//        requiredMethod("showPrecedents()");
        oObj.hidePrecedents(address);
        tRes.tested("hidePrecedents()", hasRightAmountOfShapes(0));
    }

    public void _showDependents() {
        requiredMethod("hidePrecedents()");
        oObj.showDependents(address);
        tRes.tested("showDependents()", hasRightAmountOfShapes(1));
    }

    public void _showErrors() {
        requiredMethod("clearArrows()");
        // construct an error: square root from -3
        xPrecedentAddress.setValue(-9);
        String cellAddress = new String(new byte[]{(byte)(precedentAddress.Column + 65)}) + (precedentAddress.Row+1);
        xAddress.setFormula("=SQRT(" + cellAddress + ")");
        XText xText = (XText)UnoRuntime.queryInterface(XText.class, xAddress);
        // correct error in cell:
        String error = xText.getString();
        boolean erg = error.equals("Err:502");
        log.println("Content: " + error);
        oObj.showErrors(dependentAddress);
        erg &= hasRightAmountOfShapes(2);
        tRes.tested("showErrors()", erg);
    }

    public void _showInvalid() {
        requiredMethod("showErrors()");
        boolean result = true;
        // insert a value
        xAddress.setValue(2.5);
        try {
            // add a validitation to a cell: only whole numbers are allowed
            XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xAddress);
            Object o = xPropertySet.getPropertyValue("Validation");
            XPropertySet xValidation = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, o);
            xValidation.setPropertyValue("Type", ValidationType.WHOLE);
            xPropertySet.setPropertyValue("Validation", xValidation);
            // test
            oObj.showInvalid();
            result = hasRightAmountOfShapes(1);
            oObj.clearArrows();
            result &= hasRightAmountOfShapes(0);
            // revoke the validitation to a cell
            xValidation.setPropertyValue("Type", ValidationType.ANY);
            xPropertySet.setPropertyValue("Validation", xValidation);
            // test again
            oObj.showInvalid();
            result &= hasRightAmountOfShapes(0);
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }

        tRes.tested("showInvalid()", result);
    }

    public void _showPrecedents() {
        oObj.showPrecedents(address);
        tRes.tested("showPrecedents()", hasRightAmountOfShapes(2));
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
}
