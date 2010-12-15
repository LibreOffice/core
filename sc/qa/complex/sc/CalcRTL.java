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
package complex.sc;

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyAttribute;
import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.container.XIndexAccess;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;

import complexlib.ComplexTestCase;

import java.io.PrintWriter;

import util.DrawTools;
import util.SOfficeFactory;
import util.ValueComparer;


public class CalcRTL extends ComplexTestCase {
    XSpreadsheetDocument xSheetDoc = null;

    /*
     * This testcase checks the calcrtl feature added to cws_calcrtl in three steps
     * <br>
     * (<b>REMARK: </b> the testcases expects the TableLayout to be LR_TB)
     * <br>
     * 1. A calc document is opened and the properties in css.sheet.Spreadsheet are checked
     * <br>
     * 2. A calc document is opened and a shape inserted, afterwards the properties in css.sheet.Shape are checked
     * <br>
     * 3. A calc document is opened and a shape inserted, afterwards the effect of changing the TableLayout to the shape is checked
     */
    public String[] getTestMethodNames() {
        return new String[] {
            "checkSpreadsheetProperties", "checkShapeProperties",
            "checkInfluenceOfSpreadsheetChange"
        };
    }

    /*
     * In this method a spreadsheet document is opened<br>
     * afterwards all properties of the Spreadsheet are checked.<br>
     * <p>
     * These are
     * <ul>
     *<li> IsVisible
     *<li> PageStyle
     *<li> TableLayout
     *</ul>
     */
    public void checkSpreadsheetProperties() {
        assure("Couldn't open document", openSpreadsheetDocument());

        XPropertySet set = (XPropertySet) UnoRuntime.queryInterface(
                                   XPropertySet.class, getSpreadsheet());
        assure("Problems when setting property 'IsVisible'",
               changeProperty(set, "IsVisible", Boolean.FALSE));
        assure("Problems when setting property 'IsVisible'",
               changeProperty(set, "IsVisible", Boolean.TRUE));
        assure("Problems when setting property 'PageStyle'",
               changeProperty(set, "PageStyle", "Report"));
        assure("Problems when setting property 'PageStyle'",
               changeProperty(set, "PageStyle", "Default"));
        assure("Problems when setting property 'TableLayout'",
               changeProperty(set, "TableLayout",
                              new Short(com.sun.star.text.WritingMode2.RL_TB)));
        assure("Problems when setting property 'TableLayout'",
               changeProperty(set, "TableLayout",
                              new Short(com.sun.star.text.WritingMode2.LR_TB)));
        assure("Couldn't close document", closeSpreadsheetDocument());
    }

    /*
     * In this method a spreadsheet document is opened and a shape inserted<br>
     * afterwards all calc specific properties of the Shape are checked.<br>
     * <p>
     * These are
     * <ul>
     *<li> Anchor
     *<li> HoriOrientPosition
     *<li> VertOrientPosition
     *</ul>
     */
    public void checkShapeProperties() {
        assure("Couldn't open document", openSpreadsheetDocument());

        XPropertySet set = (XPropertySet) UnoRuntime.queryInterface(
                                   XPropertySet.class, getInsertedShape());

        try {
            assure("Problems when setting property 'Anchor'",
                   changeProperty(set, "Anchor",
                                  getSpreadsheet().getCellByPosition(5, 5)));
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            assure("Problems when setting property 'Anchor'", false);
        }

        assure("Problems when setting property 'HoriOrientPosition'",
               changeProperty(set, "HoriOrientPosition", new Integer(1000)));

        assure("Problems when setting property 'VertOrientPosition'",
               changeProperty(set, "VertOrientPosition", new Integer(1000)));

        assure("Couldn't close document", closeSpreadsheetDocument());
    }

    public void checkInfluenceOfSpreadsheetChange() {
        assure("Couldn't open document", openSpreadsheetDocument());

        XShape oShape = getInsertedShape();
        XPropertySet sheetProps = (XPropertySet) UnoRuntime.queryInterface(
                                          XPropertySet.class, getSpreadsheet());
        XPropertySet shapeProps = (XPropertySet) UnoRuntime.queryInterface(
                                          XPropertySet.class, oShape);

        String[] previous = getShapeProps(shapeProps, oShape);
        assure("Problems when setting property 'TableLayout'",
               changeProperty(sheetProps, "TableLayout",
                              new Short(com.sun.star.text.WritingMode2.RL_TB)));

        String[] RL_TB = getShapeProps(shapeProps, oShape);
        assure("Problems when setting property 'TableLayout'",
               changeProperty(sheetProps, "TableLayout",
                              new Short(com.sun.star.text.WritingMode2.LR_TB)));

        String[] LR_TB = getShapeProps(shapeProps, oShape);
        assure("Anchor has changed",
               (previous[0].equals(RL_TB[0]) && previous[0].equals(LR_TB[0])));
        assure("HoriOrientPosition has changed",
               (Integer.valueOf(previous[1]).intValue() + Integer.valueOf(
                                                                  RL_TB[1])
                                                                 .intValue() +
                   Integer.valueOf(LR_TB[1]).intValue() == 2099));
        assure("VertOrientPosition has changed",
               (Integer.valueOf(previous[2]).intValue() + Integer.valueOf(
                                                                  RL_TB[2])
                                                                 .intValue() +
                   Integer.valueOf(LR_TB[2]).intValue() == 3*Integer.valueOf(previous[2]).intValue()));
        assure("x-position hasn't changed",
               (previous[3].equals(LR_TB[3]) &&
                   ((Integer.valueOf(previous[3]).intValue() * (-1)) -
                       oShape.getSize().Width != Integer.valueOf(LR_TB[2])
                                                        .intValue())));
        assure("Couldn't close document", closeSpreadsheetDocument());
    }

    protected String[] getShapeProps(XPropertySet set, XShape oShape) {
        String[] reValue = new String[4];

        try {
            reValue[0] = toString(getRealValue(set.getPropertyValue("Anchor")));
            reValue[1] = toString(set.getPropertyValue("HoriOrientPosition"));
            reValue[2] = toString(set.getPropertyValue("VertOrientPosition"));
            reValue[3] = toString(new Integer(oShape.getPosition().X));
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        } catch (com.sun.star.lang.WrappedTargetException e) {
        }

        log.println("\t Anchor :" + reValue[0]);
        log.println("\t HoriOrientPosition :" + reValue[1]);
        log.println("\t VertOrientPosition :" + reValue[2]);
        log.println("\t Shape Position (x,y) : (" + oShape.getPosition().X +
                    "," + oShape.getPosition().Y + ")");

        return reValue;
    }

    /*
     * this method opens a calc document and sets the corresponding class variable xSheetDoc
     */
    protected boolean openSpreadsheetDocument() {
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     (XMultiServiceFactory) param.getMSF());
        boolean worked = true;

        try {
            log.println("creating a sheetdocument");
            xSheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            worked = false;
            e.printStackTrace((PrintWriter) log);
        }

        return worked;
    }

    /*
     * this method closes a calc document and resets the corresponding class variable xSheetDoc
     */
    protected boolean closeSpreadsheetDocument() {
        boolean worked = true;

        log.println("    disposing xSheetDoc ");

        try {
            XCloseable oCloser = (XCloseable) UnoRuntime.queryInterface(
                                         XCloseable.class, xSheetDoc);
            oCloser.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            worked = false;
            log.println("Couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            worked = false;
            log.println("Document already disposed");
        } catch (java.lang.NullPointerException e) {
            worked = false;
            log.println("Couldn't get XCloseable");
        }

        xSheetDoc = null;

        return worked;
    }

    /*
     * This method gets the first Sheet of the SpreadsheetDocument
     *
     */
    protected XSpreadsheet getSpreadsheet() {
        XSpreadsheet oSheet = null;

        log.println("getting sheets");

        XSpreadsheets xSpreadsheets = (XSpreadsheets) xSheetDoc.getSheets();

        log.println("getting a sheet");

        XIndexAccess oIndexAccess = (XIndexAccess) UnoRuntime.queryInterface(
                                            XIndexAccess.class, xSpreadsheets);

        try {
            oSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                             XSpreadsheet.class, oIndexAccess.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace((PrintWriter) log);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace((PrintWriter) log);
        }

        return oSheet;
    }

    /*
     * This method changes a given property to a given value
     * Parameters:
     * @param set : The XPropertySet that contains the properties
     * @param pName : The name of the Property that has to be changed
     * @param pValue : The value, the property should be changed too.
     */
    protected boolean changeProperty(XPropertySet set, String pName,
                                     Object pValue) {
        boolean worked = true;

        try {
            Object oldValue = set.getPropertyValue(pName);

            // for an exception thrown during setting new value
            // to pass it to checkResult method
            Exception exception = null;

            try {
                set.setPropertyValue(pName, pValue);
            } catch (IllegalArgumentException e) {
                exception = e;
            } catch (PropertyVetoException e) {
                exception = e;
            } catch (WrappedTargetException e) {
                exception = e;
            } catch (UnknownPropertyException e) {
                exception = e;
            } catch (RuntimeException e) {
                exception = e;
            }

            // getting result value
            Object resValue = set.getPropertyValue(pName);


            // checking results
            worked = checkResult(set, pName, oldValue, pValue, resValue,
                                 exception);
        } catch (Exception e) {
            System.out.println("Exception occurred while testing property '" +
                               pName + "'");
            e.printStackTrace();
            worked = false;
        }

        return worked;
    }

    /*
     * This methods checks if a property has changed as expected
     * Parameters:
     * @param set : the given XPropertySet
     * @param propName : the name of the property that has been changed
     * @param oldValue : the value of the property before it has been changed
     * @param newValue : the value the property has been set to
     * @param resValue : the value getPropertyValue returned for the property
     * @param exception : the exeption thrown during the change of the property
     */
    protected boolean checkResult(XPropertySet set, String propName,
                                  Object oldValue, Object newValue,
                                  Object resValue, Exception exception)
                           throws Exception {
        XPropertySetInfo info = set.getPropertySetInfo();
        Property prop = info.getPropertyByName(propName);

        oldValue = getRealValue(oldValue);
        newValue = getRealValue(newValue);
        resValue = getRealValue(resValue);

        short attr = prop.Attributes;
        boolean readOnly = (prop.Attributes & PropertyAttribute.READONLY) != 0;
        boolean maybeVoid = (prop.Attributes & PropertyAttribute.MAYBEVOID) != 0;

        //check get-set methods
        if (maybeVoid) {
            log.println("Property " + propName + " is void");
        }

        if (readOnly) {
            log.println("Property " + propName + " is readOnly");
        }

        if (util.utils.isVoid(oldValue) && !maybeVoid) {
            log.println(propName + " is void, but it's not MAYBEVOID");

            return false;
        } else if (oldValue == null) {
            log.println(propName +
                        " has null value, and therefore can't be changed");

            return true;
        } else if (readOnly) {
            // check if exception was thrown
            if (exception != null) {
                if (exception instanceof PropertyVetoException) {
                    // the change of read only prohibited - OK
                    log.println("Property is ReadOnly and wasn't changed");
                    log.println("Property '" + propName + "' OK");

                    return true;
                } else if (exception instanceof IllegalArgumentException) {
                    // the change of read only prohibited - OK
                    log.println("Property is ReadOnly and wasn't changed");
                    log.println("Property '" + propName + "' OK");

                    return true;
                } else if (exception instanceof UnknownPropertyException) {
                    // the change of read only prohibited - OK
                    log.println("Property is ReadOnly and wasn't changed");
                    log.println("Property '" + propName + "' OK");

                    return true;
                } else if (exception instanceof RuntimeException) {
                    // the change of read only prohibited - OK
                    log.println("Property is ReadOnly and wasn't changed");
                    log.println("Property '" + propName + "' OK");

                    return true;
                } else {
                    throw exception;
                }
            } else {
                // if no exception - check that value
                // has not changed
                if (!ValueComparer.equalValue(resValue, oldValue)) {
                    log.println("Read only property '" + propName +
                                "' has changed");

                    try {
                        if (!util.utils.isVoid(oldValue) &&
                                oldValue instanceof Any) {
                            oldValue = AnyConverter.toObject(
                                               new Type(((Any) oldValue).getClass()),
                                               oldValue);
                        }

                        log.println("old = " + toString(oldValue));
                        log.println("new = " + toString(newValue));
                        log.println("result = " + toString(resValue));
                    } catch (com.sun.star.lang.IllegalArgumentException iae) {
                    }

                    return false;
                } else {
                    log.println("Read only property '" + propName +
                                "' hasn't changed");
                    log.println("Property '" + propName + "' OK");

                    return true;
                }
            }
        } else {
            if (exception == null) {
                // if no exception thrown
                // check that the new value is set
                if ((!ValueComparer.equalValue(resValue, newValue)) ||
                        (ValueComparer.equalValue(resValue, oldValue))) {
                    log.println("Value for '" + propName +
                                "' hasn't changed as expected");

                    try {
                        if (!util.utils.isVoid(oldValue) &&
                                oldValue instanceof Any) {
                            oldValue = AnyConverter.toObject(
                                               new Type(((Any) oldValue).getClass()),
                                               oldValue);
                        }

                        log.println("old = " + toString(oldValue));
                        log.println("new = " + toString(newValue));
                        log.println("result = " + toString(resValue));
                    } catch (com.sun.star.lang.IllegalArgumentException iae) {
                    }

                    if (resValue != null) {
                        if ((!ValueComparer.equalValue(resValue, oldValue)) ||
                                (!resValue.equals(oldValue))) {
                            log.println("But it has changed.");

                            return true;
                        } else {
                            return false;
                        }
                    } else {
                        return false;
                    }

                    //tRes.tested(propName, false);
                } else {
                    log.println("Property '" + propName + "' OK");

                    try {
                        if (!util.utils.isVoid(oldValue) &&
                                oldValue instanceof Any) {
                            oldValue = AnyConverter.toObject(
                                               new Type(((Any) oldValue).getClass()),
                                               oldValue);
                        }

                        log.println("old = " + toString(oldValue));
                        log.println("new = " + toString(newValue));
                        log.println("result = " + toString(resValue));
                    } catch (com.sun.star.lang.IllegalArgumentException iae) {
                    }

                    return true;
                }
            } else {
                throw exception;
            }
        }
    }

    /*
     * Returns a String representation of a given object, returns "null" in case of a NullObject
     * @param obj : the given object
     */
    protected String toString(Object obj) {
        return (obj == null) ? "null" : obj.toString();
    }

    /*
     * Returns a human readable String representation for CellRangeAddressable
     */
    protected Object getRealValue(Object obj) {
        Object value = null;

        if (UnoRuntime.queryInterface(XCellRangeAddressable.class, obj) != null) {
            XCellRangeAddressable aCell = (XCellRangeAddressable) UnoRuntime.queryInterface(
                                                  XCellRangeAddressable.class,
                                                  obj);
            value = "Cell in Column " + aCell.getRangeAddress().StartColumn +
                    " and Row " + aCell.getRangeAddress().StartRow;
        } else {
            return obj;
        }

        return value;
    }

    /*
     * This method inserts a RectangleShape into the calc document xSheetDoc and returns the resultung XShape-object
     */
    protected XShape getInsertedShape() {
        XShape insertedShape = null;

        try {
            log.println("getting Drawpages");

            XDrawPagesSupplier oDPS = (XDrawPagesSupplier) UnoRuntime.queryInterface(
                                              XDrawPagesSupplier.class,
                                              xSheetDoc);
            XDrawPages oDP = (XDrawPages) oDPS.getDrawPages();
            XDrawPage firstDrawPage = (XDrawPage) UnoRuntime.queryInterface(
                                              XDrawPage.class,
                                              oDP.getByIndex(0));

            SOfficeFactory SOF = SOfficeFactory.getFactory(
                                         (XMultiServiceFactory) param.getMSF());
            XComponent xComp = (XComponent) UnoRuntime.queryInterface(
                                       XComponent.class, xSheetDoc);
            insertedShape = SOF.createShape(xComp, 5000, 3500, 700, 500,
                                            "Rectangle");
            DrawTools.getShapes(firstDrawPage).add(insertedShape);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't create instance");
            e.printStackTrace((PrintWriter) log);

            return null;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't create instance");
            e.printStackTrace((PrintWriter) log);

            return null;
        }

        return insertedShape;
    }
}