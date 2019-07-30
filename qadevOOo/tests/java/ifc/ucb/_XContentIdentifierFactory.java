/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package ifc.ucb;

import lib.MultiMethodTest;
import util.utils;

import com.sun.star.ucb.XContentIdentifier;
import com.sun.star.ucb.XContentIdentifierFactory;

/**
* Testing <code>com.sun.star.ucb.XContentIdentifierFactory</code>
* interface methods :
* <ul>
*  <li><code> createContentIdentifier()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
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
     * Has <b> OK </b> status if content identifier and provider scheme are
     * proper. <p>
     */
    public void _createContentIdentifier() {
        // creating a content identifier string - tmp url
        String tmpDir = utils.getOfficeTemp(tParam.getMSF());

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
