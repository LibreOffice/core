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

import com.sun.star.sheet.DataPilotFieldAutoShowInfo;
import com.sun.star.sheet.DataPilotFieldLayoutInfo;
import com.sun.star.sheet.DataPilotFieldReference;
import com.sun.star.sheet.DataPilotFieldSortInfo;
import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.sheet.DataPilotField</code>
* service properties :
* <ul>
*  <li><code> Orientation</code></li>
*  <li><code> Function</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.sheet.DataPilotField
*/
public class _DataPilotField extends MultiPropertyTest {

    /**
     *This class is destined to custom test of property <code>SortInfo</code>.
     */
    protected PropertyTester SortInfoTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue) {
            DataPilotFieldSortInfo sortInfo = new DataPilotFieldSortInfo();
        sortInfo.Mode = com.sun.star.sheet.DataPilotFieldSortMode.MANUAL;
            sortInfo.IsAscending = false;
            sortInfo.Field = "COL1";
            log.println("Having Value and returning a new one");
            return sortInfo.equals(oldValue) ? null : sortInfo;
        }
    };

    /**
     * Test property <code>SortInfo</code> using custom <code>PropertyTest</code>.
     */
    public void _SortInfo() {
        testProperty("SortInfo", SortInfoTester);
    }

    public void _HasSortInfo() {
        requiredMethod("SortInfo");
        testProperty("HasSortInfo");
    }

    /**
     *This class is destined to custom test of property <code>LayoutInfo</code>.
     */
    protected PropertyTester LayoutInfoTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue) {
            DataPilotFieldLayoutInfo layoutInfo = new DataPilotFieldLayoutInfo();
            layoutInfo.LayoutMode = com.sun.star.sheet.DataPilotFieldLayoutMode.OUTLINE_SUBTOTALS_BOTTOM;
            layoutInfo.AddEmptyLines = true;
            log.println("Having Value and returning a new one");
            return layoutInfo.equals(oldValue) ? null : layoutInfo;
        }
    };

    /**
     * Test property <code>LayoutInfo</code> using custom <code>PropertyTest</code>.
     */
    public void _LayoutInfo() {
        testProperty("LayoutInfo", LayoutInfoTester);
    }

    public void _HasLayoutInfo() {
        requiredMethod("LayoutInfo");
        testProperty("HasLayoutInfo");
    }

    /**
     *This class is destined to custom test of property <code>AutoShowInfo</code>.
     */
    protected PropertyTester AutoShowInfoTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue) {
            DataPilotFieldAutoShowInfo AutoShowInfo = new DataPilotFieldAutoShowInfo();
            AutoShowInfo.DataField = "Col1";
            AutoShowInfo.IsEnabled = true;
            log.println("Having Value and returning a new one");
            return AutoShowInfo.equals(oldValue) ? null : AutoShowInfo;
        }
    };

    /**
     * Test property <code>AutoShowInfo</code> using custom <code>PropertyTest</code>.
     */
    public void _AutoShowInfo() {
        testProperty("AutoShowInfo", AutoShowInfoTester);
    }

    public void _HasAutoShowInfo() {
        requiredMethod("AutoShowInfo");
        testProperty("HasAutoShowInfo");
    }

    /**
     *This class is destined to custom test of property <code>Reference</code>.
     */
    protected PropertyTester ReferenceTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue) {
            DataPilotFieldReference Reference = new DataPilotFieldReference();
            Reference.ReferenceField="Col1";
            Reference.ReferenceItemType = com.sun.star.sheet.DataPilotFieldReferenceItemType.NAMED;
            log.println("Having Value and returning a new one");
            return Reference.equals(oldValue) ? null : Reference;
        }
    };

    /**
     * Test property <code>Reference</code> using custom <code>PropertyTest</code>.
     */
    public void _Reference() {
        testProperty("Reference", ReferenceTester);
    }

    public void _HasReference() {
        requiredMethod("Reference");
        testProperty("HasReference");
    }

    /*
     * this property can only set a true value to false, fo ungrouping purposes
     */
    public void _IsGroupField() {
        boolean result = true;
        try {
            Boolean propValue = (Boolean) oObj.getPropertyValue ("IsGroupField");
            if (propValue.booleanValue ()) {
                oObj.setPropertyValue ("IsGroupField", Boolean.FALSE);
                Boolean propV = (Boolean) oObj.getPropertyValue ("IsGroupField");
                result = !propV.booleanValue ();
            }
        } catch (Exception e) {
            log.println ("Exception while getting Property 'IsGroupField'"+e);
            result = false;
        }
        tRes.tested ("IsGroupField",result);
    }
}


