/*************************************************************************
 *
 *  $RCSfile: _ChartAxisXSupplier.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change:$Date: 2003-01-27 18:08:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

