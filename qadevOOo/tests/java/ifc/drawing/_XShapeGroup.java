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



package ifc.drawing;

import lib.MultiMethodTest;

import com.sun.star.drawing.XShapeGroup;

/**
* Testing <code>com.sun.star.drawing.XShapeGroup</code>
* interface methods :
* <ul>
*  <li><code> enterGroup()</code></li>
*  <li><code> leaveGroup()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XShapeGroup
*/
public class _XShapeGroup extends MultiMethodTest {
    public XShapeGroup oObj = null;

    /**
    * The method called.
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _enterGroup() {
        log.println("Testing enterGroup()...");
        oObj.enterGroup();
        tRes.tested("enterGroup()", true);
    }

    /**
    * The method called.
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _leaveGroup() {
        log.println("Testing leaveGroup()...");
        oObj.leaveGroup();
        tRes.tested("leaveGroup()", true);
    }

} // end of _XShapeGroup

