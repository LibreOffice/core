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

    @Override
    public void before() {
        xSpreadsheetView = UnoRuntime.queryInterface(
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