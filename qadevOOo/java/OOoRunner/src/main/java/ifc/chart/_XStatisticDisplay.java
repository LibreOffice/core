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



package ifc.chart;

import lib.MultiMethodTest;

import com.sun.star.beans.XPropertySet;
import com.sun.star.chart.XStatisticDisplay;

/**
* Testing <code>com.sun.star.chart.XStatisticDisplay</code>
* interface methods :
* <ul>
*  <li><code> getMinMaxLine()</code></li>
*  <li><code> getUpBar()</code></li>
*  <li><code> getDownBar()</code></li>
* </ul> <p>
* @see com.sun.star.chart.XStatisticDisplay
*/
public class _XStatisticDisplay extends MultiMethodTest {

    public XStatisticDisplay oObj = null;
    boolean                   result = true;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getMinMaxLine() {
        result = true;

        XPropertySet MinMaxLine = oObj.getMinMaxLine();
        result = (MinMaxLine != null);

        tRes.tested("getMinMaxLine()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getUpBar() {
        result = true;

        XPropertySet UpBar = oObj.getUpBar();
        result = (UpBar != null);

        tRes.tested("getUpBar()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getDownBar() {
        result = true;

        XPropertySet DownBar = oObj.getDownBar();
        result = (DownBar != null);

        tRes.tested("getDownBar()", result);
    }

} // EOF XStatisticDisplay


