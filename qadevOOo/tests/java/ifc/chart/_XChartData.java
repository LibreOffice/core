/*************************************************************************
 *
 *  $RCSfile: _XChartData.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:19:49 $
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

package ifc.chart;

import lib.MultiMethodTest;

import com.sun.star.chart.ChartDataChangeEvent;
import com.sun.star.chart.XChartData;
import com.sun.star.chart.XChartDataArray;
import com.sun.star.chart.XChartDataChangeEventListener;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.chart.XChartData</code>
* interface methods :
* <ul>
*  <li><code> addChartDataChangeEventListener()</code></li>
*  <li><code> removeChartDataChangeEventListener()</code></li>
*  <li><code> getNotANumber()</code></li>
*  <li><code> isNotANumber()</code></li>
* </ul> <p>
* @see com.sun.star.chart.XChartData
*/
public class _XChartData extends MultiMethodTest {

    public XChartData    oObj = null;
    boolean    result = true;
    double nan = 0;
    XChartDataArray dataArray = null;

    boolean dataChanged[] = new boolean[2];

    class MyEventListener implements XChartDataChangeEventListener {
            public void disposing ( EventObject oEvent ) {
                System.out.println("Listener1 disposed");
            }
        public void chartDataChanged(ChartDataChangeEvent ev) {
            dataChanged[0] = true;
        }
    }

    class MyEventListener2 implements XChartDataChangeEventListener {
            public void disposing ( EventObject oEvent ) {
                System.out.println("Listener2 disposed");
            }
        public void chartDataChanged(ChartDataChangeEvent ev) {
            dataChanged[1] = true;
        }
    }

    XChartDataChangeEventListener listener1 = new MyEventListener();
    XChartDataChangeEventListener listener2 = new MyEventListener2();

    /**
    * Test calls the method adding two listeners and then changes data. <p>
    * Has <b> OK </b> status if after data were changed
    * listeners were called. <p>
    */
    public void _addChartDataChangeEventListener() {
        dataChanged[0] = false;
        dataChanged[1] = false;

        oObj.addChartDataChangeEventListener( listener1 );
        oObj.addChartDataChangeEventListener( listener2 );

        dataArray = (XChartDataArray)
            UnoRuntime.queryInterface(XChartDataArray.class, oObj);
        double data[][] = dataArray.getData();
        data[0][0] += 0.1;
        dataArray.setData(data);
        if (!dataChanged[0]) log.println("ChartDataChangeEventListener1 "+
            "isn't called after changing data");
        if (!dataChanged[1]) log.println("ChartDataChangeEventListener2 "+
            "isn't called after changing data");
        tRes.tested("addChartDataChangeEventListener()",
            dataChanged[0] && dataChanged[1]);
    }

    /**
    * Test calls the method for one listener, changes data,
    * calls the method for other listener and again changes data. <p>
    * Has <b> OK </b> status if listener is not called after removing. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code>addChartDataChangeEventListener</code> : to have listeners
    *  that must be removed by the method </li>
    * </ul>
    */
    public void _removeChartDataChangeEventListener() {
        requiredMethod("addChartDataChangeEventListener()");

        dataChanged[0] = false;
        dataChanged[1] = false;

        oObj.removeChartDataChangeEventListener( listener1 );
        dataArray = (XChartDataArray)
            UnoRuntime.queryInterface(XChartDataArray.class, oObj);
        double data[][] = dataArray.getData();
        data[0][0] += 0.1;
        dataArray.setData(data);
        oObj.removeChartDataChangeEventListener( listener2 );
        if (dataChanged[0]) log.println("ChartDataChangeEventListener1 is "+
            "called after removing listener");
        tRes.tested("removeChartDataChangeEventListener()",
            ((!dataChanged[0]) && (dataChanged[1])));
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if the return value isn't equal to 1. <p>
    */
    public void _getNotANumber() {
        result = true;

        nan = oObj.getNotANumber();
        log.println("Current NotANumber is " + nan);
        result = nan!=1;

        tRes.tested("getNotANumber()", result);
    }

    /**
    * Test calls the method with NAN value and with non NAN value. <p>
    * Has <b> OK </b> status if the method returns true for NAN value and
    * returns false for other value<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code>getNotANumber</code> : to have the current NAN value </li>
    * </ul>
    */
    public void _isNotANumber() {
        requiredMethod("getNotANumber()");
        result = true;

        result = (oObj.isNotANumber(nan) && !oObj.isNotANumber(nan + 1));

        tRes.tested("isNotANumber()", result);
    }
}


