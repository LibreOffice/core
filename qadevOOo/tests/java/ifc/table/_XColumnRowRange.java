/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XColumnRowRange.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:07:47 $
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

package ifc.table;

import lib.MultiMethodTest;

import com.sun.star.table.XColumnRowRange;

/**
* Testing <code>com.sun.star.table.XColumnRowRange</code>
* interface methods :
* <ul>
*  <li><code> getColumns()</code></li>
*  <li><code> getRows()</code></li>
* </ul> <p>
* @see com.sun.star.table.XColumnRowRange
*/
public class _XColumnRowRange extends MultiMethodTest {

    public XColumnRowRange oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not null value
     * and no exceptions were thrown. <p>
     */
    public void _getColumns() {
        tRes.tested( "getColumns()", oObj.getColumns()!=null );
    } // getColumns()

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not null value
     * and no exceptions were thrown. <p>
     */
    public void _getRows() {
        tRes.tested( "getRows()", oObj.getRows()!=null );
    } // getRows()

} // finish class _XColumnRowRange

