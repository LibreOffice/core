/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XConfirmDeleteBroadcaster.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:45:31 $
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
