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
* Testing <code>com.sun.star.chart.LineDiagram</code>
* service properties :
* <ul>
*  <li><code> SymbolType</code></li>
*  <li><code> SymbolSize</code></li>
*  <li><code> SymbolBitmapURL</code></li>
*  <li><code> Lines</code></li>
*  <li><code> SplineType</code></li>
* </ul> <p>
* The following predefined files needed to complete the test:
* <ul>
*  <li> <code>space-metal.jpg</code> :
*  for test of property 'SymbolBitmapURL' </li>
* <li> <code>crazy-blue.jpg</code> :
*  for test of property 'SymbolBitmapURL' </li>
* <ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CHARTDOC'</code> (of type <code>XChartDocument</code>):
*  to have reference to chart document </li>
*  <li> <code>'LINE'</code> (of type <code>XDiagram</code>):
*  relation that use as parameter for method setDiagram of chart document </li>
* </ul> <p>
* @see com.sun.star.chart.LineDiagram
*/
public class _LineDiagram extends MultiPropertyTest {

    XChartDocument doc = null;
    XDiagram oldDiagram = null;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    @Override
    protected void before() {
        log.println("Setting Diagram type to LineDiagram");
        doc = (XChartDocument) tEnv.getObjRelation("CHARTDOC");
        if (doc == null) throw new StatusException(Status.failed
            ("Relation 'CHARTDOC' not found"));

        XDiagram Line = (XDiagram) tEnv.getObjRelation("LINE");
        if (Line == null) throw new StatusException(Status.failed
            ("Relation 'LINE' not found"));

        oldDiagram = doc.getDiagram();
        doc.setDiagram(Line);
        oObj = UnoRuntime.queryInterface( XPropertySet.class, doc.getDiagram() );
    }

    /**
    * Sets the old diagram for a chart document.
    */
    @Override
    protected void after() {
        doc.setDiagram(oldDiagram);
    }

    protected PropertyTester URLTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (oldValue.equals(util.utils.getFullTestURL("space-metal.jpg")))
                return util.utils.getFullTestURL("crazy-blue.jpg"); else
                return util.utils.getFullTestURL("space-metal.jpg");
        }
    } ;

    protected PropertyTester SymbolTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {

            int a = com.sun.star.chart.ChartSymbolType.AUTO;
            int b = com.sun.star.chart.ChartSymbolType.NONE;
            if ( ((Integer) oldValue).intValue() == a)
                return Integer.valueOf(b); else
                return Integer.valueOf(a);
        }
    } ;

    protected PropertyTester SplineTester = new PropertyTester() {
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
    * Tests property 'SymbolType'.
    */
    public void _SymbolType() {
        log.println("Testing with custom Property tester") ;
        testProperty("SymbolType", SymbolTester) ;
        try {
            oObj.setPropertyValue("SymbolType",Integer.valueOf(-2));
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
    * Tests property 'SplineType'.
    */
    public void _SplineType() {
        log.println("Testing with custom Property tester") ;
        testProperty("SplineType", SplineTester) ;
    }

    /**
    * Tests property 'SymbolBitmapURL'.
    */
    public void _SymbolBitmapURL() {
        log.println("Testing with custom Property tester") ;
        try {
            oObj.setPropertyValue("SymbolType",
                Integer.valueOf(com.sun.star.chart.ChartSymbolType.BITMAPURL));
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

        testProperty("SymbolBitmapURL", URLTester) ;
    }
} // EOF LineDiagram

