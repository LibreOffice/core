/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XActionLockable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:26:56 $
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

package ifc.document;

import lib.MultiMethodTest;

import com.sun.star.document.XActionLockable;

/**
* Testing <code>com.sun.star.document.XActionLockable</code>
* interface methods :
* <ul>
*  <li><code> isActionLocked()</code></li>
*  <li><code> addActionLock()</code></li>
*  <li><code> removeActionLock()</code></li>
*  <li><code> setActionLocks()</code></li>
*  <li><code> resetActionLocks()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.document.XActionLockable
*/
public class _XActionLockable extends MultiMethodTest {
    public XActionLockable oObj = null;

    /**
     * Method addActionLock called first and then
     * checked value returned by isActionLocked().<p>
     * Has <b> OK </b> status if no runtime exceptions occured
     * and returned value is true.
     */
    public void _isActionLocked() {
        //determines if at least one lock exists
        oObj.addActionLock();
        boolean result = oObj.isActionLocked();
        tRes.tested("isActionLocked()", result);
    }

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occured
    */
    public void _addActionLock() {
        requiredMethod("resetActionLocks()");
        oObj.addActionLock();
        tRes.tested("addActionLock()", true);
    }

    /**
    * Calls the method and check value returned by isActionLocked(). <p>
    * Has <b> OK </b> status if no runtime exceptions occured
    * and returned value is false.
    */
    public void _removeActionLock() {
        requiredMethod("addActionLock()");
        oObj.removeActionLock();
        boolean result = ! oObj.isActionLocked();
        tRes.tested("removeActionLock()", result);
    }

    /**
    * Calls the method with specific value. <p>
    * Has <b> OK </b> status if no runtime exceptions occured
    */
    public void _setActionLocks() {
        oObj.setActionLocks( nLock );
        tRes.tested("setActionLocks()", true);
    }

    final short nLock = 8;

    /**
    * Calls the method and checks returned value.<p>
    * Has <b> OK </b> status if the component is not currently
    * locked and returned value is the same as locks number
    * set by <code>setActionLocks</code> method test.
    */
    public void _resetActionLocks() {
        requiredMethod("setActionLocks()");
        short nLocksBeforeReset = oObj.resetActionLocks();
        boolean result = !oObj.isActionLocked() && nLocksBeforeReset == nLock;
        tRes.tested("resetActionLocks()", result);
    }
}// finish class _XActionLockable

