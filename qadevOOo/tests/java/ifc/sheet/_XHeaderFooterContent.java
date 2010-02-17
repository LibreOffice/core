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

package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.XHeaderFooterContent;
import com.sun.star.text.XText;

/**
 * Testing <code>com.sun.star.sheet.XHeaderFooterContent</code>
 * interface methods :
 * <ul>
 *  <li><code> getLeftText()</code></li>
 *  <li><code> getCenterText()</code></li>
 *  <li><code> getRightText()</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.sheet.XHeaderFooterContent
 */
public class _XHeaderFooterContent extends MultiMethodTest {

    public XHeaderFooterContent oObj = null;

    /**
    * Test calls the method and checks string obtained from returned value. <p>
    * Has <b>OK</b> status if obtained string is equal to <code>"LEFT"</code>.
    * <p>
    */
    public void _getLeftText() {
        XText left = oObj.getLeftText();
        tRes.tested("getLeftText()",left.getString().equals("LEFT"));
    }

    /**
    * Test calls the method and checks string obtained from returned value. <p>
    * Has <b>OK</b> status if obtained string is equal to <code>"RIGHT"</code>.
    * <p>
    */
    public void _getRightText() {
        XText right = oObj.getRightText();
        tRes.tested("getRightText()",right.getString().equals("RIGHT"));
    }

    /**
    * Test calls the method and checks string obtained from returned value. <p>
    * Has <b>OK</b> status if obtained string is equal to <code>"CENTER"</code>.
    * <p>
    */
    public void _getCenterText() {
        XText center = oObj.getCenterText();
        tRes.tested("getCenterText()",center.getString().equals("CENTER"));
    }

}  // finish class _XHeaderFooterContent


