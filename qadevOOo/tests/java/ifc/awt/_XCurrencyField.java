/*************************************************************************
 *
 *  $RCSfile: _XCurrencyField.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:10:41 $
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
    * Has <b>OK</b> status if no runtime exceptions occured.
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
    * Has <b>OK</b> status if no runtime exceptions occured.
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
    * Has <b>OK</b> status if no runtime exceptions occured.
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
    * Has <b>OK</b> status if no runtime exceptions occured.
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
    * Has <b>OK</b> status if no runtime exceptions occured.
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
    * Has <b>OK</b> status if no runtime exceptions occured.
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
    * Has <b>OK</b> status if no runtime exceptions occured.
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
    * Has <b>OK</b> status if no runtime exceptions occured.
    */
    public void _isStrictFormat() {

        boolean result = true;
        strict = oObj.isStrictFormat();

        tRes.tested("isStrictFormat()", result);
    }
}
