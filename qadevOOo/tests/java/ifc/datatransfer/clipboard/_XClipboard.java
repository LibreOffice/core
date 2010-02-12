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
* Test is <b> NOT </b> multithread compilant. <p>
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
    class MyOwner implements XClipboardOwner {
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
    class MyTransferable implements XTransferable {
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
    * and if <code>lostOwnerShip()</code> notification of the prevoius
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
