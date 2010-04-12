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
            oObj.setPropertyValue("HasXAxis", new Boolean(res));
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

            oObj.setPropertyValue("HasXAxisDescription", new Boolean(!res));
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

