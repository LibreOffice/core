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

package ifc.presentation;

import lib.MultiMethodTest;

import com.sun.star.container.XNameAccess;
import com.sun.star.presentation.XCustomPresentationSupplier;

/**
* Testing <code>com.sun.star.presentation.XCustomPresentationSupplier</code>
* interface methods :
* <ul>
*  <li><code> getCustomPresentations()</code></li>
* </ul> <p>
* @see com.sun.star.presentation.XCustomPresentationSupplier
*/
public class _XCustomPresentationSupplier extends MultiMethodTest {

    public XCustomPresentationSupplier    oObj = null;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getCustomPresentations() {
        XNameAccess NA = oObj.getCustomPresentations();
        tRes.tested("getCustomPresentations()", NA != null);
    }

}


