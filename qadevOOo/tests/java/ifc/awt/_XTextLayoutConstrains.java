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
* Test is <b> NOT </b> multithread compliant. <p>
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
        if (!res) {
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
        if (!res) {
            log.println("nCols: " + nCols[0]);
            log.println("nLines: " + nLines[0]);
        }
        tRes.tested("getColumnsAndLines()",res);
    }
}

