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

import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.chart.ChartDataRowProperties</code>
* service properties :
* <ul>
*  <li><code> Axis</code></li>
*  <li><code> DataErrorProperties</code></li>
*  <li><code> DataMeanValueProperties</code></li>
*  <li><code> DataRegressionProperties</code></li>
* </ul> <p>
* @see com.sun.star.chart.ChartDataRowProperties
*/
public class _ChartDataRowProperties extends MultiPropertyTest {

    public void _Axis() {
        try {
            Integer axis = (Integer) oObj.getPropertyValue("Axis");
            int newValue;
            if (axis.intValue()==2) {
                newValue = 4;
            } else {
                newValue = 2;
            }
            oObj.setPropertyValue("Axis", new Integer(newValue));
            axis = (Integer) oObj.getPropertyValue("Axis");
            tRes.tested("Axis",axis.intValue()==newValue);
        } catch (Exception e) {
            e.printStackTrace(log);
            tRes.tested("Axis",false);
        }
    }
}

