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

package ifc.datatransfer.clipboard;

import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.datatransfer.DataFlavor;
import com.sun.star.datatransfer.XTransferable;
import com.sun.star.datatransfer.clipboard.ClipboardEvent;
import com.sun.star.datatransfer.clipboard.XClipboard;
import com.sun.star.datatransfer.clipboard.XClipboardListener;
import com.sun.star.datatransfer.clipboard.XClipboardNotifier;
import com.sun.star.datatransfer.clipboard.XClipboardOwner;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.datatransfer.clipboard.XClipboardNotifier</code>
* interface methods :
* <ul>
*  <li><code> addClipboardListener()</code></li>
*  <li><code> removeClipboardListener()</code></li>
* </ul> <p>
* The object <b>must also implement</b> <code>XClipboard</code>
* interface. <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.datatransfer.clipboard.XClipboardNotifier
* @see com.sun.star.datatransfer.clipboard.XClipboard
*/
public class _XClipboardNotifier extends MultiMethodTest {

    public XClipboardNotifier oObj;

    /**
     * <code>XClipboardOwner</code> interface implementation which
     * stores parameters passed to <code>lostOwnership</code> method.
     */
    static class MyOwner implements XClipboardOwner {

        public void lostOwnership(XClipboard board, XTransferable contents) {
        }
    }

    /**
     * Simpliest <code>XTransferable</code> interface implementation
     * which supports "text/html" data type.
     */
    static class MyTransferable implements XTransferable {
        DataFlavor[] supportedFlavors;

        public MyTransferable() {
            supportedFlavors = new DataFlavor[] {
                new DataFlavor("text/plain", "Plain text", new Type(String.class))
            };
        }

        public Object getTransferData(DataFlavor dataFlavor) {
            return "";
        }

        public DataFlavor[] getTransferDataFlavors() {
            return supportedFlavors;
        }

        public boolean isDataFlavorSupported(DataFlavor dataFlavor) {
            return supportedFlavors[0].MimeType.equals(dataFlavor.MimeType);
        }
    }

    /**
     * Implementation of listener which registers its method calls.
     */
    class MyClipboardListener implements XClipboardListener {
        boolean called = false;

        public void changedContents(ClipboardEvent evt) {
            called = true;
        }

        public void disposing(EventObject wvt) {
            log.println("");
        }
    }

    MyClipboardListener myListener;

    /**
    * Adds a listener and put a new contents into clipboard. <p>
    * Has <b> OK </b> status if the listener was called on contents changing.
    */
    public void _addClipboardListener() {
        oObj.addClipboardListener(myListener = new MyClipboardListener());

        XClipboard board = UnoRuntime.queryInterface(
                XClipboard.class, tEnv.getTestObject());

        board.setContents(new MyTransferable(), new MyOwner());

        log.println("sleeping for 1 second");

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            log.println("interrupted");
            e.printStackTrace(log);
            throw new StatusException("Operation interrupted", e);
        }

        tRes.tested("addClipboardListener()", myListener.called);
    }

    /**
    * Removes the listener and put a new contents into clipboard. <p>
    * Has <b> OK </b> status if the listener was not called on contents
    * changing.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code>addClipboardListener()</code> </li>
    * </ul>
    */
    public void _removeClipboardListener() {
        try {
            requiredMethod("addClipboardListener()");
            myListener.called = false;
        } finally {
            oObj.removeClipboardListener(myListener);
        }

        XClipboard board = UnoRuntime.queryInterface(
                XClipboard.class, oObj);

        board.setContents(new MyTransferable(), new MyOwner());

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            log.println("interrupted");
            e.printStackTrace(log);
            throw new StatusException("Operation interrupted", e);
        }

        tRes.tested("removeClipboardListener()", !myListener.called);
    }
}
