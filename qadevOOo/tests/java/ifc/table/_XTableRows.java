/*************************************************************************
 *
 *  $RCSfile: _XTableRows.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-12-11 11:46:28 $
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

package ifc.table;

import com.sun.star.table.XCellRange;
import lib.MultiMethodTest;

import com.sun.star.table.XTableRows;
import lib.Status;
import lib.StatusException;

/**
* Testing <code>com.sun.star.table.XTableRows</code>
* interface methods :
* <ul>
*  <li><code> insertByIndex()</code></li>
*  <li><code> removeByIndex()</code></li>
* </ul>
*/
public class _XTableRows extends MultiMethodTest {

    public XTableRows oObj = null;
    public XCellRange range = null;

    public void before() {
        range = (XCellRange) tEnv.getObjRelation("XTableRows.XCellRange");
        if (range==null) {
            throw new StatusException(Status.failed("ObjectRelation missing"));
        }
        try {
            range.getCellByPosition(0,0).setValue(17);
            range.getCellByPosition(0,1).setValue(15);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't set value for Cell A1");
        }
    }

    /**
     * First a row inserted to valid position, then to invalid. <p>
     * Has <b> OK </b> status if in the first case number of rows increases
     * by 1, and in the second an exception is thrown. <p>
     */
    public void _insertByIndex() {

        boolean result = true;

        requiredMethod("removeByIndex()");

        int origCnt = oObj.getCount();
        log.println("Inserting row before first row");
        oObj.insertByIndex(0,1);
        result &= checkCell(1,15);
        if (checkCell(1,15)) log.println("... successful");

        try {
            oObj.insertByIndex(-1,1);
            log.println("No Exception occurred while inserting row at -1");
            result &= false;
        } catch (Exception e) {
            log.println("Inserting row at Index -1 ... OK");
            result &= true;
        }

        tRes.tested( "insertByIndex()", result );

    } // end insertByIndex()

    /**
     * First a row removed from valid position, then from invalid. <p>
     *
     * Has <b> OK </b> status if in the first case number of columns decreases
     * by 1, and in the second an exception is thrown. <p>
     */
    public void _removeByIndex() {

        boolean result = true;

        oObj.removeByIndex(0,1);
        log.println("Removing first row");
        result &= checkCell(0,15);
        if (checkCell(0,15)) log.println("... successful");

        try {
            oObj.removeByIndex(-1,1);
            log.println("No Exception occurred while Removing row at -1");
            result &= false;
        } catch (Exception e) {
            log.println("Removing row at Index -1 ... OK");
            result &= true;
        }

        tRes.tested( "removeByIndex()", result );
    } // end removeByIndex()

    public boolean checkCell(int row,double expected) {
        double getting=0;
        try {
            getting = range.getCellByPosition(0,row).getValue();
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Couldn't set value for Cell A1");
        }

        boolean res = (getting==expected);
        if (!res) {
            log.println("Expected for row "+row+" was "+expected);
            log.println("Getting for row "+row+" - "+getting);
            log.println("=> FAILED");
        }
        return res;
    }

} //finish class _XTableRows

