/*************************************************************************
 *
 *  $RCSfile: _XSheetFilterable.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:05:06 $
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