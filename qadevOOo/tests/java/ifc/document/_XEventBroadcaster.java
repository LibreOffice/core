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

package ifc.document;

import lib.MultiMethodTest;

import com.sun.star.document.XEventBroadcaster;
import com.sun.star.document.XEventListener;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.uno.UnoRuntime;

public class _XEventBroadcaster extends MultiMethodTest {

    public XEventBroadcaster oObj;
    protected boolean listenerCalled = false;
    private XEventListener listener = null;

    public class MyEventListener implements XEventListener {

        public void disposing(com.sun.star.lang.EventObject eventObject) {
        }

        public void notifyEvent(com.sun.star.document.EventObject eventObject) {
            System.out.println("EventObject "+eventObject.EventName);
            listenerCalled = true;
        }

    }

    private void switchFocus() {
        XModel docModel = UnoRuntime.queryInterface(
                XModel.class,tEnv.getTestObject());
        docModel.getCurrentController().getFrame().getContainerWindow().setFocus();
        util.utils.pause(1000);
        XController xc = UnoRuntime.queryInterface(XController.class,tEnv.getObjRelation("CONT2"));
        xc.getFrame().getContainerWindow().setFocus();
    }

    public void _addEventListener() {
        listener = new MyEventListener();
        listenerCalled = false;
        oObj.addEventListener(listener);
        switchFocus();
        util.utils.pause(1000);
        tRes.tested("addEventListener()",listenerCalled);
    }

    public void _removeEventListener() {
        requiredMethod("addEventListener()");
        listenerCalled = false;
        oObj.removeEventListener(listener);
        switchFocus();
        util.utils.pause(1000);
        tRes.tested("removeEventListener()",!listenerCalled);
    }

}
