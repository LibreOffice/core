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
import com.sun.star.chart.XTwoAxisYSupplier;

/**
* Testing <code>com.sun.star.chart.XTwoAxisYSupplier</code>
* interface methods :
* <ul>
*  <li><code> getSecondaryYAxis()</code></li>
* </ul> <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.chart.XTwoAxisYSupplier
*/
public class _XTwoAxisYSupplier extends MultiMethodTest {

    public XTwoAxisYSupplier oObj = null;
    boolean            result = true;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getSecondaryYAxis() {
        result = true;

        XPropertySet SecYAxis = oObj.getSecondaryYAxis();
        result = (SecYAxis != null);

        tRes.tested("getSecondaryYAxis()", result);
    }

    /**
    * Forces object environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }

} // EOF XTwoAxisYSupplier


