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



package ifc.lang;

import lib.MultiMethodTest;

import com.sun.star.lang.XEventListener;

/**
* Testing <code>com.sun.star.lang.XEventListener</code>
* interface methods :
* <ul>
*  <li><code> disposing()</code></li>
* </ul> <p>
* Tests nothing, all methods has <b>OK</b> status.
* @see com.sun.star.lang.XEventListener
*/
public class _XEventListener extends MultiMethodTest {

    public XEventListener oObj = null;

    /**
    * Nothing to test. Always has <b>OK</b> status.
    */
    public void _disposing() {
        log.println("The method 'disposing'");
        log.println("gets called when the broadcaster is about to be"+
            " disposed.") ;
        log.println("All listeners and all other objects which reference "+
            "the broadcaster should release the references.");
        log.println("So there is nothing to test here");
        tRes.tested("disposing()",true);

    }

} //EOF of XEventListener

