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
            oObj.setPropertyValue("AutoOrigin", Boolean.TRUE);
            oObj.setPropertyValue("AutoMax",Boolean.FALSE);
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
            oObj.setPropertyValue("AutoOrigin", Boolean.TRUE);
            oObj.setPropertyValue("AutoMin", Boolean.FALSE);
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
            oObj.setPropertyValue("Logarithmic",Boolean.FALSE);
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
        @Override
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
            oObj.setPropertyValue("AutoStepMain", Boolean.FALSE);
            oObj.setPropertyValue("Logarithmic",Boolean.FALSE);
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

    public void _StepHelp() {
        try {
            oObj.setPropertyValue("AutoStepMain", Boolean.FALSE);
            oObj.setPropertyValue("Logarithmic",Boolean.FALSE);
            oObj.setPropertyValue("StepMain", 100.0);
            oObj.setPropertyValue("StepHelpCount", 10);
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Couldn't set property value", e);
        }
        testProperty("StepHelp");
    }
}  // finish class _ChartAxis

