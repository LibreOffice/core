/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package ifc.chart;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XPropertySet;
import com.sun.star.chart.XDiagram;

/**
* Testing <code>com.sun.star.chart.XDiagram</code>
* interface methods :
* <ul>
*  <li><code> getDiagramType()</code></li>
*  <li><code> getDataRowProperties()</code></li>
*  <li><code> getDataPointProperties()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'ROWAMOUNT'</code> (of type <code>Integer</code>):
*   to have amount of rows </li>
*  <li> <code>'COLAMOUNT'</code> (of type <code>Integer</code>):
*   to have amount of columns </li>
* <ul> <p>
* @see com.sun.star.chart.XDiagram
*/
public class _XDiagram extends MultiMethodTest {

    public XDiagram      oObj = null;
    boolean         result = true;
    Integer      rowamount = null;
    Integer      colamount = null;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    public void before() {
        rowamount = (Integer)tEnv.getObjRelation("ROWAMOUNT");
        if (rowamount == null) throw new StatusException(Status.failed
            ("Relation 'ROWAMOUNT' not found"));

        colamount = (Integer)tEnv.getObjRelation("COLAMOUNT");
        if (colamount == null) throw new StatusException(Status.failed
            ("Relation 'COLAMOUNT' not found"));
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value start from 'com.sun.star.chart.' <p>
    */
    public void _getDiagramType() {
        result = true;

        String stype = oObj.getDiagramType();
        log.println("Current Diagram Type is " + stype);
        result = (stype.startsWith("com.sun.star.chart."));

        tRes.tested("getDiagramType()", result);
    }

    /**
    * Test calls the method for every row and checks returned value. <p>
    * Has <b> OK </b> status if returned value for every row isn't null and
    * no exceptions were thrown. <p>
    */
    public void _getDataRowProperties() {
        result = true;

        int rows = rowamount.intValue();
        rows -= 1;
        XPropertySet props = null;

        log.println("There are " + rows + " rows.");
        try {
            for (int i = 0; i < rows; i++) {
                props = oObj.getDataRowProperties(i);
                if (props != null) {
                    log.println("Row " + i + " - OK");
                } else {
                    log.println("Row " + i + " - FAILED");
                    result = false;
                }
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception while get data row properties");
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("getDataRowProperties()", result);
    }

    /**
    * Test calls the method for every point and checks returned value. <p>
    * Has <b> OK </b> status if returned value for every point isn't null and
    * no exceptions were thrown. <p>
    */
    public void _getDataPointProperties() {
        result = true;

        int rows = rowamount.intValue();
        int cols = colamount.intValue();
        XPropertySet props = null;

        log.println("There are " + rows + " rows and " + cols + " cols.");

        try {
            for (int i = 0; i < rows; i++)
                for (int j = 0; j < cols; j++) {
                    props = oObj.getDataPointProperties(i, j);
                    if (props != null) {
                        log.println("Row " + i + " Col " + j + " - OK");
                    } else {
                        log.println("Row " + i + " Col " + j + " - FAILED");
                        result = false;
                    }
                }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception while get data point properties");
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("getDataPointProperties()", result);
    }
}


