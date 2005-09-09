/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XTextTablesSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:28:44 $
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

import com.sun.star.container.XNameAccess;
import com.sun.star.text.XTextTablesSupplier;


/**
 * Testing <code>com.sun.star.text.XTextTablesSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getTextTables()</code></li>
 * </ul> <p>
 * The table collection tested <b>must</b> have a table
 * with name 'SwXTextDocument'. <p>
 * Test is multithread compilant. <p>
 * @see com.sun.star.text.XTextTablesSupplier
 */
public class _XTextTablesSupplier extends MultiMethodTest {

    public static XTextTablesSupplier oObj = null;

    /**
     * Get the collection of tables. Then check it. <p>
     * Has <b>OK</b> status if returned collection contains
     * element with name 'SwXTextDocument'.
     */
    public void _getTextTables() {
        boolean res = false;

        XNameAccess the_tables = oObj.getTextTables();
        res = the_tables.hasByName("SwXTextDocument");

        tRes.tested("getTextTables()",res);
    }

}  // finish class _XTextTablesSupplier

