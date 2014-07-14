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



package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.util.XCancellable;

/**
* Testing <code>com.sun.star.util.XCancellable</code>
* interface methods :
* <ul>
*  <li><code> cancel()</code></li>
* </ul> <p>
* @see com.sun.star.util.XCancellable
*/
public class _XCancellable extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XCancellable oObj = null ;

    /**
     * Just calls the method. <p>
     * Has <b>OK</b> status if no exception has occurred. <p>
     */
    public void _cancel() {
        oObj.cancel() ;

        tRes.tested("cancel()", true) ;
    }

    /**
     * Forces object recreation.
     */
    public void after() {
        disposeEnvironment() ;
    }

}  // finish class _XCancellable

