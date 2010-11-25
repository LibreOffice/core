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
        oObj = (XPropertySet)
            UnoRuntime.queryInterface( XPropertySet.class, doc.getDiagram() );
        try {
            oObj.setPropertyValue("Dim3D", new Boolean(true));
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
    protected void after() {
        log.println("Setting Diagram back to 2D");
        XChartDocument doc = (XChartDocument) tEnv.getObjRelation("CHARTDOC");
        if (doc == null) throw new StatusException(Status.failed
            ("Relation 'CHARTDOC' not found"));

        log.println("Change Diagram to 3D");
        oObj = (XPropertySet)
            UnoRuntime.queryInterface( XPropertySet.class, doc.getDiagram() );
        try {
            oObj.setPropertyValue("Dim3D", new Boolean(false));
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


