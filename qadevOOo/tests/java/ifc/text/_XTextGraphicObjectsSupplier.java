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
import com.sun.star.text.XTextGraphicObjectsSupplier;

/**
 * Testing <code>com.sun.star.text.XTextGraphicObjectsSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getGraphicObjects()</code></li>
 * </ul> <p>
 *
 * The component <b>must have</b> the graphic object with
 * name  'SwXTextDocument'. <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XTextGraphicObjectsSupplier
 */
public class _XTextGraphicObjectsSupplier extends MultiMethodTest {

    public static XTextGraphicObjectsSupplier oObj = null;

    /**
     * Gets graphic objects collection from the component, and checks
     * if the object with name 'SwXTextDocument' exists. <p>
     * Has <b>OK</b> status if the object exists.
     */
    public void _getGraphicObjects() {
        boolean res = false;

        XNameAccess the_graphics = oObj.getGraphicObjects();
        res = the_graphics.hasByName("SwXTextDocument");

        tRes.tested("getGraphicObjects()",res);
    }

}  // finish class _XTextGraphicObjectsSupplier

