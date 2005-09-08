/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XSheetCellRanges.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:54:38 $
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

package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.container.XEnumeration;
import com.sun.star.sheet.XSheetCellRanges;
import com.sun.star.table.CellRangeAddress;

/**
* Testing <code>com.sun.star.sheet.XSheetCellRanges</code>
* interface methods :
* <ul>
*  <li><code> getCells()</code></li>
*  <li><code> getRangeAddressesAsString()</code></li>
*  <li><code> getRangeAddresses()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetCellRanges
*/
public class _XSheetCellRanges extends MultiMethodTest{

    public XSheetCellRanges oObj = null;

    /**
    * Test calls the method, creates enumeration of returned value
    * and checks that the enumeration has elements. <p>
    * Has <b> OK </b> status if gained enumeration has elements. <p>
    */
    public void _getCells() {
        log.println("Testing getCells ...");

        XEnumeration oEnum = oObj.getCells().createEnumeration();
        boolean res = oEnum.hasMoreElements();
        if (!res) {
            log.println(
                    "The Enumeration gained via getCells() has no Elements");
        }
        tRes.tested("getCells()", res);
    }

    /**
    * Test calls the method and checks length of returned array. <p>
    * Has <b> OK </b> status if length of returned array is greater than 2.<p>
    */
    public void _getRangeAddresses() {
        log.println("Testing getRangeAddresses ...");
        CellRangeAddress[] oRanges = oObj.getRangeAddresses();
        int howmuch = oRanges.length;
        tRes.tested("getRangeAddresses()", (howmuch > 2) );
    }

    /**
    * Test calls the method and checks returned string. <p>
    * Has <b> OK </b> status if returned string starts from 'Sheet'.<p>
    */
    public void _getRangeAddressesAsString() {
        log.println("Testing getRangeAddressesAsString ...");
        String oRanges = oObj.getRangeAddressesAsString();
        tRes.tested("getRangeAddressesAsString()",oRanges.indexOf("C1:D4")>0);
    }

} // finished class _XSheetCellRanges

