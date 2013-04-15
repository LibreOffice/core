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
            oSheet = UnoRuntime.queryInterface(
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

            XColumnRowRange oColumnRowRange = UnoRuntime.queryInterface(
                                                      XColumnRowRange.class,
                                                      oSheet);
            XTableRows oRows = oColumnRowRange.getRows();
            XPropertySet rowProp = UnoRuntime.queryInterface(
                                           XPropertySet.class,
                                           oRows.getByIndex(0));
            boolean locRes = ((Boolean) rowProp.getPropertyValue("IsVisible")).booleanValue();

            if (locRes) {
                log.println("Row 1 should be invisible after filter()");
                res &= false;
            } else {
                res &= true;
            }

            rowProp = UnoRuntime.queryInterface(
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
            log.println("problems getting Property 'isVisible' " +
                        e.getLocalizedMessage());
            res = false;
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("problems getting Property 'isVisible' " +
                        e.getLocalizedMessage());
            res = false;
        }

        tRes.tested("filter()", res);
    }
}
