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



package ifc.table;

import lib.MultiMethodTest;
import util.ValueComparer;

import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XTableChart;

/**
* Testing <code>com.sun.star.table.XTableChart</code>
* interface methods :
* <ul>
*  <li><code> getHasColumnHeaders()</code></li>
*  <li><code> setHasColumnHeaders()</code></li>
*  <li><code> getHasRowHeaders()</code></li>
*  <li><code> setHasRowHeaders()</code></li>
*  <li><code> getRanges()</code></li>
*  <li><code> setRanges()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.table.XTableChart
*/
public class _XTableChart extends MultiMethodTest {

    public XTableChart oObj = null;

    /**
     * Sets the property to <code>false</code> and then check it. <p>
     * Has <b> OK </b> status if the method returns <code>false</code>. <p>
     */
    public void _getHasColumnHeaders() {
        oObj.setHasColumnHeaders(false);
        tRes.tested("getHasColumnHeaders()", !oObj.getHasColumnHeaders() );
    } // getHasColumnHeaders()

    /**
     * Sets the property to <code>true</code> and then check it. <p>
     * Has <b> OK </b> status if the method returns <code>true</code>. <p>
     */
    public void _setHasColumnHeaders() {
        oObj.setHasColumnHeaders(true);
        tRes.tested("setHasColumnHeaders()", oObj.getHasColumnHeaders() );
    } // setHasColumnHeaders()

    /**
     * Sets the property to <code>false</code> and then check it. <p>
     * Has <b> OK </b> status if the method returns <code>false</code>. <p>
     */
    public void _getHasRowHeaders() {
        oObj.setHasRowHeaders(false);
        tRes.tested("getHasRowHeaders()", !oObj.getHasRowHeaders() );
    } // getHasRowHeaders()

    /**
     * Sets the property to <code>true</code> and then check it. <p>
     * Has <b> OK </b> status if the method returns <code>true</code>. <p>
     */
    public void _setHasRowHeaders() {
        oObj.setHasRowHeaders(true);
        tRes.tested("setHasRowHeaders()", oObj.getHasRowHeaders() );
    } // setHasRowHeaders()

    CellRangeAddress[] the_Ranges = null;

    /**
     * Test calls the method and stores the range returned. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> valuie. <p>
     */
    public void _getRanges() {
        the_Ranges = oObj.getRanges();
        tRes.tested("getRanges()", the_Ranges != null );
     } // getRanges()

    /**
     * Changes the first range in range array obtained by
     * <code>getRanges</code> method, then set changed array. <p>
     * Has <b> OK </b> status if range array get is the same as was
     * set. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getRanges() </code> : to have initial ranges </li>
     * </ul>
     */
    public void _setRanges() {
        requiredMethod("getRanges()");
        CellRangeAddress[] tmpRanges = oObj.getRanges();
        tmpRanges[0].EndRow = 1;
        oObj.setRanges(tmpRanges);
        tRes.tested("setRanges()", ValueComparer.equalValue(
                                                tmpRanges,oObj.getRanges()));
        oObj.setRanges(the_Ranges);
     } // getRanges()

} // finish class _XTableChartsSupplier



