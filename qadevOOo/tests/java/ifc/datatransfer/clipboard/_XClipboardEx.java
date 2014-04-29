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



package ifc.datatransfer.clipboard;

import lib.MultiMethodTest;

import com.sun.star.datatransfer.clipboard.XClipboardEx;

/**
* Testing <code>com.sun.star.datatransfer.clipboard.XClipboardEx</code>
* interface methods :
* <ul>
*  <li><code> getRenderingCapabilities()</code></li>
* </ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.datatransfer.clipboard.XClipboardEx
*/
public class _XClipboardEx extends MultiMethodTest {

    public XClipboardEx oObj;

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred.
    */
    public void _getRenderingCapabilities() {
        byte caps = oObj.getRenderingCapabilities();
        tRes.tested("getRenderingCapabilities()", true);
    }
}
