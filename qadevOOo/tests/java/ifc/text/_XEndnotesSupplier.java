/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XEndnotesSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:20:35 $
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

import lib.MultiMethodTest;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.text.XEndnotesSupplier;


/**
 * Testing <code>com.sun.star.text.XEndnotesSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getEndnotes()</code></li>
 *  <li><code> getEndnoteSettings()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XEndnotesSupplier
 */
public class _XEndnotesSupplier extends MultiMethodTest {

    public static XEndnotesSupplier oObj = null;        // oObj filled by MultiMethodTest

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the collection returned has at least
     * one element.
     */
    public void _getEndnotes() {
        boolean res = false;

        XIndexAccess the_endnotes = oObj.getEndnotes();
        res = the_endnotes.hasElements();

        tRes.tested("getEndnotes()",res);
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getEndnoteSettings() {
        boolean res = false;

        XPropertySet the_props = oObj.getEndnoteSettings();
        res = ( the_props != null );

        tRes.tested("getEndnoteSettings()",res);
    }

}  // finish class _XEndnotesSupplier


