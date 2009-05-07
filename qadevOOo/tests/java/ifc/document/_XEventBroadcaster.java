/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XEventBroadcaster.java,v $
 * $Revision: 1.5 $
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

package ifc.document;

import lib.MultiMethodTest;

import com.sun.star.document.XEventBroadcaster;
import com.sun.star.document.XEventListener;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.uno.UnoRuntime;

public class _XEventBroadcaster extends MultiMethodTest {

    public XEventBroadcaster oObj;
    protected static boolean listenerCalled=false;
    private static XEventListener listener=null;

    public class MyEventListener implements XEventListener {

        public void disposing(com.sun.star.lang.EventObject eventObject) {
        }

        public void notifyEvent(com.sun.star.document.EventObject eventObject) {
            System.out.println("EventObject "+eventObject.EventName);
            listenerCalled = true;
        }

    }

    private void switchFocus() {
        XModel docModel = (XModel) UnoRuntime.queryInterface(
                XModel.class,tEnv.getTestObject());
        docModel.getCurrentController().getFrame().getContainerWindow().setFocus();
        util.utils.shortWait(1000);
        XController xc = (XController) UnoRuntime.queryInterface(XController.class,tEnv.getObjRelation("CONT2"));
        xc.getFrame().getContainerWindow().setFocus();
    }

    public void _addEventListener() {
        listener = new MyEventListener();
        listenerCalled = false;
        oObj.addEventListener(listener);
        switchFocus();
        util.utils.shortWait(1000);
        tRes.tested("addEventListener()",listenerCalled);
    }

    public void _removeEventListener() {
        requiredMethod("addEventListener()");
        listenerCalled = false;
        oObj.removeEventListener(listener);
        switchFocus();
        util.utils.shortWait(1000);
        tRes.tested("removeEventListener()",!listenerCalled);
    }

}
