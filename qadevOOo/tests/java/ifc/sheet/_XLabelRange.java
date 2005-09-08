/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XLabelRange.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:49:14 $
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


