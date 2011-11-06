/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
