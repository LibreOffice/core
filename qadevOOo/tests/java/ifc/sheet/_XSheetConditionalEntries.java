/*************************************************************************
 *
 *  $RCSfile: _XSheetConditionalEntries.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:04:34 $
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

import com.sun.star.beans.PropertyValue;
import com.sun.star.sheet.ConditionOperator;
import com.sun.star.sheet.XSheetConditionalEntries;
import com.sun.star.table.CellAddress;

/**
* Testing <code>com.sun.star.sheet.XSheetConditionalEntries</code>
* interface methods :
* <ul>
*  <li><code> addNew()</code></li>
*  <li><code> removeByIndex()</code></li>
*  <li><code> clear()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetConditionalEntries
*/
public class _XSheetConditionalEntries extends MultiMethodTest {
    public XSheetConditionalEntries oObj = null;
    int nNum = 0;

    /**
    * Test adds a conditional entry to the format. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _addNew() {
        nNum = oObj.getCount();
        oObj.addNew( Conditions(4) );
        boolean res = (nNum + 1) == oObj.getCount();

        tRes.tested("addNew()", res);
    }

    /**
    * Test calls the method and checks number of conditional entries in
    * collection. <p>
    * Has <b> OK </b> status if number of conditional entries in co0llection
    * after method call is equal zero. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addNew() </code> : to have one conditional entry in
    *  collection at least </li>
    * </ul>
    */
    public void _clear() {
        requiredMethod("removeByIndex()");
        oObj.clear();
        int anz = oObj.getCount();
        tRes.tested("clear()", anz == 0);
    }

    /**
    * Test adds a conditional entry, removes entry with index 0
    * and compares number of entries after adding to number of entries after
    * removing. <p>
    * Has <b> OK </b> status if number of entries after adding is greater
    * than number of entries after removing. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> clear() </code> : to be sure that collection hasn't
    *  elements </li>
    * </ul>
    */
    public void _removeByIndex() {
        requiredMethod("addNew()");
        oObj.removeByIndex(0);
        int pastNum = oObj.getCount();
        tRes.tested("removeByIndex()", pastNum == nNum);
    }

    /**
    * Method creates array of property value for conditional entry using
    * passed parameter <code>nr</code>.
    * @param nr number of row for conditional entry
    */
    protected PropertyValue[] Conditions(int nr) {
        PropertyValue[] con = new PropertyValue[5];
        CellAddress ca = new CellAddress();
        ca.Column = 1;
        ca.Row = 5;
        ca.Sheet = 0;
        con[0] = new PropertyValue();
        con[0].Name = "StyleName";
        con[0].Value = "Result2";
        con[1] = new PropertyValue();
        con[1].Name = "Formula1";
        con[1].Value = "$Sheet1.$B$"+nr;
        con[2] = new PropertyValue();
        con[2].Name = "Formula2";
        con[2].Value = "";
        con[3] = new PropertyValue();
        con[3].Name = "Operator";
        con[3].Value = ConditionOperator.EQUAL;
        con[4] = new PropertyValue();
        con[4].Name = "SourcePosition";
        con[4].Value = ca;
        return con;
    }

    /**
    * Forces object environment recreation.
    */
    protected void after() {
        tEnv.dispose();
    }
}  // finish class _XSheetConditionalEntries


