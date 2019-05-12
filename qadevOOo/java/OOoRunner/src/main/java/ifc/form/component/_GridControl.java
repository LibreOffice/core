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

/**
* Testing <code>com.sun.star.form.component.GridControl</code>
* service properties :
* <ul>
*  <li><code> Border</code></li>
*  <li><code> Enabled</code></li>
*  <li><code> FontDescriptor</code></li>
*  <li><code> RowHeight</code></li>
*  <li><code> Tabstop</code></li>
*  <li><code> TextColor</code></li>
* </ul>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.form.component.GridControl
*/
public class _GridControl extends MultiPropertyTest {

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _RowHeight() {
        testProperty("RowHeight", new PropertyTester() {
            protected Object getNewValue(String prop, Object old) {
                return util.utils.isVoid(old) ? new Integer(11) :
                    super.getNewValue(prop, old) ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _Tabstop() {
        testProperty("Tabstop", new PropertyTester() {
            protected Object getNewValue(String prop, Object old) {
                return util.utils.isVoid(old) ? new Boolean(true) :
                    super.getNewValue(prop, old) ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _TextColor() {
        testProperty("TextColor", new PropertyTester() {
            protected Object getNewValue(String prop, Object old) {
                return util.utils.isVoid(old) ? new Integer(342) :
                    super.getNewValue(prop, old) ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _BorderColor() {
        testProperty("BorderColor", new PropertyTester() {
            protected Object getNewValue(String prop, Object old) {
                return util.utils.isVoid(old) ? new Integer(342) :
                    super.getNewValue(prop, old) ;
            }
        }) ;
    }
}  // finish class _GridControl


