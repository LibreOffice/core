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

