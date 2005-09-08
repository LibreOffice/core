/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XStatisticDisplay.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:15:21 $
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
import com.sun.star.chart.XStatisticDisplay;

/**
* Testing <code>com.sun.star.chart.XStatisticDisplay</code>
* interface methods :
* <ul>
*  <li><code> getMinMaxLine()</code></li>
*  <li><code> getUpBar()</code></li>
*  <li><code> getDownBar()</code></li>
* </ul> <p>
* @see com.sun.star.chart.XStatisticDisplay
*/
public class _XStatisticDisplay extends MultiMethodTest {

    public XStatisticDisplay oObj = null;
    boolean                   result = true;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getMinMaxLine() {
        result = true;

        XPropertySet MinMaxLine = oObj.getMinMaxLine();
        result = (MinMaxLine != null);

        tRes.tested("getMinMaxLine()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getUpBar() {
        result = true;

        XPropertySet UpBar = oObj.getUpBar();
        result = (UpBar != null);

        tRes.tested("getUpBar()", result);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _getDownBar() {
        result = true;

        XPropertySet DownBar = oObj.getDownBar();
        result = (DownBar != null);

        tRes.tested("getDownBar()", result);
    }

} // EOF XStatisticDisplay


