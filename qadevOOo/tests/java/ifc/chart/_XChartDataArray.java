/*************************************************************************
 *
 *  $RCSfile: _XChartDataArray.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-11-18 16:20:33 $
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

    protected void before() {
        Object o = tEnv.getObjRelation("CRDESC");
        if (o != null) {
            mbExcludeSetRowAndSetColumn = true;
            msExcludeMessage = (String)o;
        }
        if (!mbExcludeSetRowAndSetColumn) {
            XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, oObj);
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

        for (int i = 0; i < rowDscs.length; i++) {
            for (int j = 0; j < colDscs.length; j++)
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
        bResult = true;

        String[] dscs = oObj.getColumnDescriptions();
        bResult &= dscs.length == colDscs.length;
        if (bResult) {
            for (int i = 0; i < dscs.length; i++) {
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
        bResult = true;

        String[] dscs = oObj.getRowDescriptions();
        bResult &= dscs.length == rowDscs.length;
        if (bResult) {
            for (int i = 0; i < dscs.length; i++) {
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
        for (int i = 0; i < rowDscs.length; i++) {
            for (int j = 0; j < colDscs.length; j++) {
                bResult &= data[i][j] == _data[i][j];
            }
        }

        tRes.tested("getData()", bResult);
    }

    protected void after() {
        disposeEnvironment();
    }
}


