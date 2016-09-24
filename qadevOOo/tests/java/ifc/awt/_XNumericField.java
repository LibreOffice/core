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

package ifc.awt;


import lib.MultiMethodTest;

import com.sun.star.awt.XNumericField;
import util.utils;

/**
* Testing <code>com.sun.star.awt.XNumericField</code>
* interface methods :
* <ul>
*  <li><code> setValue()</code></li>
*  <li><code> getValue()</code></li>
*  <li><code> setMin()</code></li>
*  <li><code> getMin()</code></li>
*  <li><code> setMax()</code></li>
*  <li><code> getMax()</code></li>
*  <li><code> setFirst()</code></li>
*  <li><code> getFirst()</code></li>
*  <li><code> setLast()</code></li>
*  <li><code> getLast()</code></li>
*  <li><code> setSpinSize()</code></li>
*  <li><code> getSpinSize()</code></li>
*  <li><code> setDecimalDigits()</code></li>
*  <li><code> getDecimalDigits()</code></li>
*  <li><code> setStrictFormat()</code></li>
*  <li><code> isStrictFormat()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.awt.XNumericField
*/
public class _XNumericField extends MultiMethodTest {

    public XNumericField oObj = null;
    private double val = 0 ;
    private double min = 0 ;
    private double max = 0 ;
    private double first = 0 ;
    private double last = 0 ;
    private double spin = 0 ;
    private short digits = 0 ;
    private boolean strict = true ;

    /**
    * Sets value changed and then compares it to get value. <p>
    * Has <b>OK</b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getValue </code>  </li>
    * </ul>
    */
    public void _setValue() {
        requiredMethod("getValue()");

        double value = val + 1.1;
        oObj.setValue(value) ;

        tRes.tested("setValue()", utils.approxEqual(oObj.getValue(), value));
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getValue() {
        val = oObj.getValue() ;

        tRes.tested("getValue()", true) ;
    }

    /**
    * Sets minimal value changed and then compares it to get value. <p>
    * Has <b>OK</b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getMin </code>  </li>
    * </ul>
    */
    public void _setMin() {
        requiredMethod("getMin()") ;

        double value = min + 1.1;
        oObj.setMin(value);
        double ret = oObj.getMin();
        boolean result = utils.approxEqual(ret, value);

        tRes.tested("setMin()", result);
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getMin() {

        boolean result = true ;
        min = oObj.getMin() ;

        tRes.tested("getMin()", result) ;
    }

    /**
    * Sets maximal value changed and then compares it to get value. <p>
    * Has <b>OK</b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getMax </code>  </li>
    * </ul>
    */
    public void _setMax() {
        requiredMethod("getMax()") ;

        double value = max + 1.1;
        oObj.setMax(value);
        double ret = oObj.getMax();
        boolean result = utils.approxEqual(ret, value);

        tRes.tested("setMax()", result) ;
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getMax() {

        boolean result = true ;
        max = oObj.getMax() ;

        tRes.tested("getMax()", result) ;
    }

    /**
    * Sets value changed and then compares it to get value. <p>
    * Has <b>OK</b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getFirst </code>  </li>
    * </ul>
    */
    public void _setFirst() {
        requiredMethod("getFirst()") ;

        double value = first + 1.1;
        oObj.setFirst(value);
        double ret = oObj.getFirst();
        boolean result = utils.approxEqual(ret, value);

        tRes.tested("setFirst()", result) ;
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getFirst() {

        boolean result = true ;
        first = oObj.getFirst() ;

        tRes.tested("getFirst()", result) ;
    }

    /**
    * Sets value changed and then compares it to get value. <p>
    * Has <b>OK</b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getLast </code>  </li>
    * </ul>
    */
    public void _setLast() {
        requiredMethod("getLast()") ;

        double value = last + 1.1;
        oObj.setLast(value);
        double ret = oObj.getLast();
        boolean result = utils.approxEqual(ret, value);

        tRes.tested("setLast()", result) ;
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getLast() {

        boolean result = true ;
        last = oObj.getLast() ;

        tRes.tested("getLast()", result) ;
    }

    /**
    * Sets value changed and then compares it to get value. <p>
    * Has <b>OK</b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getSpinSize </code>  </li>
    * </ul>
    */
    public void _setSpinSize() {
        requiredMethod("getSpinSize()") ;

        boolean result = true ;
        double value = spin + 1.1;
        oObj.setSpinSize(value) ;
        result = utils.approxEqual(oObj.getSpinSize(), value);

        tRes.tested("setSpinSize()", result) ;
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getSpinSize() {

        boolean result = true ;
        spin = oObj.getSpinSize() ;

        tRes.tested("getSpinSize()", result) ;
    }

    /**
    * Sets value changed and then compares it to get value. <p>
    * Has <b>OK</b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getDecimalDigits </code>  </li>
    * </ul>
    */
    public void _setDecimalDigits() {
        requiredMethod("getDecimalDigits()") ;

        boolean result = true ;
        oObj.setDecimalDigits((short)(digits + 1)) ;

        short res = oObj.getDecimalDigits() ;
        result = res == (digits + 1) ;

        tRes.tested("setDecimalDigits()", result) ;
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getDecimalDigits() {

        boolean result = true ;
        digits = oObj.getDecimalDigits() ;

        tRes.tested("getDecimalDigits()", result) ;
    }

    /**
    * Sets value changed and then compares it to get value. <p>
    * Has <b>OK</b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> isStrictFormat </code>  </li>
    * </ul>
    */
    public void _setStrictFormat() {
        requiredMethod("isStrictFormat()") ;

        boolean result = true ;
        oObj.setStrictFormat(!strict) ;
        result = oObj.isStrictFormat() == !strict ;

        tRes.tested("setStrictFormat()", result) ;
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _isStrictFormat() {

        boolean result = true ;
        strict = oObj.isStrictFormat() ;

        tRes.tested("isStrictFormat()", result) ;
    }
}


