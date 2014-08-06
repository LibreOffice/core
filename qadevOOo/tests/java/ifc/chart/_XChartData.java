/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package ifc.chart;

import com.sun.star.chart.ChartDataChangeEvent;
import com.sun.star.chart.XChartData;
import com.sun.star.chart.XChartDataArray;
import com.sun.star.chart.XChartDataChangeEventListener;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;

import lib.MultiMethodTest;


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
    public XChartData oObj = null;
    boolean result = true;
    double nan = 0;
    XChartDataArray dataArray = null;
    boolean[] dataChanged = new boolean[2];
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

        oObj.addChartDataChangeEventListener(listener1);
        oObj.addChartDataChangeEventListener(listener2);

        dataArray = UnoRuntime.queryInterface(
                            XChartDataArray.class, oObj);

        double[][] data = dataArray.getData();
        data[0][0] += 0.1;
        dataArray.setData(data);

        if (!dataChanged[0]) {
            log.println("ChartDataChangeEventListener1 " +
                        "isn't called after changing data");
        }

        if (!dataChanged[1]) {
            log.println("ChartDataChangeEventListener2 " +
                        "isn't called after changing data");
        }

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

        oObj.removeChartDataChangeEventListener(listener1);
        dataArray = UnoRuntime.queryInterface(
                            XChartDataArray.class, oObj);

        double[][] data = dataArray.getData();
        data[0][0] += 0.1;
        dataArray.setData(data);
        oObj.removeChartDataChangeEventListener(listener2);

        if (dataChanged[0]) {
            log.println("ChartDataChangeEventListener1 is " +
                        "called after removing listener");
        }

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
        result = nan != 1;

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

    /**
    * Forces environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }

    class MyEventListener implements XChartDataChangeEventListener {
        public void disposing(EventObject oEvent) {
            System.out.println("Listener1 disposed");
        }

        public void chartDataChanged(ChartDataChangeEvent ev) {
            dataChanged[0] = true;
        }
    }

    class MyEventListener2 implements XChartDataChangeEventListener {
        public void disposing(EventObject oEvent) {
            System.out.println("Listener2 disposed");
        }

        public void chartDataChanged(ChartDataChangeEvent ev) {
            dataChanged[1] = true;
        }
    }
}