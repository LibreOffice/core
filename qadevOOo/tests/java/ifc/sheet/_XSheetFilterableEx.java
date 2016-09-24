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

import com.sun.star.sheet.TableFilterField;
import com.sun.star.sheet.XSheetFilterDescriptor;
import com.sun.star.sheet.XSheetFilterable;
import com.sun.star.sheet.XSheetFilterableEx;
import com.sun.star.uno.UnoRuntime;


/**
 * Interface test for XSheetFilterableEx
 *
 * methods:
 *
 *  createFilterDescriptorByObject
 *
 */
public class _XSheetFilterableEx extends MultiMethodTest {
    public XSheetFilterableEx oObj = null;

    /**
     * queries a XSheetFilterable from the parent Object and then creates
     * a XSheetFilterDescriptor with it.
     * Returns OK if the Descriptor isn't null and checkFilterDescriptor returns true
     */

    public void _createFilterDescriptorByObject() {
        boolean res = true;
        Object parent = tEnv.getTestObject();
        XSheetFilterable aFilterable = UnoRuntime.queryInterface(
                                               XSheetFilterable.class, parent);
        XSheetFilterDescriptor desc = oObj.createFilterDescriptorByObject(
                                              aFilterable);

        if (desc != null) {
            res &= checkFilterDescriptor(desc);
        } else {
            log.println("gained XSheetFilterDescriptor is null");
            res &= false;
        }
        tRes.tested("createFilterDescriptorByObject()",res);
    }

    /**
     * returns true if the Method getFilterFields() returns a non empty array
     * and all Fields can be gained without exception
     */

    public boolean checkFilterDescriptor(XSheetFilterDescriptor desc) {
        TableFilterField[] fields = desc.getFilterFields();
        boolean res = true;

        if (fields.length == 0) {
            log.println("The gained Descriptor is empty");
            res &= false;
        } else {
               log.println("Found "+fields.length+" TableFields");
        }

        for (int k = 0; k < fields.length; k++) {
            log.println("StringValue(" + k + "): " + fields[k].StringValue);
            log.println("IsNumeric(" + k + "): " + fields[k].IsNumeric);
            log.println("NumericValue(" + k + "): " + fields[k].NumericValue);
            log.println("Field(" + k + "): " + fields[k].Field);
            log.println("Connection(" + k + "): " + fields[k].Connection);
            log.println("Operator(" + k + "): " + fields[k].Operator);
        }

        return res;
    }
}
