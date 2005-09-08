/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XApproveActionBroadcaster.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:44:29 $
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
import lib.Status;
import lib.StatusException;

import com.sun.star.form.XApproveActionBroadcaster;
import com.sun.star.form.XApproveActionListener;
import com.sun.star.lang.EventObject;

/**
* Testing <code>com.sun.star.form.XApproveActionBroadcaster</code>
* interface methods:
* <ul>
*  <li><code> addApproveActionListener() </code></li>
*  <li><code> removeApproveActionListener() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.form.XApproveActionBroadcaster
*/
public class _XApproveActionBroadcaster extends MultiMethodTest {
    public XApproveActionBroadcaster oObj = null;

    /**
    * Class we need to test methods.
    */
    protected class TestListener implements XApproveActionListener {
        public boolean approve = false ;

        public void init() {
            approve = false ;
        }
        public void disposing(EventObject ev) {}
        public boolean approveAction(EventObject ev) {
            log.println("XApproveActionListener: ActionListener was called");
            return approve ;
        }

    }

    private TestListener listener = new TestListener();

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown.
    */
    public void _addApproveActionListener() {
        oObj.addApproveActionListener(listener) ;
        tRes.tested("addApproveActionListener()", true);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addApproveActionListener() </code> : adds listener to an
    * object </li>
    * </ul>
    */
    public void _removeApproveActionListener() {
        requiredMethod("addApproveActionListener()");
        listener.init() ;
        listener.approve = true ;
        oObj.removeApproveActionListener(listener);
        tRes.tested("removeApproveActionListener()", true);
    }


    /**
    * Just log output
    */
    protected void after() {
        log.println("Skipping all XApproveActionBroadcaster methods, since they"
                + " need user interaction");
        throw new StatusException(Status.skipped(true));
    }

}


