/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
