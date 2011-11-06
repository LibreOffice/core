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

import com.sun.star.chart.ChartSeriesAddress;

/**
* Testing <code>com.sun.star.chart.ChartTableAddressSupplier</code>
* service properties :
* <ul>
*  <li><code> CategoriesRangeAddress</code></li>
*  <li><code> SeriesAddresses</code></li>
*  <li><code> MainTitleAddress</code></li>
*  <li><code> SubTitleAddress</code></li>
* </ul> <p>
* @see com.sun.star.chart.ChartTableAddressSupplier
*/
public class _ChartTableAddressSupplier extends MultiPropertyTest {

    /**
    * Tests property 'SeriesAddresses'.
    */
    public void _SeriesAddresses() {
        testProperty("SeriesAddresses", new PropertyTester() {
            protected Object getNewValue(String propName, Object oldValue) {
                ChartSeriesAddress[] addr = (ChartSeriesAddress[]) oldValue ;
                ChartSeriesAddress[] newAddr = null ;
                if (addr.length > 1) {
                    newAddr = new ChartSeriesAddress[addr.length - 1] ;
                    System.arraycopy(addr, 0, newAddr, 0, addr.length - 1) ;
                } else {
                    newAddr = addr ;
                }

                return newAddr ;
            }
        }) ;
    }
}

