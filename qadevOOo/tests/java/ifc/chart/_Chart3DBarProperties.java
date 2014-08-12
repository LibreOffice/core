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

import lib.MultiPropertyTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XPropertySet;
import com.sun.star.chart.XChartDocument;
import com.sun.star.chart.XDiagram;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.chart.Chart3DBarProperties</code>
* service properties:
* <ul>
*  <li><code> SolidType</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CHARTDOC'</code> (of type <code>XChartDocument</code>):
*   to have reference to chart document </li>
*  <li> <code>'BAR'</code> (of type <code>XDiagram</code>):
*   relation that use as parameter for method setDiagram of chart document </li>
* <ul> <p>
* @see com.sun.star.chart.Chart3DBarProperties
* @see com.sun.star.chart.XChartDocument
* @see com.sun.star.chart.XDiagram
*/
public class _Chart3DBarProperties extends MultiPropertyTest {

    /**
    * Retrieves object relations and prepares a chart document.
    * @throws StatusException if one of relations not found.
    */
    @Override
    protected void before() {
        log.println("Setting Diagram type to BarDiagram");
        XChartDocument doc = (XChartDocument) tEnv.getObjRelation("CHARTDOC");
        if (doc == null) throw new StatusException(Status.failed
            ("Relation 'CHARTDOC' not found"));

        XDiagram bar = (XDiagram) tEnv.getObjRelation("BAR");
        if (bar == null) throw new StatusException(Status.failed
            ("Relation 'BAR' not found"));

        doc.setDiagram(bar);
        log.println("Change Diagram to 3D");
        oObj = UnoRuntime.queryInterface( XPropertySet.class, doc.getDiagram() );
        try {
            oObj.setPropertyValue("Dim3D", Boolean.TRUE);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't change Diagram to 3D");
            e.printStackTrace(log);
            throw new StatusException("Couldn't change Diagram to 3D", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't change Diagram to 3D");
            e.printStackTrace(log);
            throw new StatusException("Couldn't change Diagram to 3D", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Couldn't change Diagram to 3D");
            e.printStackTrace(log);
            throw new StatusException("Couldn't change Diagram to 3D", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Couldn't change Diagram to 3D");
            e.printStackTrace(log);
            throw new StatusException("Couldn't change Diagram to 3D", e);
        }
    }

    /**
    * Sets the diagram back to 2D as 2D rendering is much faster for the following tests.
    */
    @Override
    protected void after() {
        log.println("Setting Diagram back to 2D");
        XChartDocument doc = (XChartDocument) tEnv.getObjRelation("CHARTDOC");
        if (doc == null) throw new StatusException(Status.failed
            ("Relation 'CHARTDOC' not found"));

        log.println("Change Diagram to 3D");
        oObj = UnoRuntime.queryInterface( XPropertySet.class, doc.getDiagram() );
        try {
            oObj.setPropertyValue("Dim3D", Boolean.FALSE);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't change Diagram back to 2D");
            e.printStackTrace(log);
            throw new StatusException("Couldn't change Diagram back to 2D", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't change Diagram back to 2D");
            e.printStackTrace(log);
            throw new StatusException("Couldn't change Diagram back to 2D", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Couldn't change Diagram back to 2D");
            e.printStackTrace(log);
            throw new StatusException("Couldn't change Diagram back to 2D", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Couldn't change Diagram back to 2D");
            e.printStackTrace(log);
            throw new StatusException("Couldn't change Diagram back to 2D", e);
        }
    }

}  // finish class _Chart3DBarProperties


