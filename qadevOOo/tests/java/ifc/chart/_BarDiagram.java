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
* Testing <code>com.sun.star.chart.BarDiagram</code>
* service properties:
* <ul>
*   <li><code>'Vertical'</code></li>
*   <li><code>'Deep'</code></li>
*   <li><code>'StackedBarsConnected'</code></li>
*   <li><code>'NumberOfLines'</code></li>
* </ul>
* Properties testing is automated
* by <code>lib.MultiPropertyTest</code> except property
* <code>'NumberOfLines'</code>. <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CHARTDOC'</code> (of type <code>XChartDocument</code>):
*  to have reference to chart document </li>
*  <li> <code>'BAR'</code> (of type <code>XDiagram</code>):
*  relation that use as parameter for method setDiagram of chart document </li>
* <ul> <p>
* @see com.sun.star.chart.BarDiagram
* @see com.sun.star.chart.XChartDocument
* @see com.sun.star.chart.XDiagram
* @see lib.MultiPropertyTest
*/
public class _BarDiagram extends MultiPropertyTest {

    XChartDocument doc = null;
    XDiagram oldDiagram = null;

    /**
    * Retrieves object relations and prepares a chart document.
    * @throws StatusException if one of relations not found.
    */
    @Override
    protected void before() {
        log.println("Setting Diagram type to BarDiagram");
        doc = (XChartDocument) tEnv.getObjRelation("CHARTDOC");
        if (doc == null) throw new StatusException(Status.failed
            ("Relation 'CHARTDOC' not found"));

        XDiagram bar = (XDiagram) tEnv.getObjRelation("BAR");
        if (bar == null) throw new StatusException(Status.failed
            ("Relation 'BAR' not found"));

        oldDiagram = doc.getDiagram();
        doc.setDiagram(bar);
        oObj = UnoRuntime.queryInterface( XPropertySet.class, doc.getDiagram() );
        log.println("Set it to 3D");
        try {
            oObj.setPropertyValue("Dim3D", Boolean.TRUE);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception while set property value");
            e.printStackTrace(log);
            throw new StatusException("Exception while set property value", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception while set property value");
            e.printStackTrace(log);
            throw new StatusException("Exception while set property value", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Exception while set property value");
            e.printStackTrace(log);
            throw new StatusException("Exception while set property value", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception while set property value");
            e.printStackTrace(log);
            throw new StatusException("Exception while set property value", e);
        }
    }

    /**
    * Sets the old diagram for a chart document.
    */
    @Override
    protected void after() {
        doc.setDiagram(oldDiagram);
    }

    protected PropertyTester LineTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            int a = 0;
            int b = 2;
            if ( ((Integer) oldValue).intValue() == a)
                return Integer.valueOf(b); else
                return Integer.valueOf(a);
        }
    } ;

    /**
    * Tests property 'NumberOfLines'.
    * This property tests when diagram in 2D-mode only
    * except all other properties. This property is currently supported by
    * two dimensional vertical bar charts only.
    */
    public void _NumberOfLines() {
        log.println("Set it to 2D");
        try {
            oObj.setPropertyValue("Dim3D", Boolean.FALSE);
            oObj.setPropertyValue("Vertical", Boolean.FALSE);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception while set property value");
            e.printStackTrace(log);
            throw new StatusException("Exception while set property value", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception while set property value");
            e.printStackTrace(log);
            throw new StatusException("Exception while set property value", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Exception while set property value");
            e.printStackTrace(log);
            throw new StatusException("Exception while set property value", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception while set property value");
            e.printStackTrace(log);
            throw new StatusException("Exception while set property value", e);
        }

        log.println("Testing with custom Property tester") ;
        testProperty("NumberOfLines", LineTester) ;
    }
} // EOF BarDiagram

