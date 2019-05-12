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



package ifc.sheet;

import lib.MultiPropertyTest;
import util.ValueChanger;

/**
* Testing <code>com.sun.star.sheet.SheetLink</code>
* service properties :
* <ul>
*  <li><code> Url</code></li>
*  <li><code> Filter</code></li>
*  <li><code> FilterOptions</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.sheet.SheetLink
*/
public class _SheetLink extends MultiPropertyTest {

    /**
     *This class is destined to custom test of property <code>Url</code>.
     */
    protected PropertyTester UrlTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            String newValue = (String) ValueChanger.changePValue(oldValue);
            if ( !newValue.startsWith("file://") ) {
                newValue = "file://" + newValue;
            }

            return newValue;
        }
    };

    /**
     * Test property <code>Url</code> using custom <code>PropertyTest</code>.
     */
    public void _Url() {
        testProperty("Url", UrlTester);
    }

    /**
     *This class is destined to custom test of property <code>Filter</code>.
     */
    protected PropertyTester FilterTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            return "StarCalc 4.0";
        }
    };

    /**
     * Test property <code>Filter</code> using custom <code>PropertyTest</code>.
     */
    public void _Filter() {
        testProperty("Filter", FilterTester);
    }
}


