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
        oObj = (XPropertySet)
            UnoRuntime.queryInterface( XPropertySet.class, doc.getDiagram() );
    }

    /**
    * Sets the old diagram for a chart document.
    */
    protected void after() {
        doc.setDiagram(oldDiagram);
    }

    protected PropertyTester URLTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (oldValue.equals(util.utils.getFullTestURL("space-metal.jpg")))
                return util.utils.getFullTestURL("crazy-blue.jpg"); else
                return util.utils.getFullTestURL("space-metal.jpg");
        }
    } ;

    protected PropertyTester SymbolTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {

            int a = com.sun.star.chart.ChartSymbolType.AUTO;
            int b = com.sun.star.chart.ChartSymbolType.NONE;
            if ( ((Integer) oldValue).intValue() == a)
                return new Integer(b); else
                return new Integer(a);
        }
    } ;

    protected PropertyTester SplineTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {

            int a = 0;
            int b = 2;
            if ( ((Integer) oldValue).intValue() == a)
                return new Integer(b); else
                return new Integer(a);
        }
    } ;

    /**
    * Tests property 'SymbolType'.
    */
    public void _SymbolType() {
        log.println("Testing with custom Property tester") ;
        testProperty("SymbolType", SymbolTester) ;
        try {
            oObj.setPropertyValue("SymbolType",new Integer(-2));
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
                new Integer(com.sun.star.chart.ChartSymbolType.BITMAPURL));
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

