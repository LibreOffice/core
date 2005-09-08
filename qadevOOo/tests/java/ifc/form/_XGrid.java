/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XGrid.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:46:30 $
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

package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.form.XGrid;

/**
* Testing <code>com.sun.star.form.XGrid</code>
* interface methods :
* <ul>
*  <li><code> getCurrentColumnPosition()</code></li>
*  <li><code> setCurrentColumnPosition(short nPos)</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XGrid
*/
public class _XGrid extends MultiMethodTest {
    public XGrid oObj = null;
    public short positionValue;

    /**
    * The method called. Then CurrentColumnPosition is obtained and
    * compared with previously changed value.
    * Has <b> OK </b> status if the method successfully returns
    * and the returned value is correct.
    */
    public void _getCurrentColumnPosition() {
        boolean result;

        requiredMethod("setCurrentColumnPosition()");
        log.println("Testing getCurrentColumnPosition()...");
        if (oObj.getCurrentColumnPosition() == positionValue) {
            result = true;
        } else {
            result = false;
        }
        tRes.tested("getCurrentColumnPosition()", result);
    }

    /**
    * The method called. Then CurrentColumnPosition is obtained, changed
    * and saved.
    * Has <b> OK </b> status if no exceptions was occured.
    */
    public void _setCurrentColumnPosition() {
        log.println("Testing setCurrentColumnPosition()...");
        if (oObj.getCurrentColumnPosition() != 0) {
            positionValue = 0;
        } else {
            positionValue = 1;
        }
        oObj.setCurrentColumnPosition(positionValue);
        tRes.tested("setCurrentColumnPosition()", true);
    }

}
