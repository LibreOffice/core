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
    @Override
    protected void before() {
        XChartDocument doc = (XChartDocument) tEnv.getObjRelation("CHARTDOC");
        if (doc == null) throw new StatusException(Status.failed
            ("Relation 'CHARTDOC' not found"));

        oObj = UnoRuntime.queryInterface( XPropertySet.class, doc.getDiagram() );
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
            oObj.setPropertyValue("HasYAxis", Boolean.valueOf(res));
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

            oObj.setPropertyValue("HasYAxisDescription", Boolean.valueOf(!res));
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

