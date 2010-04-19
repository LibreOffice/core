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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.datatransfer.clipboard.XClipboardNotifier
* @see com.sun.star.datatransfer.clipboard.XClipboard
*/
public class _XClipboardNotifier extends MultiMethodTest {

    public XClipboardNotifier oObj;

    /**
     * <code>XClipboardOwner</code> interface implementation which
     * stores parameters passed to <code>lostOwnership</code> method.
     */
    class MyOwner implements XClipboardOwner {
        XClipboard board;
        XTransferable contents;

        public void lostOwnership(XClipboard board, XTransferable contents) {
            this.board = board;
            this.contents = contents;
        }
    }

    /**
     * Simpliest <code>XTransferable</code> interface implementation
     * which supports "text/htmp" data type.
     */
    class MyTransferable implements XTransferable {
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

        XClipboard board = (XClipboard)UnoRuntime.queryInterface(
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

        XClipboard board = (XClipboard)UnoRuntime.queryInterface(
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
