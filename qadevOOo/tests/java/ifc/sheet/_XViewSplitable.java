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

import com.sun.star.sheet.XViewSplitable;

/**
* Testing <code>com.sun.star.sheet.XViewSplitable</code>
* interface methods :
* <ul>
*  <li><code> getIsWindowSplit()</code></li>
*  <li><code> getSplitHorizontal()</code></li>
*  <li><code> getSplitVertical()</code></li>
*  <li><code> getSplitColumn()</code></li>
*  <li><code> getSplitRow()</code></li>
*  <li><code> splitAtPosition()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XViewSplitable
*/
public class _XViewSplitable extends MultiMethodTest {

    public XViewSplitable oObj = null;
    protected final int posX = 101, posY = 51;

    /**
    * Test just calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _splitAtPosition(){
        oObj.splitAtPosition(posX, posY);
        tRes.tested("splitAtPosition()", true);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is equal to true. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> splitAtPosition() </code> : that the view was splitted into
    *   individual panes </li>
    * </ul>
    */
    public void _getIsWindowSplit(){
        requiredMethod("splitAtPosition()");
        tRes.tested("getIsWindowSplit()", oObj.getIsWindowSplit());
    }

    /**
    * Test calls the method and compares returned value with value that was
    * set by method <code>splitAtPosition()</code>. <p>
    * Has <b> OK </b> status if returned value is equal to value that was
    * set by method <code>splitAtPosition()</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> splitAtPosition() </code> : to split the view at the
    *  specified position </li>
    * </ul>
    */
    public void _getSplitHorizontal() {
        requiredMethod("splitAtPosition()");
        tRes.tested("getSplitHorizontal()", oObj.getSplitHorizontal() == posX);
    }

    /**
    * Test calls the method and compares returned value with value that was
    * set by method <code>splitAtPosition()</code>. <p>
    * Has <b> OK </b> status if returned value is equal to value that was
    * set by method <code>splitAtPosition()</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> splitAtPosition() </code> : to split the view at the
    *  specified position </li>
    * </ul>
    */
    public void _getSplitVertical() {
        requiredMethod("splitAtPosition()");
        tRes.tested("getSplitVertical()", oObj.getSplitVertical() == posY);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't equal zero. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> splitAtPosition() </code> : to split the view at the
    *  specified position </li>
    * </ul>
    */
    public void _getSplitColumn() {
        requiredMethod("splitAtPosition()");
        tRes.tested("getSplitColumn()", oObj.getSplitColumn() != 0);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't equal zero. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> splitAtPosition() </code> : to split the view at the
    *  specified position </li>
    * </ul>
    */
    public void _getSplitRow() {
        requiredMethod("splitAtPosition()");
        tRes.tested("getSplitRow()", oObj.getSplitRow() != 0);
    }
}  // finish class _XViewSplitable


