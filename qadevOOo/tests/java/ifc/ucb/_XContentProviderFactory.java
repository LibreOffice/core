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

