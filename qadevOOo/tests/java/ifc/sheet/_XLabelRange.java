/*************************************************************************
 *
 *  $RCSfile: _XLabelRange.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:01:38 $
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
import util.ValueComparer;

import com.sun.star.sheet.XLabelRange;
import com.sun.star.table.CellRangeAddress;

/**
* Testing <code>com.sun.star.sheet.XLabelRange</code>
* interface methods :
* <ul>
*  <li><code> getLabelArea()</code></li>
*  <li><code> setLabelArea()</code></li>
*  <li><code> getDataArea()</code></li>
*  <li><code> setDataArea()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XLabelRange
*/
public class _XLabelRange extends MultiMethodTest {

    public XLabelRange oObj = null;
    public CellRangeAddress setDAddress = null;
    public CellRangeAddress setLAddress = null;

    /**
    * Test creates and stores <code>CellRangeAddress</code>, calls the method.
    * <p>Has <b> OK </b> status if the method successfully returns. <p>
    * @see com.sun.star.table.CellRangeAddress
    */
    public void _setDataArea() {
        int nr = Thread.activeCount();
        setDAddress = new CellRangeAddress((short)1, nr, 1, nr, 8);
        oObj.setDataArea(setDAddress);
        tRes.tested("setDataArea()", true);
    }

    /**
    * Test creates and stores <code>CellRangeAddress</code>, calls the method.
    * <p>Has <b> OK </b> status if the method successfully returns. <p>
    * @see com.sun.star.table.CellRangeAddress
    */
    public void _setLabelArea() {
        int nr = Thread.activeCount();
        setLAddress = new CellRangeAddress((short)1, nr, 0, nr, 0);
        oObj.setLabelArea(setLAddress);
        tRes.tested("setLabelArea()", true);
    }

    /**
    * Test calls the method and compares returned value with value that was set.
    * <p>Has <b> OK </b> status if values are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setDataArea() </code> : to have address of the cell range for
    *  which the labels are valid</li>
    * </ul>
    */
    public void _getDataArea() {
        requiredMethod("setDataArea()");
        CellRangeAddress gA = oObj.getDataArea();
        tRes.tested("getDataArea()", ValueComparer.equalValue(gA, setDAddress));
    }

    /**
    * Test calls the method and compares returned value with value set before.
    * <p>Has <b> OK </b> status if if values are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setLabelArea() </code>: to have the cell range that contains
    *  the labels</li>
    * </ul>
    */
    public void _getLabelArea() {
        requiredMethod("setLabelArea()");
        CellRangeAddress gA1 = oObj.getLabelArea();
        tRes.tested("getLabelArea()",
            ValueComparer.equalValue(gA1, setLAddress));
    }
}  // finish class _XLabelRange


