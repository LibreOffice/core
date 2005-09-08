/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _ChartTwoAxisXSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:11:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
* Testing <code>com.sun.star.chart.ChartTwoAxisXSupplier</code>
* service properties :
* <ul>
*  <li><code> HasSecondaryXAxis</code></li>
*  <li><code> HasSecondaryXAxisDescription</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'CHARTDOC'</code> (of type <code>XChartDocument</code>):
*  to have reference to chart document </li>
*  <li> <code>'BAR'</code> (of type <code>XDiagram</code>):
*  relation that use as parameter for method setDiagram of chart document </li>
* </ul> <p>
* @see com.sun.star.chart.ChartTwoAxisXSupplier
*/
public class _ChartTwoAxisXSupplier extends MultiPropertyTest {

    XChartDocument doc = null;
    XDiagram oldDiagram = null;

    /**
    * Retrieves object relations and prepares a chart document.
    * @throws StatusException if one of relations not found.
    */
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
        log.println("Set it to 3D");
        oObj = (XPropertySet)
            UnoRuntime.queryInterface( XPropertySet.class, doc.getDiagram() );
        try {
            oObj.setPropertyValue("Dim3D", new Boolean(true));
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
    protected void after() {
        doc.setDiagram(oldDiagram);
    }
} // EOF ChartTwoAxisXSupplier

