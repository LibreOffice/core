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
import com.sun.star.datatransfer.clipboard.XClipboard;
import com.sun.star.datatransfer.clipboard.XClipboardOwner;

/**
* Testing <code>com.sun.star.datatransfer.clipboard.XClipboard</code>
* interface methods :
* <ul>
*  <li><code> getContents()</code></li>
*  <li><code> setContents()</code></li>
*  <li><code> getName()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.datatransfer.clipboard.XClipboard
*/
public class _XClipboard extends MultiMethodTest {

    public XClipboard oObj;

    MyTransferable myTransferable1;
    MyTransferable myTransferable2;

    MyOwner myOwner;

    /**
     * <code>XClipboardOwner</code> interface implementation which
     * stores parameters passed to <code>lostOwnership</code> method.
     */
    static class MyOwner implements XClipboardOwner {
        XClipboard board;
        XTransferable contents;

        public void lostOwnership(XClipboard board, XTransferable contents) {
            this.board = board;
            this.contents = contents;
        }

    }

    /**
     * Simpliest <code>XTransferable</code> interface implementation.
     */
    static class MyTransferable implements XTransferable {
        public Object getTransferData(DataFlavor dataFlavor) {
            return "";
        }

        public com.sun.star.datatransfer.DataFlavor[] getTransferDataFlavors() {
            return new DataFlavor[0];
        }

        public boolean isDataFlavorSupported(DataFlavor dataFlavor) {
            return false;
        }

    }

    /**
     * Initially sets the content of the clipboard.
     */
    @Override
    public void before() {
        oObj.setContents(myTransferable1 = new MyTransferable(),
                myOwner = new MyOwner());
    }

    /**
     * Gets the name of the clipboard. <p>
     * Has <b>OK</b> status if not <code>null</code> value returned.
     */
    public void _getName() {
        String name = oObj.getName();
        tRes.tested("getName()", name != null);
    }

    /**
     * Gets the contents of the clipboard. <p>
     * Has <b>OK</b> status if the content obtained is equal to content
     * set in <code>before</code> method.
     */
    public void _getContents() {
        tRes.tested("getContents()", oObj.getContents() == myTransferable1);
    }

    /**
    * Sets new contents for the clipboard. Then checks if it was set,
    * and if <code>lostOwnership()</code> notification of the prevoius
    * contents was called with appropriate parameters.<p>
    * Has <b> OK </b> status if <code>getContents</code> returns the same
    * object which is set, and notification was received.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getContents() </code> : for right testing order </li>
    * </ul>
    */
    public void _setContents() {
        requiredMethod("getContents()");
        myTransferable2 = new MyTransferable();

        oObj.setContents(myTransferable2, new MyOwner());

        log.println("sleeping for 1 second");

        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            log.println("interrupted");
            e.printStackTrace(log);
            throw new StatusException("Operation interrupted", e);
        }

        tRes.tested("setContents()", oObj.getContents() == myTransferable2);
    }
}
