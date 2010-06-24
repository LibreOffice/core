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
 * Test is <b> NOT </b> multithread compilant. <p>
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
     * Has <b>OK</b> status if set and get arays are equal.
     */
    public void _setColumns(){

        TextColumn newCol = new TextColumn(5,1,1);
        TextColumn[] cols = {newCol};
        oObj.setColumns(cols);
        TextColumn[] gCols = oObj.getColumns();
        tRes.tested("setColumns()",util.ValueComparer.equalValue(cols, gCols));
    }

}  // finish class _XTextColumns


