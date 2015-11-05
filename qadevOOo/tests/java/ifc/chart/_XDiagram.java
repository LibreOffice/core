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
    @Override
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
        result = stype.startsWith("com.sun.star.chart.");

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


