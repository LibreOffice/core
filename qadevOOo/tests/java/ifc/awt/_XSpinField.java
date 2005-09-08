/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XSpinField.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:00:27 $
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

import com.sun.star.awt.SpinEvent;
import com.sun.star.awt.XSpinField;
import com.sun.star.awt.XSpinListener;
import com.sun.star.lang.EventObject;

/**
* Testing <code>com.sun.star.awt.XSpinField</code>
* interface methods :
* <ul>
*  <li><code> addSpinListener()</code></li>
*  <li><code> removeSpinListener()</code></li>
*  <li><code> up()</code></li>
*  <li><code> down()</code></li>
*  <li><code> first()</code></li>
*  <li><code> last()</code></li>
*  <li><code> enableRepeat()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XSpinField
*/
public class _XSpinField extends MultiMethodTest {

    public XSpinField oObj = null;

    /**
    * Listener implementation which set flags on appropriate
    * listener methods calls.
    */
    protected class TestListener implements XSpinListener {
        public boolean upFl = false ;
        public boolean downFl = false ;
        public boolean firstFl = false ;
        public boolean lastFl = false ;

        public void up(SpinEvent e) {
            upFl = true ;
        }
        public void down(SpinEvent e) {
            downFl = true ;
        }
        public void first(SpinEvent e) {
            firstFl = true ;
        }
        public void last(SpinEvent e) {
            lastFl = true ;
        }
        public void disposing(EventObject e) {}
    }

    private TestListener listener = new TestListener() ;

    /**
    * Just adds a listener. <p>
    * Has <b>OK</b> status if no runtime exceptions occured.
    */
    public void _addSpinListener() {
        oObj.addSpinListener(listener) ;

        tRes.tested("addSpinListener()", true) ;
    }

    /**
    * Calls the method. <p>
    * Has <b>OK</b> status if the appropriate listener method
    * was called. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addSpinListener </code> </li>
    * </ul>
    */
    public void _up() {
        requiredMethod("addSpinListener()") ;

        oObj.up() ;
        shortWait();

        tRes.tested("up()", listener.upFl) ;
    }

    /**
    * Calls the method. <p>
    * Has <b>OK</b> status if the appropriate listener method
    * was called. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addSpinListener </code> </li>
    * </ul>
    */
    public void _down() {
        requiredMethod("addSpinListener()") ;

        oObj.down() ;
        shortWait();

        tRes.tested("down()", listener.downFl) ;
    }

    /**
    * Calls the method. <p>
    * Has <b>OK</b> status if the appropriate listener method
    * was called.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addSpinListener </code> </li>
    * </ul>
    */
    public void _first() {
        requiredMethod("addSpinListener()") ;

        oObj.first() ;
        shortWait();

        tRes.tested("first()", listener.firstFl) ;
    }

    /**
    * Calls the method. <p>
    * Has <b>OK</b> status if the appropriate listener method
    * was called.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addSpinListener </code> </li>
    * </ul>
    */
    public void _last() {
        requiredMethod("addSpinListener()") ;

        oObj.last() ;
        shortWait();

        tRes.tested("last()", listener.lastFl) ;
    }

    /**
    * Removes the listener, then calls <code>up</code> method and
    * checks if te listener wasn't called. <p>
    * Has <b>OK</b> status if listener wasn't called. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addSpinListener </code> </li>
    *  <li> <code> up </code> </li>
    *  <li> <code> down </code> </li>
    *  <li> <code> first </code> </li>
    *  <li> <code> last </code> </li>
    * </ul>
    */
    public void _removeSpinListener() {
        requiredMethod("addSpinListener()") ;
        executeMethod("up()") ;
        executeMethod("down()") ;
        executeMethod("first()") ;
        executeMethod("last()") ;

        listener.upFl = false ;

        oObj.removeSpinListener(listener) ;

        oObj.up() ;

        tRes.tested("removeSpinListener()", !listener.upFl) ;
    }

    /**
    * Enables then disables repeating. <p>
    * Has <b>OK</b> status if no runtime exceptions occured.
    */
    public void _enableRepeat() {
        oObj.enableRepeat(true) ;
        oObj.enableRepeat(false) ;

        tRes.tested("enableRepeat()", true) ;
    }

    /**
     * Waits for 0.5 sec to allow listener to be called.
     */
    private void shortWait() {
        try {
            Thread.sleep(500);
        }
        catch (InterruptedException ex) {
        }

    }
}
