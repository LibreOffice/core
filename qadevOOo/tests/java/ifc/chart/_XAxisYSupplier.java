/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XAxisYSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:13:53 $
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

package ifc.chart;

import lib.MultiMethodTest;

import com.sun.star.beans.XPropertySet;
import com.sun.star.chart.XAxisYSupplier;
import com.sun.star.drawing.XShape;

/**
* Testing <code>com.sun.star.chart.XAxisYSupplier</code>
* interface methods :
* <ul>
*  <li><code> getYMainGrid()</code></li>
*  <li><code> getYAxisTitle()</code></li>
*  <li><code> getYAxis()</code></li>
*  <li><code> getYHelpGrid()</code></li>
* </ul> <p>
* @see com.sun.star.chart.XAxisYSupplier
*/
public class _XAxisYSupplier extends MultiMethodTest {

    public XAxisYSupplier oObj = null;
    boolean                result = true;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getYMainGrid() {
        result = true;

        XPropertySet MGrid = oObj.getYMainGrid();
        result = (MGrid != null);

        tRes.tested("getYMainGrid()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getYAxisTitle() {
        result = true;

        XShape title = oObj.getYAxisTitle();
        result = (title != null);

        tRes.tested("getYAxisTitle()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getYAxis() {
        result = true;

        XPropertySet axis = oObj.getYAxis();
        result = (axis != null);

        tRes.tested("getYAxis()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getYHelpGrid() {
        result = true;

        XPropertySet HGrid = oObj.getYHelpGrid();
        result = (HGrid != null);

        tRes.tested("getYHelpGrid()", result);
    }
} // EOF XAxisYSupplier


