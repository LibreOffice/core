/*************************************************************************
 *
 *  $RCSfile: _XClipboardNotifier.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:25:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
