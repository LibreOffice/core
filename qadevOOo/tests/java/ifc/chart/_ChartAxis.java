/*************************************************************************
 *
 *  $RCSfile: _ChartAxis.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:17:04 $
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
import lib.StatusException;
import util.ValueChanger;

/**
* Testing <code>com.sun.star.chart.ChartAxis</code>
* service properties:
* <ul>
*  <li><code> ArrangeOrder</code></li>
*  <li><code> AutoMax</code></li>
*  <li><code> AutoMin</code></li>
*  <li><code> AutoOrigin</code></li>
*  <li><code> AutoStepHelp</code></li>
*  <li><code> AutoStepMain</code></li>
*  <li><code> DisplayLabels</code></li>
*  <li><code> GapWidth</code></li>
*  <li><code> HelpMarks</code></li>
*  <li><code> Logarithmic</code></li>
*  <li><code> Marks</code></li>
*  <li><code> Max</code></li>
*  <li><code> Min</code></li>
*  <li><code> NumberFormat</code></li>
*  <li><code> Origin</code></li>
*  <li><code> Overlap</code></li>
*  <li><code> StepHelp</code></li>
*  <li><code> StepMain</code></li>
*  <li><code> TextBreak</code></li>
*  <li><code> TextRotation</code></li>
*  <li><code> TextCanOverlap</code></li>
* </ul> <p>
* @see com.sun.star.chart.ChartAxis
*/
public class _ChartAxis extends MultiPropertyTest {

    /**
    * Tests property 'Max'.
    * Property 'AutoOrigin' sets to true and property 'AutoMax'
    * sets to false before test.
    */
    public void _Max() {
        try {
            //if AutoOrigin isn't true then this property works only when
            //current Origin is less then new value of Max
            oObj.setPropertyValue("AutoOrigin", new Boolean(true));
            oObj.setPropertyValue("AutoMax",new Boolean(false));
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        }

        testProperty("Max");
    }

    /**
    * Tests property 'Min'.
    * Property 'AutoOrigin' sets to true and property 'AutoMin'
    * sets to false before test.
    */
    public void _Min() {
        try {
            //if AutoOrigin isn't true then this property works only when
            //current Origin is greater then new value of Min
            oObj.setPropertyValue("AutoOrigin", new Boolean(true));
            oObj.setPropertyValue("AutoMin", new Boolean(false));
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        }

        testProperty("Min");
    }

    /**
    * Tests property 'Origin'.
    * Property 'Logarithmic' sets to false before test.
    */
    public void _Origin() {
        try {
            oObj.setPropertyValue("Logarithmic",new Boolean(false));
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        }

        testProperty("Origin");
    }

    protected PropertyTester StepMainTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            Double ValueToSet = (Double) ValueChanger.changePValue(oldValue);
            double stm = ValueToSet.doubleValue();
            stm = stm / 5;
            return new Double(stm);
        }
    };

    /**
    * Tests property 'StepMain'.
    * Property 'Logarithmic' and 'AutoStepMain' sets
    * to false before test.
    */
    public void _StepMain() {
        try {
            oObj.setPropertyValue("AutoStepMain", new Boolean(false));
            oObj.setPropertyValue("Logarithmic",new Boolean(false));
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
            throw new StatusException("Couldn't set property value", e);
        }

        testProperty("StepMain", StepMainTester);
    }
}  // finish class _ChartAxis

