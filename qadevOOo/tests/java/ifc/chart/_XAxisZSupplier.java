/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XAxisZSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:14:10 $
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
import com.sun.star.chart.XAxisZSupplier;
import com.sun.star.drawing.XShape;

/**
* Testing <code>com.sun.star.chart.XAxisZSupplier</code>
* interface methods :
* <ul>
*  <li><code> getZMainGrid()</code></li>
*  <li><code> getZAxisTitle()</code></li>
*  <li><code> getZAxis()</code></li>
*  <li><code> getZHelpGrid()</code></li>
* </ul> <p>
* @see com.sun.star.chart.XAxisZSupplier
*/
public class _XAxisZSupplier extends MultiMethodTest {
    public XAxisZSupplier oObj = null;
    boolean            result = true;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getZMainGrid() {
        result = true;

        XPropertySet MGrid = oObj.getZMainGrid();
        result = (MGrid != null);

        tRes.tested("getZMainGrid()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getZAxisTitle() {
        result = true;

        XShape title = oObj.getZAxisTitle();
        result = (title != null);

        tRes.tested("getZAxisTitle()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getZAxis() {
        result = true;

        XPropertySet axis = oObj.getZAxis();
        result = (axis != null);

        tRes.tested("getZAxis()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the method returns
    * value that isn't null. <p>
    */
    public void _getZHelpGrid() {
        result = true;

        XPropertySet HGrid = oObj.getZHelpGrid();
        result = (HGrid != null);

        tRes.tested("getZHelpGrid()", result);
    }

} // EOF XAxisZSupplier


