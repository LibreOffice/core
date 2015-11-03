/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.util.XConfirmDeleteBroadcaster
*/
public class _XConfirmDeleteBroadcaster extends MultiMethodTest {
    public XConfirmDeleteBroadcaster oObj = null;
    protected XConfirmDeleteListener mxConfirmDeleteListener =
                                                new ConfirmDeleteImpl();

    private static class ConfirmDeleteImpl implements XConfirmDeleteListener {
        public boolean confirmDelete(RowChangeEvent rowChangeEvent) {
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
