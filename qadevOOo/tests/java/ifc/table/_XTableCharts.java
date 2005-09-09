/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XTableCharts.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:08:17 $
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

import com.sun.star.awt.Rectangle;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XTableCharts;

/**
* Testing <code>com.sun.star.table.XTableCharts</code>
* interface methods :
* <ul>
*  <li><code> addNewByName()</code></li>
*  <li><code> removeByName()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'ADDR'</code>
*  (of type <code>com.sun.star.table.CellRangeAddress[]</code>):
*   data source ranges for chart creating. </li>
*  <li> <code>'RECT'</code>
*  (of type <code>com.sun.star.awt.Rectangle</code>):
*   the position of chart </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.table.XTableCharts
*/
public class _XTableCharts extends MultiMethodTest {

    public XTableCharts oObj = null;

    /**
     * Adds a new chart into table with specified name . <p>
     * Has <b> OK </b> status if <code>hasByName()</code> returns
     * <code>true</code>. <p>
     */
    public void _addNewByName() {

        CellRangeAddress[] the_Range = (CellRangeAddress[])
            tEnv.getObjRelation("ADDR");
        Rectangle the_rect = (Rectangle) tEnv.getObjRelation("RECT");
        oObj.addNewByName("XTableCharts",the_rect,the_Range,true,true);
        tRes.tested("addNewByName()", oObj.hasByName("XTableCharts") );

    } // addNewByName()

    /**
     * Removes the chart added before. <p>
     * Has <b> OK </b> status if <code>hasByName()</code> returns
     * <code>false</code>. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> addNewByName() </code> : chart mst be added first </li>
     * </ul>
     */
    public void _removeByName() {
        requiredMethod("addNewByName()") ;

        oObj.removeByName("XTableCharts");
        tRes.tested("removeByName()", !oObj.hasByName("XTableCharts") );

    } // removeByName()

} // finish class _XTableCharts



