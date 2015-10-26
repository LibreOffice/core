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

import com.sun.star.beans.XPropertySet;
import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.chart.XChartDataArray;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.chart.XChartDataArray</code>
* interface methods :
* <ul>
*  <li><code> getColumnDescriptions()</code></li>
*  <li><code> getData()</code></li>
*  <li><code> getRowDescriptions()</code></li>
*  <li><code> setColumnDescriptions()</code></li>
*  <li><code> setData()</code></li>
*  <li><code> setRowDescriptions()</code></li>
* </ul> <p>
* @see com.sun.star.chart.XChartDataArray
*/
public class _XChartDataArray extends MultiMethodTest {

    public XChartDataArray    oObj = null;
    boolean    bResult = true;
    String[] colDscs = new String[3];
    String[] rowDscs = new String[3];
    double[][] data = null;
    private boolean mbExcludeSetRowAndSetColumn = false;
    private String msExcludeMessage;

    @Override
    protected void before() {
        Object o = tEnv.getObjRelation("CRDESC");
        if (o != null) {
            mbExcludeSetRowAndSetColumn = true;
            msExcludeMessage = (String)o;
        }
        if (mbExcludeSetRowAndSetColumn) {
            return;
        }
        XPropertySet xProp = UnoRuntime.queryInterface(XPropertySet.class, oObj);
        if(xProp != null) {
            try {
                boolean columnAsLabel = ((Boolean)xProp.getPropertyValue("ChartColumnAsLabel")).booleanValue();
                boolean rowAsLabel = ((Boolean)xProp.getPropertyValue("ChartRowAsLabel")).booleanValue();
                if (!columnAsLabel) {
                    xProp.setPropertyValue("ChartColumnAsLabel", Boolean.TRUE);
                }
                if (!rowAsLabel) {
                    xProp.setPropertyValue("ChartRowAsLabel", Boolean.TRUE);
                }
            }
            catch(Exception e) {
                // ignore
            }
        }
    }

    /**
    * Test calls the method and restores new values. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _setColumnDescriptions() {
        bResult = true;

        colDscs = oObj.getColumnDescriptions();
        if (mbExcludeSetRowAndSetColumn) {
            log.println(msExcludeMessage);
            throw new StatusException(Status.skipped(true));
        }
        for (int i = 0; i < colDscs.length; i++) {
            colDscs[i] = "Col" + i;
        }
        oObj.setColumnDescriptions(colDscs);

        tRes.tested("setColumnDescriptions()", bResult);
    }

    /**
    * Test calls the method and restores new values. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setColumnDescriptions </code></li>
    * </ul>
    */
    public void _setRowDescriptions() {
        bResult = true;

        rowDscs = oObj.getRowDescriptions();
        if (mbExcludeSetRowAndSetColumn) {
            log.println(msExcludeMessage);
            throw new StatusException(Status.skipped(true));
        }
        for (int i = 0; i < rowDscs.length; i++) {
            rowDscs[i] = "Row" + i;
        }
        oObj.setRowDescriptions(rowDscs);

        tRes.tested("setRowDescriptions()", bResult);
    }

    /**
    * Test calls the method and restores new values. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setRowDescriptions </code></li>
    * </ul>
    */
    public void _setData() {
        rowDscs = oObj.getRowDescriptions();
        colDscs = oObj.getColumnDescriptions();

        bResult = true;
        double[][] _data = oObj.getData();
        data = _data;
        if(rowDscs.length != 0 && rowDscs.length != data.length)
            bResult = false;
        if(colDscs.length != 0 && colDscs.length != data[0].length)
            bResult = false;
        for (int i = 0; i < data.length; i++) {
            for (int j = 0; j < data[i].length; j++)
                data[i][j] = i * (j + 1);
        }
        oObj.setData(data);

        tRes.tested("setData()", bResult);
    }

    /**
    * Test calls the method and compare returned values with values restored
    * after method <code>setColumnDescriptions</code>. <p>
    * Has <b> OK </b> status if the returned values equils to restored values. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setData </code> : to set and restore new values </li>
    * </ul>
    */
    public void _getColumnDescriptions() {
        requiredMethod("setColumnDescriptions()");

        String[] dscs = oObj.getColumnDescriptions();
        bResult = dscs.length == colDscs.length;
        if (bResult) {
            for (int i = 0; i < dscs.length; i++) {
                log.println("Col " + i + ": got " + dscs[i] + " expected: " + colDscs[i]);
                bResult &= dscs[i].equals(colDscs[i]);
            }
        }

        tRes.tested("getColumnDescriptions()", bResult);
    }

    /**
    * Test calls the method and compare returned values with values restored
    * after method <code>setRowDescriptions</code>. <p>
    * Has <b> OK </b> status if the returned values equils to restored values. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setData </code> : to set and restore new values </li>
    * </ul>
    */
    public void _getRowDescriptions() {
        requiredMethod("setRowDescriptions()");

        String[] dscs = oObj.getRowDescriptions();
        bResult = dscs.length == rowDscs.length;
        if (bResult) {
            for (int i = 0; i < dscs.length; i++) {
                log.println("Row " + i + ": got " + dscs[i] + " expected: " + rowDscs[i]);
                bResult &= dscs[i].equals(rowDscs[i]);
            }
        }

        tRes.tested("getRowDescriptions()", bResult);
    }

    /**
    * Test calls the method and compare returned values with values restored
    * after method <code>setData</code>. <p>
    * Has <b> OK </b> status if the returned values equils to restored values. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setData </code> : to set and restore new values </li>
    * </ul>
    */
    public void _getData() {
        requiredMethod("setData()");
        bResult = true;

        double[][] _data = oObj.getData();
        data = _data;
        if(rowDscs.length != 0 && rowDscs.length != data.length)
            bResult = false;
        if(colDscs.length != 0 && colDscs.length != data[0].length)
            bResult = false;
        for (int i = 0; i < data.length; i++) {
            for (int j = 0; j < data[i].length; j++) {
                bResult &= data[i][j] == _data[i][j];
            }
        }

        tRes.tested("getData()", bResult);
    }

    @Override
    protected void after() {
        disposeEnvironment();
    }
}


