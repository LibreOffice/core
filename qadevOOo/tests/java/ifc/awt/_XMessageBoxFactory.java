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

package ifc.awt;

import com.sun.star.awt.AsyncCallback;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XCallback;
import com.sun.star.awt.XMessageBox;
import com.sun.star.awt.XMessageBoxFactory;
import com.sun.star.awt.XRequestCallback;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Any;
import com.sun.star.uno.UnoRuntime;
import lib.MultiMethodTest;
import util.UITools;

/**
 * Testing <code>com.sun.star.awt.XMessageBoxFactory</code>
 * interface methods :
 * <ul>
 *  <li><code> </code>createMessageBox()</li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.awt.XMessageBoxFactory
 */
public class _XMessageBoxFactory extends MultiMethodTest {
    public XMessageBoxFactory oObj = null;

    public void _createMessageBox() {
        final XMessageBox mb = oObj.createMessageBox(
            (XWindowPeer) tEnv.getObjRelation("WINPEER"),
            com.sun.star.awt.MessageBoxType.ERRORBOX, 1, "The Title",
            "The Message");
        final UITools tools = new UITools(
            (XMultiServiceFactory) tParam.getMSF(),
            UnoRuntime.queryInterface(XWindow.class, mb));
        final boolean[] done = new boolean[] { false };
        final boolean[] good = new boolean[] { false };
        XRequestCallback async = AsyncCallback.create(
            tParam.getComponentContext());
        async.addCallback(
            new XCallback() {
                public void notify(Object aData) {
                    mb.execute();
                    synchronized (done) {
                        done[0] = true;
                        done.notifyAll();
                    }
                }
            },
            Any.VOID);
        async.addCallback(
            new XCallback() {
                public void notify(Object aData) {
                    try {
                        tools.clickButton("OK");
                    } catch (RuntimeException e) {
                        throw e;
                    } catch (Exception e) {
                        throw new RuntimeException(e);
                    }
                    synchronized (good) {
                        good[0] = true;
                    }
                }
            },
            Any.VOID);
        synchronized (done) {
            while (!done[0]) {
                try {
                    done.wait();
                } catch (InterruptedException e) {
                    throw new RuntimeException(e);
                }
            }
        }
        boolean ok;
        synchronized (good) {
            ok = good[0];
        }
        tRes.tested("createMessageBox()", ok);
    }
}
