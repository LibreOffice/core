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
import util.utils;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;

/**
* Testing <code>com.sun.star.ucb.XContentIdentifierFactory</code>
* interface methods :
* <ul>
*  <li><code> createContentIdentifier()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.ucb.XContentIdentifierFactory
*/
public class _XContentIdentifierFactory extends MultiMethodTest {
    /**
     * Contains the tested object.
     */
    public XContentIdentifierFactory oObj;

    /**
     * Calls the tested method with a file url and
     * gets an identifier. <p>
     * Has <b> OK </b> status if content identifier an provider sheme are
     * proper. <p>
     */
    public void _createContentIdentifier() {
        // creating a content identifier string - tmp url
        String tmpDir = utils.getOfficeTemp((XMultiServiceFactory)tParam.getMSF());

        String contentId = utils.getFullURL(tmpDir);

        // the scheme is file
        String scheme = "file";

        XContentIdentifier identifier = oObj.createContentIdentifier(contentId);

        // verifying results
        String resId = identifier.getContentIdentifier();
        String resScheme = identifier.getContentProviderScheme();

        tRes.tested("createContentIdentifier()", contentId.equals(resId)
                && scheme.equals(resScheme));
    }
}
