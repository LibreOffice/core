/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XSpreadsheetView.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:58:06 $
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
import lib.Status;
import lib.StatusException;

import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetView;

/**
* Testing <code>com.sun.star.sheet.XSpreadsheetView</code>
* interface methods :
* <ul>
*  <li><code> getActiveSheet()</code></li>
*  <li><code> setActiveSheet()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'Sheet'</code> (of type <code>XSpreadsheet</code>):
*   to set new active spreadsheet </li>
* <ul> <p>
* @see com.sun.star.sheet.XSpreadsheetView
*/
public class _XSpreadsheetView extends MultiMethodTest {

    public XSpreadsheetView oObj = null;
    public XSpreadsheet oSheet = null;

    /**
    * Test calls the method, stores and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getActiveSheet() {
        oSheet = oObj.getActiveSheet();
        tRes.tested("getActiveSheet()", oSheet != null);
    }

    /**
    * Test sets new active sheet that was obtained by relation
    * <code>'Sheet'</code>, gets the current active sheet and compares
    * returned value with value that was stored by method
    * <code>getFilterFields()</code>. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getActiveSheet() </code> : to have the current
    *  active sheet </li>
    * </ul>
    */
    public void _setActiveSheet() {
        requiredMethod("getActiveSheet()");

        XSpreadsheet new_Sheet = (XSpreadsheet)tEnv.getObjRelation("Sheet");
        if (new_Sheet == null) throw new StatusException(Status.failed
            ("Relation 'Sheet' not found"));

        oObj.setActiveSheet(new_Sheet);
        new_Sheet = oObj.getActiveSheet();
        tRes.tested("setActiveSheet()", !oSheet.equals(new_Sheet));
    }


}  // finish class _XSpreadsheetView


