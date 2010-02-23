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

package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.container.XNameAccess;
import com.sun.star.text.XTextEmbeddedObjectsSupplier;

/**
 * Testing <code>com.sun.star.text.XTextEmbeddedObjectsSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getEmbeddedObjects()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XTextEmbeddedObjectsSupplier
 */
public class _XTextEmbeddedObjectsSupplier extends MultiMethodTest {

    public XTextEmbeddedObjectsSupplier oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getEmbeddedObjects(){
        boolean bResult = false;
        XNameAccess oNA = oObj.getEmbeddedObjects();
        bResult = oNA != null ;
        tRes.tested("getEmbeddedObjects()", bResult );
    }

 }  // finish class _XTextEmbeddedObjectsSupplier

