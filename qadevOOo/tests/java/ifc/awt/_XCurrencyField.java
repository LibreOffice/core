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

package ifc.awt;

import lib.MultiMethodTest;

import com.sun.star.awt.XCurrencyField;

/**
* Testing <code>com.sun.star.awt.XCurrencyField</code>
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
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XCurrencyField
*/
public class _XCurrencyField extends MultiMethodTest {

    public XCurrencyField oObj = null;
    private double val = 0;
    private double min = 0;
    private double max = 0;
    private double first = 0;
    private double last = 0;
    private double spin = 0;
    private short digits = 0;
    private boolean strict = true;

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

        oObj.setValue(val + 1.1);

        tRes.tested("setValue()", oObj.getValue() == val + 1.1);
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getValue() {
        val = oObj.getValue();

        tRes.tested("getValue()", true);
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
        requiredMethod("getMin()");

        oObj.setMin(min + 1.1);

        tRes.tested("setMin()", oObj.getMin() == min + 1.1);
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getMin() {

        boolean result = true;
        min = oObj.getMin();

        tRes.tested("getMin()", result);
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
        requiredMethod("getMax()");

        boolean result = true;
        oObj.setMax(max + 1.1);
        result = oObj.getMax() == max + 1.1;

        tRes.tested("setMax()", result);
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getMax() {

        boolean result = true;
        max = oObj.getMax();

        tRes.tested("getMax()", result);
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
        requiredMethod("getFirst()");

        boolean result = true;
        oObj.setFirst(first + 1.1);
        double ret = oObj.getFirst();
        result = ret == first + 1.1;

        tRes.tested("setFirst()", result);
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getFirst() {

        boolean result = true;
        first = oObj.getFirst();

        tRes.tested("getFirst()", result);
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
        requiredMethod("getLast()");

        boolean result = true;
        oObj.setLast(last + 1.1);
        double ret = oObj.getLast();

        result = ret == last + 1.1;

        tRes.tested("setLast()", result);
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getLast() {

        boolean result = true;
        last = oObj.getLast();

        tRes.tested("getLast()", result);
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
        requiredMethod("getSpinSize()");

        boolean result = true;
        oObj.setSpinSize(spin + 1.1);
        result = oObj.getSpinSize() == spin + 1.1;

        tRes.tested("setSpinSize()", result);
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getSpinSize() {

        boolean result = true;
        spin = oObj.getSpinSize();

        tRes.tested("getSpinSize()", result);
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
        requiredMethod("getDecimalDigits()");

        boolean result = true;
        oObj.setDecimalDigits((short) (digits + 1));

        short res = oObj.getDecimalDigits();
        result = res == ((short) digits + 1);

        tRes.tested("setDecimalDigits()", result);
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getDecimalDigits() {

        boolean result = true;
        digits = oObj.getDecimalDigits();

        tRes.tested("getDecimalDigits()", result);
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
        requiredMethod("isStrictFormat()");

        boolean result = true;
        oObj.setStrictFormat(!strict);
        result = oObj.isStrictFormat() == !strict;

        tRes.tested("setStrictFormat()", result);
    }

    /**
    * Just calls the method and stores value returned. <p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _isStrictFormat() {

        boolean result = true;
        strict = oObj.isStrictFormat();

        tRes.tested("isStrictFormat()", result);
    }
}
