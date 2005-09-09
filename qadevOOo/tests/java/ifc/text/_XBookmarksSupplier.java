/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XBookmarksSupplier.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:19:20 $
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

package ifc.text;

import com.sun.star.container.XNameAccess;
import com.sun.star.text.XBookmarksSupplier;
import lib.MultiMethodTest;


/**
 * Testing <code>com.sun.star.text.XBookmarksSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getBookmarks()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XBookmarksSupplier
 */
public class _XBookmarksSupplier extends MultiMethodTest {

    public static XBookmarksSupplier oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the collection returned has at least
     * one element.
     */
    public void _getBookmarks() {
        boolean res = false;

        XNameAccess the_bookmarks = oObj.getBookmarks();
        log.println("Found: ");
        util.dbg.printArray(the_bookmarks.getElementNames());
        res = the_bookmarks.getElementNames().length>0;

        tRes.tested("getBookmarks()",res);
    }

}  // finish class _XBookmarksSupplier
