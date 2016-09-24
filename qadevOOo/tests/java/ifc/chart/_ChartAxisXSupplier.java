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

/**
* Testing <code>com.sun.star.chart.ChartAxisXSupplier</code>
* service properties :
* <ul>
*  <li><code> HasXAxis</code></li>
*  <li><code> HasXAxisDescription</code></li>
*  <li><code> HasXAxisGrid</code></li>
*  <li><code> HasXAxisHelpGrid</code></li>
*  <li><code> HasXAxisTitle</code></li>
* </ul> <p>
* @see com.sun.star.chart.ChartAxisXSupplier
*/
public class _ChartAxisXSupplier extends MultiPropertyTest {
    public void _HasXAxis() {
        try {
            log.println("Property HasXAxis");
            boolean res = ((Boolean)oObj.getPropertyValue(
                                            "HasXAxis")).booleanValue();
            if (!res)
                oObj.setPropertyValue("HasXAxis", Boolean.TRUE);
            // test connected property HasXAxisDescription
            if (!((Boolean)oObj.getPropertyValue(
                                        "HasXAxisDescription")).booleanValue())
                oObj.setPropertyValue("HasXAxisDescription", Boolean.TRUE);

            oObj.setPropertyValue("HasXAxis", Boolean.FALSE);
            boolean setVal = ((Boolean)oObj.getPropertyValue(
                                                "HasXAxis")).booleanValue();
            log.println("Start value: " + setVal);
            // description should also be false now
            setVal |= ((Boolean)oObj.getPropertyValue(
                                     "HasXAxisDescription")).booleanValue();
            log.println("Connected value axis description: " + setVal);

            oObj.setPropertyValue("HasXAxis", Boolean.TRUE);
            setVal |= !((Boolean)oObj.getPropertyValue(
                                                "HasXAxis")).booleanValue();
            log.println("Changed value: " + !setVal);

            // description should be true again
            setVal |= !((Boolean)oObj.getPropertyValue(
                                     "HasXAxisDescription")).booleanValue();
            log.println("Changed connected value axis description: " + !setVal);

            tRes.tested("HasXAxis", !setVal);
            // leave axis untouched
            oObj.setPropertyValue("HasXAxis", Boolean.valueOf(res));
        }
        catch (com.sun.star.lang.WrappedTargetException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasXAxis", false);
        }
        catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasXAxis", false);
        }
        catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasXAxis", false);
        }
        catch (com.sun.star.beans.PropertyVetoException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasXAxis", false);
        }
    }

    public void _HasXAxisDescription() {
        requiredMethod("HasXAxis");
        try {
            log.println("Property HasXAxisDescription");
            if (!((Boolean)oObj.getPropertyValue("HasXAxis")).booleanValue())
                oObj.setPropertyValue("HasXAxis", Boolean.TRUE);

            boolean res = ((Boolean)oObj.getPropertyValue(
                                        "HasXAxisDescription")).booleanValue();
            log.println("Start value: " + res);

            oObj.setPropertyValue("HasXAxisDescription", Boolean.valueOf(!res));
            boolean setValue = ((Boolean)oObj.getPropertyValue(
                                        "HasXAxisDescription")).booleanValue();
            log.println("Changed value: " + setValue);
            tRes.tested("HasXAxisDescription", res != setValue);
        }
        catch (com.sun.star.lang.WrappedTargetException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasXAxisDescription", false);
        }
        catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasXAxisDescription", false);
        }
        catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasXAxisDescription", false);
        }
        catch (com.sun.star.beans.PropertyVetoException e) {
            log.println(e.getMessage());
            e.printStackTrace(log);
            tRes.tested("HasXAxisDescription", false);
        }
    }
} // EOF ChartAxisXSupplier

