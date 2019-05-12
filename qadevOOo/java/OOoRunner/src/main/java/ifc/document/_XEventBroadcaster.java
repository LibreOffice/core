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
