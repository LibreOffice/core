/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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