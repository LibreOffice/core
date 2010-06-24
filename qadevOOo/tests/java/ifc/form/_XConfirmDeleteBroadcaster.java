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

package ifc.form;

import com.sun.star.form.XConfirmDeleteBroadcaster;
import com.sun.star.form.XConfirmDeleteListener;
import com.sun.star.lang.EventObject;
import com.sun.star.sdb.RowChangeEvent;
import lib.MultiMethodTest;

/**
* Testing <code>com.sun.star.util.XConfirmDeleteBroadcaster</code>
* interface methods :
* <ul>
*  <li><code>addConfirmDeleteListener()</code></li>
*  <li><code>removeConfirmDeleteListener()</code></li>
* </ul> <p>
*
* Result checking is not performed. Confirm delete
* listeners are called only in case of
* interaction with UI. <p>
*
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.util.XConfirmDeleteBroadcaster
*/
public class _XConfirmDeleteBroadcaster extends MultiMethodTest {
    public XConfirmDeleteBroadcaster oObj = null;
    protected boolean confirmed = false;
    protected XConfirmDeleteListener mxConfirmDeleteListener =
                                                new ConfirmDeleteImpl();

    private class ConfirmDeleteImpl implements XConfirmDeleteListener {
        public boolean confirmDelete(RowChangeEvent rowChangeEvent) {
            confirmed = true;
            return true;
        }
        public void disposing(EventObject eventObject) {
        }
    }

    protected void addConfirmDeleteListener() {
        oObj.addConfirmDeleteListener(mxConfirmDeleteListener);
        tRes.tested("addConfirmDeleteListener()", true);
    }

    protected void removeConfirmDeleteListener() {
        oObj.removeConfirmDeleteListener(mxConfirmDeleteListener);
        tRes.tested("removeConfirmDeleteListener()", true);
    }
}
