/*************************************************************************
 *
 *  $RCSfile: _XRefreshable.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:31:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.util.XRefreshListener;
import com.sun.star.util.XRefreshable;

/**
* Testing <code>com.sun.star.util.XRefreshable</code>
* interface methods :
* <ul>
*  <li><code> refresh()</code></li>
*  <li><code> addRefreshListener()</code></li>
*  <li><code> removeRefreshListener()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.util.XRefreshable
*/
public class _XRefreshable extends MultiMethodTest {

    public XRefreshable oObj = null;

    final boolean listenerCalled[] = new boolean[1];


    /**
    * <code>XRefreshListener</code> implementation which
    * sets a flag when <code>refreshed</code> method is
    * called.
    */
    public class MyRefreshListener implements XRefreshListener {
        public void refreshed (com.sun.star.lang.EventObject e) {
            listenerCalled[0] = true;
        }

        public void disposing (com.sun.star.lang.EventObject obj) {}
    }

    XRefreshListener listener = new MyRefreshListener();

    /**
    * Just adds a listener. <p>
    * Always has <b>OK</b> status.
    */
    public void _addRefreshListener() {

         oObj.addRefreshListener(listener) ;
         tRes.tested("addRefreshListener()", true);
    }

    /**
    * Calls the method and checks if the listener was called. <p>
    * Has <b>OK</b> status if listener's flag is set after call.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addRefreshListener </code> : to have a listener added.</li>
    * </ul>
    */
    public void _refresh() {

        requiredMethod("addRefreshListener()");

        oObj.refresh();

        tRes.tested("refresh()", listenerCalled[0]);
        if (!listenerCalled[0])
            log.println("RefreshListener wasn't called after refresh");

    }

    /**
    * Removes the listener added before and calls <code>refresh</code>
    * method. Listener must not be called. <p>
    * Has <b>OK</b> status if listener's flag isn't changed.
    * <ul>
    *  <li> <code> refresh </code> : listener added must be already
    *    tested.</li>
    * </ul>
    */
    public void _removeRefreshListener() {
        requiredMethod("refresh()");
        listenerCalled[0] = false;

        oObj.removeRefreshListener(listener) ;
        oObj.refresh();

        tRes.tested("removeRefreshListener()", !listenerCalled[0]);
        if (listenerCalled[0])
            log.println("RefreshListener was called after removing");
    }
} // finish class _XRefreshable

