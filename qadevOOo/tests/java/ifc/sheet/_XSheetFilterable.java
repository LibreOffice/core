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
package ifc.sheet;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XPropertySet;
import com.sun.star.sheet.TableFilterField;
import com.sun.star.sheet.XSheetFilterDescriptor;
import com.sun.star.sheet.XSheetFilterable;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.table.XColumnRowRange;
import com.sun.star.table.XTableRows;
import com.sun.star.uno.UnoRuntime;


public class _XSheetFilterable extends MultiMethodTest {
    public XSheetFilterable oObj;
    protected XSpreadsheet oSheet;
    protected XSheetFilterDescriptor desc;

    protected void before() {
        oSheet = (XSpreadsheet) tEnv.getObjRelation("SHEET");

        if (oSheet == null) {
            log.println("Object relation oSheet is missing");
            log.println("Trying to query the needed Interface");
            oSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                             XSpreadsheet.class, tEnv.getTestObject());

            if (oSheet == null) {
                throw new StatusException(Status.failed(
                                                  "Object relation oSheet is missing"));
            }
        }
    }

    public void _createFilterDescriptor() {
        desc = oObj.createFilterDescriptor(true);

        TableFilterField[] tff = new TableFilterField[2];
        tff[0] = new TableFilterField();
        tff[0].IsNumeric = true;
        tff[0].Field = 0;
        tff[0].NumericValue = 2;
        tff[0].Operator = com.sun.star.sheet.FilterOperator.GREATER_EQUAL;
        tff[1] = new TableFilterField();
        tff[1].IsNumeric = false;
        tff[1].Field = 1;
        tff[1].StringValue = "C";
        tff[1].Operator = com.sun.star.sheet.FilterOperator.LESS;
        desc.setFilterFields(tff);
        tRes.tested("createFilterDescriptor()", true);
    }

    public void _filter() {
        boolean res = true;

        try {
            oSheet.getCellByPosition(0, 0).setValue(1);
            oSheet.getCellByPosition(0, 1).setValue(2);
            oSheet.getCellByPosition(0, 2).setValue(3);
            oSheet.getCellByPosition(1, 0).setFormula("A");
            oSheet.getCellByPosition(1, 1).setFormula("D");
            oSheet.getCellByPosition(1, 2).setFormula("B");
            oObj.filter(desc);

            XColumnRowRange oColumnRowRange = (XColumnRowRange) UnoRuntime.queryInterface(
                                                      XColumnRowRange.class,
                                                      oSheet);
            XTableRows oRows = (XTableRows) oColumnRowRange.getRows();
            XPropertySet rowProp = (XPropertySet) UnoRuntime.queryInterface(
                                           XPropertySet.class,
                                           oRows.getByIndex(0));
            boolean locRes = ((Boolean) rowProp.getPropertyValue("IsVisible")).booleanValue();

            if (locRes) {
                log.println("Row 1 should be invisible after filter()");
                res &= false;
            } else {
                res &= true;
            }

            rowProp = (XPropertySet) UnoRuntime.queryInterface(
                              XPropertySet.class, oRows.getByIndex(1));
            locRes = ((Boolean) rowProp.getPropertyValue("IsVisible")).booleanValue();

            if (locRes) {
                log.println("Row 2 should be invisible after filter()");
                res &= false;
            } else {
                res &= true;
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("couldn't fill cells " + e.getLocalizedMessage());
            res = false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("problems geting Property 'isVisible' " +
                        e.getLocalizedMessage());
            res = false;
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("problems geting Property 'isVisible' " +
                        e.getLocalizedMessage());
            res = false;
        }

        tRes.tested("filter()", res);
    }
}