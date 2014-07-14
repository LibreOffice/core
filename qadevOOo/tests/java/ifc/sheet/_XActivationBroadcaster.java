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


package ifc.sheet;

import com.sun.star.sheet.XActivationBroadcaster;
import com.sun.star.sheet.XActivationEventListener;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetView;
import com.sun.star.uno.UnoRuntime;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;


public class _XActivationBroadcaster extends MultiMethodTest {
    public XActivationBroadcaster oObj;
    protected boolean listenerCalled = false;
    protected XSpreadsheetView xSpreadsheetView = null;
    protected XActivationEventListener listener = null;
    protected XSpreadsheet two = null;

    public void _addActivationEventListener() {

        log.println("trying to add an invalid listener");
        oObj.addActivationEventListener(listener);
        log.println(".... OK");

        log.println("adding a valid listener");
        listener = new MyListener();
        oObj.addActivationEventListener(listener);

        XSpreadsheet org = xSpreadsheetView.getActiveSheet();
        xSpreadsheetView.setActiveSheet(two);

        if (!listenerCalled) {
            log.println("Listener wasn't called");
        }

        xSpreadsheetView.setActiveSheet(org);
        tRes.tested("addActivationEventListener()", listenerCalled);
    }

    public void _removeActivationEventListener() {
        requiredMethod("addActivationEventListener()");
        listenerCalled = false;
        oObj.removeActivationEventListener(listener);

        XSpreadsheet org = xSpreadsheetView.getActiveSheet();
        xSpreadsheetView.setActiveSheet(two);

        if (listenerCalled) {
            log.println("Listener was called even though it is removed");
        }

        xSpreadsheetView.setActiveSheet(org);
        tRes.tested("removeActivationEventListener()", !listenerCalled);
    }

    public void before() {
        xSpreadsheetView = (XSpreadsheetView) UnoRuntime.queryInterface(
                                   XSpreadsheetView.class,
                                   tEnv.getTestObject());

        two = (XSpreadsheet) tEnv.getObjRelation("Sheet");

        if ((xSpreadsheetView == null) || (two == null)) {
            throw new StatusException(Status.failed(
                                              "precondition for test is missing"));
        }
    }

    protected class MyListener implements XActivationEventListener {
        public void activeSpreadsheetChanged(com.sun.star.sheet.ActivationEvent activationEvent) {
            listenerCalled = true;
        }

        public void disposing(com.sun.star.lang.EventObject eventObject) {
        }
    }
}