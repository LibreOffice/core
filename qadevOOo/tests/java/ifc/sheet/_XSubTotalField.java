/*************************************************************************
 *
 *  $RCSfile: _XSubTotalField.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:06:57 $
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

package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.GeneralFunction;
import com.sun.star.sheet.SubTotalColumn;
import com.sun.star.sheet.XSubTotalField;

/**
* Testing <code>com.sun.star.sheet.XSubTotalField</code>
* interface methods :
* <ul>
*  <li><code> getGroupColumn()</code></li>
*  <li><code> setGroupColumn()</code></li>
*  <li><code> getSubTotalColumns()</code></li>
*  <li><code> setSubTotalColumns()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSubTotalField
*/
public class _XSubTotalField extends MultiMethodTest {

    public XSubTotalField oObj = null;
    public int GroupColumn = 0;
    public SubTotalColumn[] STColumns = null;

    /**
    * Test calls the method, stores and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't equal to zero. <p>
    */
    public void _getGroupColumn() {
        GroupColumn = oObj.getGroupColumn();
        tRes.tested("getGroupColumn()", GroupColumn != 0);
    } // end of getGroupColumn

    /**
    * Test calls the method, stores and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getSubTotalColumns() {
        STColumns = oObj.getSubTotalColumns();
        tRes.tested("getSubTotalColumns()", STColumns != null);
    } // end of getSubTotalColumns

    /**
    * Test sets new value of the column by which entries are grouped,
    * gets the current value and compares
    * returned value with value that was stored by method
    * <code>getGroupColumn()</code>. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getGroupColumn() </code> : to have the current column by
    *  which entries are grouped </li>
    * </ul>
    */
    public void _setGroupColumn() {
        requiredMethod("getGroupColumn()");
        oObj.setGroupColumn(2);
        tRes.tested("setGroupColumn()", GroupColumn != oObj.getGroupColumn());
    } // end of getGroupColumn

    /**
    * Test sets new value using method, gets the current value and compares
    * returned value with value that was stored by method
    * <code>getSubTotalColumns()</code>. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getSubTotalColumns() </code> : to have the current value</li>
    * </ul>
    */
    public void _setSubTotalColumns() {
        requiredMethod("getSubTotalColumns()");
        SubTotalColumn[] columns = new SubTotalColumn[2];
        SubTotalColumn column = new SubTotalColumn();
        column.Column=2;
        column.Function=GeneralFunction.AVERAGE;
        columns[0]=column;
        columns[1]=STColumns[0];
        oObj.setSubTotalColumns(columns);
        tRes.tested(
            "setSubTotalColumns()",
            !STColumns.equals(oObj.getSubTotalColumns()) );
    } // end of getSubTotalColumns

}  // finish class _XSubTotalField


