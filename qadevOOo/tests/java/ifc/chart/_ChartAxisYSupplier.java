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
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.chart.ChartAxisYSupplier</code>
* service properties :
* <ul>
*  <li><code> HasYAxis</code></li>
*  <li><code> HasYAxisDescription</code></li>
*  <li><code> HasYAxisGrid</code></li>
*  <li><code> HasYAxisHelpGrid</code></li>
*  <li><code> HasYAxisTitle</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CHARTDOC'</code> (of type <code>XChartDocument</code>):
*  to have reference to chart document </li>
* </ul> <p>
* @see com.sun.star.chart.ChartAxisYSupplier
*/
public class _ChartAxisYSupplier extends MultiPropertyTest {

    /**
    * Retrieves object relations and prepares a chart document.
    * @throws StatusException if one of relations not found.
    */
    protected void before() {
        XChartDocument doc = (XChartDocument) tEnv.getObjRelation("CHARTDOC");
        if (doc == null) throw new StatusException(Status.failed
            ("Relation 'CHARTDOC' not found"));

        oObj = (XPropertySet)
            UnoRuntime.queryInterface( XPropertySet.class, doc.getDiagram() );
    }

    public void _HasYAxis() {
        try {
            log.println("Property HasYAxis");
            boolean res = ((Boolean)oObj.getPropertyValue(
                                            "HasYAxis")).booleanValue();
            if (!res)
                oObj.setPropertyValue("HasYAxis", Boolean.TRUE);
            // test connected property HasYAxisDescription
            if (!((Boolean)oObj.getPropertyValue(
                                        "HasYAxisDescription")).booleanValue())
                oObj.setPropertyValue("HasYAxisDescription", Boolean.TRUE);

            oObj.setPropertyValue("HasYAxis", Boolean.FALSE);
            boolean setVal = ((Boolean)oObj.getPropertyValue(
                                                "HasYAxis")).booleanValue();
            log.println("Start value: " + setVal);
            // description should also be false now
            setVal |= ((Boolean)oObj.getPropertyValue(
                                     "HasYAxisDescription")).booleanValue();
            log.println("Connected value axis description: " + setVal);

            oObj.setPropertyValue("HasYAxis", Boolean.TRUE);
            setVal |= !((Boolean)oObj.getPropertyValue(
                                                "HasYAxis")).booleanValue();
            log.println("Changed value: " + !setVal);

            // description should be true again
            setVal |= !((Boolean)oObj.getPropertyValue(
                                     "HasYAxisDescription")).booleanValue();
            log.println("Changed connected value axis description: "+!setVal);

            tRes.tested("HasYAxis", !setVal);
            // leave axis untouched
            oObj.setPropertyValue("HasYAxis", new Boolean(res));
        }
        catch (com.sun.star.lang.WrappedTargetException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasYAxis", false);
        }
        catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasYAxis", false);
        }
        catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasYAxis", false);
        }
        catch (com.sun.star.beans.PropertyVetoException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasYAxis", false);
        }
    }

    public void _HasYAxisDescription() {
        requiredMethod("HasYAxis");
        try {
            log.println("Property HasYAxisDescription");
            if (!((Boolean)oObj.getPropertyValue("HasYAxis")).booleanValue())
                oObj.setPropertyValue("HasYAxis", Boolean.TRUE);

            boolean res = ((Boolean)oObj.getPropertyValue(
                                        "HasYAxisDescription")).booleanValue();
            log.println("Start value: " + res);

            oObj.setPropertyValue("HasYAxisDescription", new Boolean(!res));
            boolean setValue = ((Boolean)oObj.getPropertyValue(
                                        "HasYAxisDescription")).booleanValue();
            log.println("Changed value: " + setValue);
            tRes.tested("HasYAxisDescription", res != setValue);
        }
        catch (com.sun.star.lang.WrappedTargetException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasYAxisDescription", false);
        }
        catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasYAxisDescription", false);
        }
        catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasYAxisDescription", false);
        }
        catch (com.sun.star.beans.PropertyVetoException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasYAxisDescription", false);
        }
    }

} // EOF ChartAxisYSupplier

