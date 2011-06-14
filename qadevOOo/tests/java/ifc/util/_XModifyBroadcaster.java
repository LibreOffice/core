/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.lang.EventObject;
import com.sun.star.util.XModifyBroadcaster;
import com.sun.star.util.XModifyListener;

/**
* Testing <code>com.sun.star.util.XModifyBroadcaster</code>
* interface methods :
* <ul>
*  <li><code>addModifyListener()</code></li>
*  <li><code>removeModifyListener()</code></li>
* </ul> <p>
*
* Result checking is not performed. Modify
* listeners are called only in case of
* interaction with UI. <p>
*
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.util.XModifyBroadcaster
*/
public class _XModifyBroadcaster extends MultiMethodTest {
    public XModifyBroadcaster oObj = null;

    boolean modified = false;

    protected class TestModifyListener implements XModifyListener {
        public void disposing ( EventObject oEvent ) {}
        public void modified (EventObject aEvent ) {
            modified = true;
        }
    }

    private TestModifyListener listener = new TestModifyListener();

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _addModifyListener() {
        log.println("'Modified' events are called only in case"+
            " of user interaction.");
        oObj.addModifyListener(listener);
        tRes.tested("addModifyListener()", true);
    }

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _removeModifyListener() {
        requiredMethod("addModifyListener()");
        oObj.removeModifyListener(listener);
        tRes.tested("removeModifyListener()", true);
    }
}// finish class _XModifyBroadcaster

