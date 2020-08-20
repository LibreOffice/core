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

package ifc.accessibility;


import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.accessibility.XAccessibleValue;

/**
 * Testing <code>com.sun.star.accessibility.XAccessibleValue</code>
 * interface methods :
 * <ul>
 *  <li><code> getCurrentValue()</code></li>
 *  <li><code> setCurrentValue()</code></li>
 *  <li><code> getMaximumValue()</code></li>
 *  <li><code> getMinimumValue()</code></li>
 * </ul> <p>
 *
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'XAccessibleValue.anotherFromGroup'</code>
 *  (of type <code>XAccessibleValue</code>) <b> optional </b>:
 *   another component from the group(e.g. radio button group)</li>
 *  </ul><p>
 * @see com.sun.star.accessibility.XAccessibleValue
 */
public class _XAccessibleValue extends MultiMethodTest {

    public XAccessibleValue oObj = null;

    private double minVal = 0;
    private double maxVal = 0;
    private static final double curVal = 0;
    private Object val = null;
    XAccessibleValue anotherFromGroup = null;

    @Override
    protected void before() {
        anotherFromGroup = (XAccessibleValue)tEnv.getObjRelation(
            "XAccessibleValue.anotherFromGroup");
    }

    /**
     * Gets current value and stores it as double. <p>
     *
     * Has <b> OK </b> status if the current value is between Min and Max
     * values. <p>
     *
     * The following method tests are to be executed before :
     * <ul>
     *  <li> <code> getMaximumValue </code> </li>
     *  <li> <code> getMinimumValue </code> </li>
     * </ul>
     */
    public void _getCurrentValue() {
        executeMethod("getMaximumValue()");
        executeMethod("getMinimumValue()");

        boolean result = true;

        double curVal ;
        val = oObj.getCurrentValue() ;
        if (util.utils.isVoid(val)) {
            val = Integer.valueOf(0);
            curVal = 0;
        } else  {
            curVal = getDoubleValue(val);
        }

        if (curVal < minVal || maxVal < curVal) {
            log.println((curVal - minVal) + "," + (maxVal - curVal));
            log.println("Current value " + curVal + " is not in range ["
                + minVal + "," + maxVal + "]");
            result = false;
        }

        tRes.tested("getCurrentValue()", result) ;
    }

    /**
     * Performs testing for following cases :
     * <ul>
     *  <li> Creates new value in valid range and sets it. </li>
     *  <li> Sets maximum and minimum values. </li>
     *  <li> Sets Min - 1, and Max + 1 values </li>
     * </ul> <p>
     *
     * Has <b> OK </b> status if for the first case the value returned
     * is the same as was set and the method <code>setCurrentValue</code>
     * returns <code>true</code>.
     *
     * In the second if Max and Min values are set and method
     * <code>setCurrentValue</code> returns <code>true</code>.
     *
     * In the third case invalid values are truncated to Min and Max
     * values accordingly. <p>
     *
     * The following method tests are to be executed before :
     * <ul>
     *  <li> <code> getCurrentValue() </code> </li>
     * </ul>
     */
    public void _setCurrentValue() {
        executeMethod("getCurrentValue()");

        boolean result = true ;
        boolean partResult=true;
        String noMax = "com.sun.star.comp.toolkit.AccessibleScrollBar";
        String implName = util.utils.getImplName(oObj);

        if (tEnv.getObjRelation("ValueNotPersistent")!=null) {
            log.println("Excluded since it works like AccessibleAction");
            tRes.tested("setCurrentValue()",Status.skipped(true));
            return;
        }

        if (anotherFromGroup == null) {
            double newVal = curVal + 1;
            if (newVal > maxVal) newVal -= 2;
            if (newVal < minVal) newVal += 1;

            log.println("New value is " + newVal);

            Object setVal = getObjectValue(newVal, val.getClass());

            result &= oObj.setCurrentValue(setVal);

            if (!result) {
                log.println("The value can't be set");
                throw new StatusException(Status.skipped(true));
            }

            double resVal = getDoubleValue(oObj.getCurrentValue());
            log.println("Res value is " + resVal);

            result &= Math.abs(newVal - resVal) < 0.00001;

            log.println("Checking min/max values");
            result &= oObj.setCurrentValue(getObjectValue(minVal, val.getClass()));
            log.println("Setting to "+ getObjectValue(minVal, val.getClass()));
            resVal = getDoubleValue(oObj.getCurrentValue());
            log.println("Result min value is " + resVal);
            result &= Math.abs(minVal - resVal) < 0.00001;
            log.println("\t works: "+(Math.abs(minVal - resVal) < 0.00001));

            result &= oObj.setCurrentValue(getObjectValue(maxVal, val.getClass()));
            log.println("Setting to "+ getObjectValue(maxVal, val.getClass()));
            resVal = getDoubleValue(oObj.getCurrentValue());
            log.println("Result max value is " + resVal);
            partResult = Math.abs(maxVal - resVal) < 0.00001;

            if (implName.equals(noMax)) {
                log.println("If one sets the maximum value of a scroll bar with XScrollBar::setMaximum(),"+
                "then XScrollBar::getValue() returns the maximum value minus the visible size of"+
                "the thumb");
                //using arbitrary Value, since we can't determine the resulting value
                partResult = resVal > 10;
            }

            result &=partResult;
            log.println("\t works: "+partResult);

            log.println("Checking truncating of min/max values");
            oObj.setCurrentValue(getObjectValue(minVal - 1, val.getClass()));
            log.println("Setting to "+ getObjectValue(minVal -1 , val.getClass()));
            resVal = getDoubleValue(oObj.getCurrentValue());
            log.println("Result min value is " + resVal);
            result &= Math.abs(minVal - resVal) < 0.00001;
            log.println("\t works: "+(Math.abs(minVal - resVal) < 0.00001));

            oObj.setCurrentValue(getObjectValue(maxVal + 1, val.getClass()));
            log.println("Setting to "+ getObjectValue(maxVal +1 , val.getClass()));
            resVal = getDoubleValue(oObj.getCurrentValue());
            log.println("Result max value is " + resVal);
            partResult = Math.abs(maxVal - resVal) < 0.00001;
            if (implName.equals(noMax)) {
                log.println("If one sets the maximum value of a scroll bar with XScrollBar::setMaximum(),"+
                "then XScrollBar::getValue() returns the maximum value minus the visible size of"+
                "the thumb");
                //using arbitrary Value, since we can't determine the resulting value
                partResult = resVal > 10;
            }

            result &=partResult;
            log.println("\t works: "+partResult);
        } else {
            int curValBase = getIntegerValue(val);
            Object valAnotherFromGroup = anotherFromGroup.getCurrentValue();
            int curValAnother = getIntegerValue(valAnotherFromGroup);
            log.println("Current value of base component: " + curValBase);
            log.println("Current value of another component from group: " +
                curValAnother);
            log.println("Set value of base component to " + curValAnother);
            if (tEnv.getTestCase().getObjectName().equals("AccessibleRadioButton")) {
                anotherFromGroup.setCurrentValue(Integer.valueOf(curValBase));
            } else {
                oObj.setCurrentValue(valAnotherFromGroup);
            }
            log.println("Checking of values...");
            int newValBase = getIntegerValue(oObj.getCurrentValue());
            int newValAnother = getIntegerValue(
                anotherFromGroup.getCurrentValue());
            log.println("New value of base component: " + newValBase);
            log.println("Expected value of base component: " + curValAnother);
            log.println("New value of another component from group: " +
                newValAnother);
            log.println("Expected value of another component from group: " +
                curValBase);

            result = (newValBase == curValAnother) &&
                (newValAnother == curValBase);
        }

        tRes.tested("setCurrentValue()", result);
    }

    /**
     * Gets and stores maximal value. <p>
     *
     * Has <b> OK </b> status if non empty value returned and
     * Max value is greater than Min value.
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getMinimumValue() </code> : to compare with </li>
     * </ul>
     */
    public void _getMaximumValue() {
        requiredMethod("getMinimumValue()");

        boolean result = true ;

        Object val = oObj.getMaximumValue();
        if (util.utils.isVoid(val)) {
            maxVal = Double.MAX_VALUE ;
            result = false;
        } else {
            maxVal = getDoubleValue(val);
        }
        log.println("Max is " + val.getClass()+ " = " + maxVal);

        result &= maxVal >= minVal;

        tRes.tested("getMaximumValue()", result) ;
    }

    /**
     * Gets and stores minimal value. <p>
     *
     * Has <b> OK </b> status if non empty value returned. <p>
     *
     */
    public void _getMinimumValue() {
        boolean result = true ;

        Object val = oObj.getMinimumValue() ;
        if (util.utils.isVoid(val)) {
            minVal = - Double.MAX_VALUE ;
            result = false;
        } else {
            minVal = getDoubleValue(val);
        }
        log.println("Min is " + val.getClass()+ " = " + minVal);

        tRes.tested("getMinimumValue()", result) ;
    }

    private int getIntegerValue(Object val) {
        if (val instanceof Integer) {
            return ((Integer) val).intValue();
        } else {
            throw new StatusException
                (Status.failed("Unexpected value type: " + val.getClass()));
        }
    }

    private double getDoubleValue(Object val) {
        if (val instanceof Integer) {
            return ((Integer) val).doubleValue();
        }
        else if (val instanceof Short) {
            return ((Short) val).doubleValue();
        }
        else if (val instanceof Float) {
            return ((Float) val).doubleValue();
        }
        else if (val instanceof Double) {
            return ((Double) val).doubleValue();
        }
        else if (util.utils.isVoid(val)) {
            return Double.NaN;
        }
        else {
            throw new StatusException
                (Status.failed("Undetected value type: " + val.getClass()));
        }
    }

    private Object getObjectValue(double val, Class<?> clazz) {
        if (clazz.equals(Integer.class)) {
            return Integer.valueOf((int)val);
        }
        else if (clazz.equals(Short.class)) {
            return Short.valueOf((short)val);
        }
        else if (clazz.equals(Float.class)) {
            return new Float((float)val);
        }
        else if (clazz.equals(Double.class)) {
            return new Double(val);
        }
        else {
            throw new StatusException
                (Status.failed("Unexpected class: " + clazz));
        }
    }

    /**
     * Disposes test environment.
     */
    @Override
    protected void after() {
        disposeEnvironment();
    }
}
