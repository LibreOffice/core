/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XActivationBroadcaster.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:41:19 $
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