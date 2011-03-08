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

import com.sun.star.text.XTextField;

/**
 * Testing <code>com.sun.star.text.XTextField</code>
 * interface methods :
 * <ul>
 *  <li><code> getPresentation()</code></li>
 * </ul> <p>
 * Test is multithread compilant. <p>
 * @see com.sun.star.text.XTextField
 */
public class _XTextField extends MultiMethodTest{
    public XTextField oObj = null;

    /**
     * Calls the method with <code>true</code> and <code>false</code>
     * parameter. <p>
     * Has <b>OK</b> status if in both cases not <code>null</code>
     * value returned.
     */
    public void _getPresentation() {

        boolean result = true;

        // begin test here
        log.println("getting presentetion with bShowCommand flag...");
        result &= oObj.getPresentation(true) != null;
        log.println("getting presentetion without bShowCommand flag...");
        result &= oObj.getPresentation(false) != null;

        tRes.tested( "getPresentation()", result );

     } // end getPresentation()
}

