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
* Testing <code>com.sun.star.chart.ChartAxisZSupplier</code>
* service properties :
* <ul>
*  <li><code> HasZAxis</code></li>
*  <li><code> HasZAxisDescription</code></li>
*  <li><code> HasZAxisGrid</code></li>
*  <li><code> HasZAxisHelpGrid</code></li>
*  <li><code> HasZAxisTitle</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CHARTDOC'</code> (of type <code>XChartDocument</code>):
*  to have reference to chart document </li>
*  <li> <code>'BAR'</code> (of type <code>XDiagram</code>):
*  relation that use as parameter for method setDiagram of chart document</li>
* </ul> <p>
* @see com.sun.star.chart.ChartAxisZSupplier
*/
public class _ChartAxisZSupplier extends MultiPropertyTest {

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
        log.println("Set it to 3D");
        oObj = UnoRuntime.queryInterface( XPropertySet.class, doc.getDiagram() );
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

    public void _HasZAxis() {
        try {
            log.println("Property HasZAxis");
            boolean res = ((Boolean)oObj.getPropertyValue(
                                            "HasZAxis")).booleanValue();
            if (!res)
                oObj.setPropertyValue("HasZAxis", Boolean.TRUE);
            // test connected property HasZAxisDescription
            if (!((Boolean)oObj.getPropertyValue(
                                        "HasZAxisDescription")).booleanValue())
                oObj.setPropertyValue("HasZAxisDescription", Boolean.TRUE);

            oObj.setPropertyValue("HasZAxis", Boolean.FALSE);
            boolean setVal = ((Boolean)oObj.getPropertyValue(
                                                "HasZAxis")).booleanValue();
            log.println("Start value: " + setVal);
            // description should also be false now
            setVal |= ((Boolean)oObj.getPropertyValue(
                                     "HasZAxisDescription")).booleanValue();
            log.println("Connected value axis description: " + setVal);

            oObj.setPropertyValue("HasZAxis", Boolean.TRUE);
            setVal |= !((Boolean)oObj.getPropertyValue(
                                                "HasZAxis")).booleanValue();
            log.println("Changed value: " + !setVal);

            // description should be true again
            setVal |= !((Boolean)oObj.getPropertyValue(
                                     "HasZAxisDescription")).booleanValue();
            log.println("Changed connected value axis description: " + !setVal);

            tRes.tested("HasZAxis", !setVal);
            // leave axis untouched
            oObj.setPropertyValue("HasZAxis", Boolean.valueOf(res));
        }
        catch (com.sun.star.lang.WrappedTargetException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasZAxis", false);
        }
        catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasZAxis", false);
        }
        catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasZAxis", false);
        }
        catch (com.sun.star.beans.PropertyVetoException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasZAxis", false);
        }
    }

    public void _HasZAxisDescription() {
        requiredMethod("HasZAxis");
        try {
            log.println("Property HasZAxisDescription");
            if (!((Boolean)oObj.getPropertyValue("HasZAxis")).booleanValue())
                oObj.setPropertyValue("HasZAxis", Boolean.TRUE);

            boolean res = ((Boolean)oObj.getPropertyValue(
                                        "HasZAxisDescription")).booleanValue();
            log.println("Start value: " + res);

            oObj.setPropertyValue("HasZAxisDescription", Boolean.valueOf(!res));
            boolean setValue = ((Boolean)oObj.getPropertyValue(
                                        "HasZAxisDescription")).booleanValue();
            log.println("Changed value: " + setValue);
            tRes.tested("HasZAxisDescription", res != setValue);
        }
        catch (com.sun.star.lang.WrappedTargetException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasZAxisDescription", false);
        }
        catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasZAxisDescription", false);
        }
        catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasZAxisDescription", false);
        }
        catch (com.sun.star.beans.PropertyVetoException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasZAxisDescription", false);
        }
    }


    /**
    * Forces environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }

} // EOF ChartAxisZSupplier

