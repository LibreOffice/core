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



package ifc.ucb;

import lib.MultiMethodTest;

import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XContentProviderFactory;


/**
* Testing <code>com.sun.star.ucb.XContentProviderFactory</code>
* interface methods :
* <ul>
*  <li><code> createContentProvider()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ucb.XContentProviderFactory
*/
public class _XContentProviderFactory extends MultiMethodTest {

    public static XContentProviderFactory oObj = null;

    /**
     * Creates a propvider. <p>
     * Has <b> OK </b> status if the returned value is not
     * <code>null</code>. <p>
     */
    public void _createContentProvider() {
        XContentProvider CP = oObj.createContentProvider
            ("com.sun.star.ucb.ContentProviderFactory");
        tRes.tested("createContentProvider()",CP != null);
    }
}

