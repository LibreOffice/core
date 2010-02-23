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

package ifc.document;

import lib.MultiMethodTest;

import com.sun.star.document.XEmbeddedObjectSupplier;

/**
* Testing <code>com.sun.star.document.XEmbeddedObjectSupplier</code>
* interface methods :
* <ul>
*  <li><code> getEmbeddedObject()</code></li>
* </ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.document.XEmbeddedObjectSupplier
*/
public class _XEmbeddedObjectSupplier extends MultiMethodTest {

    public XEmbeddedObjectSupplier oObj = null;

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if not <code>null</code> value returned.
    */
    public void _getEmbeddedObject() {
        tRes.tested("getEmbeddedObject()",
                oObj.getEmbeddedObject() != null);
    }
}  // finish class _XEmbeddedObjectSupplier

