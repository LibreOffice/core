/*************************************************************************
 *
 *  $RCSfile: _BarDiagram.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:16:47 $
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
        oObj = (XPropertySet)
            UnoRuntime.queryInterface( XPropertySet.class, doc.getDiagram() );
        log.println("Set it to 3D");
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

    protected PropertyTester LineTester = new PropertyTester() {
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
    * Tests property 'NumberOfLines'.
    * This property tests when diagram in 2D-mode only
    * except all other properties. This property is currently supported by
    * two dimensional vertical bar charts only.
    */
    public void _NumberOfLines() {
        log.println("Set it to 2D");
        try {
            oObj.setPropertyValue("Dim3D", new Boolean(false));
            oObj.setPropertyValue("Vertical", new Boolean(false));
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

