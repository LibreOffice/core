/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    protected static final int posX = 101, posY = 51;

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


