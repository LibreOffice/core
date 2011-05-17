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
            log.println("Listener was called eventhough it is removed");
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