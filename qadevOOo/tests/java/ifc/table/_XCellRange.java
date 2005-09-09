/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XCellRange.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:06:32 $
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

import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;

/**
* Testing <code>com.sun.star.table.XCellRange</code>
* interface methods :
* <ul>
*  <li><code> getCellByPosition()</code></li>
*  <li><code> getCellRangeByPosition()</code></li>
*  <li><code> getCellRangeByName()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'ValidRange'</code> (of type <code>String</code>):
*   cell range that can be defined by the object test instead of
*   definition at this test ("<code>A1:A1</code>")</li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.table.XCellRange
*/
public class _XCellRange extends MultiMethodTest {
    public XCellRange oObj = null;

    /**
     * First a cell get from valid position, second - from invalid. <p>
     * Has <b> OK </b> status if in the first case not null value is
     * returned and no exceptions are thrown, and in the second
     * case <code>IndexOutOfBoundsException</code> is thrown. <p>
     */
    public void _getCellByPosition() {

        boolean result = false;

        try {
            XCell cell = oObj.getCellByPosition(0,0);
            result = cell != null ;
            log.println("Getting cell by position with a valid position ... OK");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception occurred while getting cell by position with a valid position");
            e.printStackTrace(log);
            result = false;
        }

        try {
            oObj.getCellByPosition(-1,1);
            log.println("No Exception occurred while getting cell by position with invalid position");
            result &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Getting cell by position with a invalid position ... OK");
            result &= true;
        }

        tRes.tested( "getCellByPosition()", result );

    } // end getCellByPosition()

    /**
     * A range is tried to obtain with valid name. <p>
     * Has <b> OK </b> status if not null range is
     * returned. <p>
     */
    public void _getCellRangeByName() {

        boolean result = false;

        String valid = (String) tEnv.getObjRelation("ValidRange");
        if (valid == null ) valid = "A1:A1";
        XCellRange range = oObj.getCellRangeByName(valid);
        result = range != null ;
        log.println("Getting cellrange by name with a valid name ... OK");

        tRes.tested( "getCellRangeByName()", result );


    } // end getCellRangeByName()

    /**
     * First a range is tried to obtain with valid bounds,
     * second - with invalid. <p>
     * Has <b> OK </b> status if in the first case not null range is
     * returned and no exceptions are thrown, and in the second
     * case <code>IndexOutOfBoundsException</code> is thrown. <p>
     */
    public void _getCellRangeByPosition() {

        boolean result = false;

        try {
            XCellRange range = oObj.getCellRangeByPosition(0,0,0,0);
            result = range != null;
            log.println("Getting cellrange by Position with a valid position ... OK");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception occurred while getting cellrange by position with a valid position");
            e.printStackTrace(log);
            result = false;
        }

        try {
            oObj.getCellRangeByPosition(-1,0,-1,1);
            log.println("No Exception occurred while getting cellrange by position with invalid position");
            result &= false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Getting cellrange by position with a invalid position ... OK");
            result &= true;
        }

        tRes.tested( "getCellRangeByPosition()", result );


    } // end getCellRangeByPosition()

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }

} // finish class _XCellRange

