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

package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.util.XMergeable;


/**
 * Testing <code>com.sun.star.util.XMergeable</code>
 * interface methods :
 * <ul>
 *  <li><code> merge()</code></li>
 *  <li><code> getIsMerged()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.util.XMergeable
 */
public class _XMergeable extends MultiMethodTest {

    public XMergeable oObj = null;                // oObj filled by MultiMethodTest

    /**
     * Checks the property <code>IsMerged</code> and sets it
     * to the opposite value. <p>
     * Has <b> OK </b> status if the property value has changed. <p>
     */
    public void _getIsMerged () {

        boolean isMerged = oObj.getIsMerged();
        oObj.merge(!isMerged);
        tRes.tested("getIsMerged()", isMerged != oObj.getIsMerged());
    }

    /**
     * Checks the property <code>IsMerged</code> and sets it
     * to the opposite value. <p>
     * Has <b> OK </b> status if the property value has changed. <p>
     */
    public void _merge () {

        boolean isMerged = oObj.getIsMerged();
        oObj.merge(!isMerged);
        tRes.tested("merge()", isMerged != oObj.getIsMerged());
    }
} // finisch class _XMergeable


