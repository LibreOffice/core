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

package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.text.TextColumn;
import com.sun.star.text.XTextColumns;

/**
 * Testing <code>com.sun.star.text.XTextColumns</code>
 * interface methods :
 * <ul>
 *  <li><code> getReferenceValue()</code></li>
 *  <li><code> getColumnCount()</code></li>
 *  <li><code> setColumnCount()</code></li>
 *  <li><code> getColumns()</code></li>
 *  <li><code> setColumns()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XTextColumns
 */
public class _XTextColumns extends MultiMethodTest {

    public XTextColumns oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns
     * positive value.
     */
    public void _getColumnCount(){

        short howmuch = oObj.getColumnCount();
        tRes.tested("getColumnCount()",howmuch >=0);
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getColumns(){
        TextColumn[] cols = oObj.getColumns();
        tRes.tested("getColumns()",cols != null);
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns
     * positive value.
     */
    public void _getReferenceValue(){

        int ref = oObj.getReferenceValue();
        tRes.tested("getReferenceValue()",ref >0);
    }

    /**
     * Sets the column count property to some value
     * then checks it by <code>getColumnCount</code> method. <p>
     *
     * Has <b>OK</b> status if set and get values are equal.
     */
    public void _setColumnCount(){

        oObj.setColumnCount((short) 3);
        short howmuch = oObj.getColumnCount();
        tRes.tested("setColumnCount()",howmuch == 3);
    }

    /**
     * Sets columns to some array
     * then checks it by <code>getColumns</code> method. <p>
     *
     * Has <b>OK</b> status if set and get arrays are equal.
     */
    public void _setColumns(){

        TextColumn newCol = new TextColumn(5,1,1);
        TextColumn[] cols = {newCol};
        oObj.setColumns(cols);
        TextColumn[] gCols = oObj.getColumns();
        tRes.tested("setColumns()",util.ValueComparer.equalValue(cols, gCols));
    }

}  // finish class _XTextColumns


