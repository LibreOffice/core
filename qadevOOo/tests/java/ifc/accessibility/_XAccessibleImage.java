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

package ifc.accessibility;

import lib.MultiMethodTest;

import com.sun.star.accessibility.XAccessibleImage;

/**
 * Testing <code>com.sun.star.accessibility.XAccessibleImage</code>
 * interface methods :
 * <ul>
 *  <li><code>getAccessibleImageDescription()</code></li>
 *  <li><code>getAccessibleImageHeight()</code></li>
 *  <li><code>getAccessibleImageWidth()</code></li>
 * </ul> <p>
 * @see com.sun.star.accessibility.XAccessibleImage
 */
public class _XAccessibleImage extends MultiMethodTest {

    public XAccessibleImage oObj = null;

    /**
     * Just calls the method.
     */
    public void _getAccessibleImageDescription() {
        String descr = oObj.getAccessibleImageDescription();
        log.println("getAccessibleImageDescription(): '" + descr + "'");
        tRes.tested("getAccessibleImageDescription()", descr != null);
    }

    /**
     * Just calls the method.
     */
    public void _getAccessibleImageHeight() {
        int height = oObj.getAccessibleImageHeight();
        log.println("getAccessibleImageHeight(): " + height);
        tRes.tested("getAccessibleImageHeight()", true);
    }

    /**
     * Just calls the method.
     */
    public void _getAccessibleImageWidth() {
        int width = oObj.getAccessibleImageWidth();
        log.println("getAccessibleImageWidth(): " + width);
        tRes.tested("getAccessibleImageWidth()", true);
    }
}