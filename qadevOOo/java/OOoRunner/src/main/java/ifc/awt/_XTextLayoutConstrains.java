/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

