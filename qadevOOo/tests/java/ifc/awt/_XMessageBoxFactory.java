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
            new Rectangle(0, 0, 100, 100), "errorbox", 1, "The Title",
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
