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



package ifc.form.component;

import lib.MultiPropertyTest;
import util.dbg;

import com.sun.star.form.TabulatorCycle;
import com.sun.star.uno.Enum;

/**
* Testing <code>com.sun.star.form.component.DatabaseForm</code>
* service properties:
* <ul>
*  <li><code> MasterFields</code></li>
*  <li><code> DetailFields</code></li>
*  <li><code> Cycle</code></li>
*  <li><code> NavigationBarMode</code></li>
*  <li><code> AllowInserts</code></li>
*  <li><code> AllowUpdates</code></li>
*  <li><code> AllowDeletes</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.form.component.DatabaseForm
*/
public class _DatabaseForm extends MultiPropertyTest {

    /**
    * In this property test only debugging information output
    * is customized.
    */
    public void _NavigationBarMode() {
        dbg.printPropertyInfo(oObj, "NavigationBarMode");
        testProperty("NavigationBarMode", new PropertyTester() {
            public String toString(Object obj) {
                if (obj == null) {
                    return "null";
                } else {
                    return "(" + obj.getClass().toString() + ")"
                            + ((Enum)obj).getValue();
                }
            }
        });
    }

    /**
    * This property new value is always fixed and debugging
    * information output is customized.
    */
    public void _Cycle() {
        dbg.printPropertyInfo(oObj, "Cycle");
        testProperty("Cycle", new PropertyTester() {
            public Object getNewValue(String propName, Object oldValue) {
                return TabulatorCycle.CURRENT;
            }

            public String toString(Object obj) {
                if (obj == null) {
                    return "null";
                } else {
                    return "(" + obj.getClass().toString() + ")"
                            + ((Enum)obj).getValue();
                }
            }
        });
    }
}

