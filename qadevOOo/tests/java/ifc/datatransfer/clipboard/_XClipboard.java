/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XClipboard.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:24:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
