/*************************************************************************
 *
 *  $RCSfile: _XConfirmDeleteBroadcaster.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-11-18 16:22:17 $
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
