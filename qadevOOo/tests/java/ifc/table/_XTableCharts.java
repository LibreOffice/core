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

package ifc.table;

import lib.MultiMethodTest;

import com.sun.star.awt.Rectangle;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XTableCharts;

/**
* Testing <code>com.sun.star.table.XTableCharts</code>
* interface methods :
* <ul>
*  <li><code> addNewByName()</code></li>
*  <li><code> removeByName()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'ADDR'</code>
*  (of type <code>com.sun.star.table.CellRangeAddress[]</code>):
*   data source ranges for chart creating. </li>
*  <li> <code>'RECT'</code>
*  (of type <code>com.sun.star.awt.Rectangle</code>):
*   the position of chart </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.table.XTableCharts
*/
public class _XTableCharts extends MultiMethodTest {

    public XTableCharts oObj = null;

    /**
     * Adds a new chart into table with specified name . <p>
     * Has <b> OK </b> status if <code>hasByName()</code> returns
     * <code>true</code>. <p>
     */
    public void _addNewByName() {

        CellRangeAddress[] the_Range = (CellRangeAddress[])
            tEnv.getObjRelation("ADDR");
        Rectangle the_rect = (Rectangle) tEnv.getObjRelation("RECT");
        oObj.addNewByName("XTableCharts",the_rect,the_Range,true,true);
        tRes.tested("addNewByName()", oObj.hasByName("XTableCharts") );

    } // addNewByName()

    /**
     * Removes the chart added before. <p>
     * Has <b> OK </b> status if <code>hasByName()</code> returns
     * <code>false</code>. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> addNewByName() </code> : chart must be added first </li>
     * </ul>
     */
    public void _removeByName() {
        requiredMethod("addNewByName()") ;

        oObj.removeByName("XTableCharts");
        tRes.tested("removeByName()", !oObj.hasByName("XTableCharts") );

    } // removeByName()

} // finish class _XTableCharts



