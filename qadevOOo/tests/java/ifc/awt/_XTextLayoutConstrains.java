/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XTextLayoutConstrains.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:01:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

