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
import util.ValueComparer;

import com.sun.star.sheet.TableFilterField;
import com.sun.star.sheet.XSheetFilterDescriptor;

/**
* Testing <code>com.sun.star.sheet.XSheetFilterDescriptor</code>
* interface methods :
* <ul>
*  <li><code> getFilterFields()</code></li>
*  <li><code> setFilterFields()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetFilterDescriptor
*/
public class _XSheetFilterDescriptor extends MultiMethodTest {

    public XSheetFilterDescriptor oObj = null;
    public TableFilterField[] TFF = null;

    /**
    * Test calls the method, stores and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getFilterFields(){
        TFF = oObj.getFilterFields();
        tRes.tested("getFilterFields()", TFF != null );
    }

    /**
    * Test sets new value of the definitions of the filter fields,
    * gets the current definitions of the filter fields and compares
    * returned value with value that was stored by method
    * <code>getFilterFields()</code>. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getFilterFields() </code> : to have the current definitions
    *  of the filter fields </li>
    * </ul>
    */
    public void _setFilterFields() {
        requiredMethod("getFilterFields()");
        TableFilterField[] newTFF = new TableFilterField[1];
        newTFF[0] = new TableFilterField();
        oObj.setFilterFields(newTFF);
        newTFF = oObj.getFilterFields();
        tRes.tested("setFilterFields()", ! ValueComparer.equalValue(TFF, newTFF));
    }

}  // finish class _XSheetFilterDescriptor


