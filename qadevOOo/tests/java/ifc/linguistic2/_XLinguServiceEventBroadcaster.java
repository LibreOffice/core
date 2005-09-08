/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XLinguServiceEventBroadcaster.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:18:31 $
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

package ifc.linguistic2;

import lib.MultiMethodTest;

import com.sun.star.lang.EventObject;
import com.sun.star.linguistic2.LinguServiceEvent;
import com.sun.star.linguistic2.XLinguServiceEventBroadcaster;
import com.sun.star.linguistic2.XLinguServiceEventListener;

/**
* Testing <code>com.sun.star.linguistic2.XLinguServiceEventBroadcaster</code>
* interface methods:
* <ul>
*   <li><code>addLinguServiceEventListener()</code></li>
*   <li><code>removeLinguServiceEventListener()</code></li>
* </ul><p>
* @see com.sun.star.linguistic2.XLinguServiceEventBroadcaster
*/
public class _XLinguServiceEventBroadcaster extends MultiMethodTest {

    public XLinguServiceEventBroadcaster oObj = null;

    /**
    * Class implements interface <code>XLinguServiceEventListener</code>
    * for test method <code>addLinguServiceEventListener</code>.
    * @see com.sun.star.linguistic2.XLinguServiceEventListener
    */
    public class MyLinguServiceEventListener implements
            XLinguServiceEventListener {
        public void disposing ( EventObject oEvent ) {
            log.println("Listener has been disposed");
        }
        public void processLinguServiceEvent(LinguServiceEvent aServiceEvent) {
            log.println("Listener called");
        }

    };

    XLinguServiceEventListener listener = new MyLinguServiceEventListener();

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is true. <p>
    */
    public void _addLinguServiceEventListener() {
        boolean res = true;

        res = oObj.addLinguServiceEventListener(listener);

        tRes.tested("addLinguServiceEventListener()",res);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is true. <p>
    */
    public void _removeLinguServiceEventListener() {
        boolean res = true;

        res = oObj.removeLinguServiceEventListener(listener);

        tRes.tested("removeLinguServiceEventListener()",res);
    }

}  // finish class XLinguServiceEventBroadcaster


