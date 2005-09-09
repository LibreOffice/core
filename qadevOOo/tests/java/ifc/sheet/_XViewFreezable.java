/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XViewFreezable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:00:03 $
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

package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.XViewFreezable;

/**
* Testing <code>com.sun.star.sheet.XViewFreezable</code>
* interface methods :
* <ul>
*  <li><code> hasFrozenPanes()</code></li>
*  <li><code> freezeAtPosition()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XViewFreezable
*/
public class _XViewFreezable extends MultiMethodTest {

    public XViewFreezable oObj = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _freezeAtPosition() {
        oObj.freezeAtPosition(2, 2);
        tRes.tested("freezeAtPosition()", true);
    }

    /**
    * Test calls the method, checks returned value, unfreezes panes, calls the
    * method and checks returned value again. <p>
    * Has <b> OK </b> status if returned value is true after firts call and
    * returned value is false after second. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> freezeAtPosition() </code> : to freeze panes with the
    *   specified number of columns and rows </li>
    * </ul>
    */
    public void _hasFrozenPanes() {
        requiredMethod("freezeAtPosition()");
        boolean result = oObj.hasFrozenPanes();
        oObj.freezeAtPosition(0,0);
        result &= !oObj.hasFrozenPanes();
        tRes.tested("hasFrozenPanes()", result);
    }


}  // finish class _XViewFreezable


