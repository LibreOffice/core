/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XBoundControl.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:45:03 $
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

import com.sun.star.form.XBoundControl;

/**
* Testing <code>com.sun.star.form.XBoundControl</code>
* interface methods:
* <ul>
*  <li><code> getLock() </code></li>
*  <li><code> setLock() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XBoundControl
*/
public class _XBoundControl extends MultiMethodTest {
    public XBoundControl oObj = null;
    public boolean defaultState = false;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _getLock() {
        boolean res = false;
        defaultState = oObj.getLock();
        res = true;
        tRes.tested("getLock()",res);
    }

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if method locks input (set and gotten values are
    * equal). <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getLock() </code>: determines whether the input is
    *  currently locked or not </li>
    * </ul>
    */
    public void _setLock() {
        boolean res = false;
        boolean toSet = !defaultState;

        requiredMethod("getLock()");
        oObj.setLock(toSet);
        res = (oObj.getLock() != defaultState);
        oObj.setLock(defaultState);
        tRes.tested("setLock()", res);
    }

} //EOF of XBoundControl

