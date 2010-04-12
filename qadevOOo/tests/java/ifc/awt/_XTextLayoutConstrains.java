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

package ifc.awt;

import lib.MultiMethodTest;

import com.sun.star.awt.Size;
import com.sun.star.awt.XTextLayoutConstrains;

/**
* Testing <code>com.sun.star.awt.XTextLayoutConstrains</code>
* interface methods:
* <ul>
*  <li><code> getMinimumSize() </code></li>
*  <li><code> getColumnsAndLines() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XTextLayoutConstrains
*/
public class _XTextLayoutConstrains extends MultiMethodTest {
    public XTextLayoutConstrains oObj = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if both returned size fields are not equal to zero.
    */
    public void _getMinimumSize() {
        short nCols = 0;
        short nLines = 0;
        Size mSize = oObj.getMinimumSize(nCols,nLines);
        boolean res = ( (mSize.Height != 0) && (mSize.Width != 0) );
        if (res == false) {
            log.println("mSize.height: " + mSize.Height);
            log.println("mSize.width: " + mSize.Width);
        }
        tRes.tested("getMinimumSize()", res);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if both returned values are not equal to zero.
    */
    public void _getColumnsAndLines() {
        short[] nCols = new short[1];
        short[] nLines = new short[1];
        oObj.getColumnsAndLines(nCols,nLines);
        boolean res = ( (nCols[0] != 0) && (nLines[0] != 0) );
        if (res == false) {
            log.println("nCols: " + nCols[0]);
            log.println("nLines: " + nLines[0]);
        }
        tRes.tested("getColumnsAndLines()",res);
    }
}

