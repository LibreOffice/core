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

import com.sun.star.awt.XTimeField;
import com.sun.star.util.Time;

/**
* Testing <code>com.sun.star.awt.XTimeField</code>
* interface methods :
* <ul>
*  <li><code> setTime()</code></li>
*  <li><code> getTime()</code></li>
*  <li><code> setMin()</code></li>
*  <li><code> getMin()</code></li>
*  <li><code> setMax()</code></li>
*  <li><code> getMax()</code></li>
*  <li><code> setFirst()</code></li>
*  <li><code> getFirst()</code></li>
*  <li><code> setLast()</code></li>
*  <li><code> getLast()</code></li>
*  <li><code> setEmpty()</code></li>
*  <li><code> isEmpty()</code></li>
*  <li><code> setStrictFormat()</code></li>
*  <li><code> isStrictFormat()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XTimeField
*/
public class _XTimeField extends MultiMethodTest {

    public XTimeField oObj = null;
    private boolean strict = false ;

    /**
    * Sets a new value and checks if it was correctly set. <p>
    * Has <b> OK </b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getTime </code> </li>
    * </ul>
    */
    public void _setTime() {
        requiredMethod("getTime()") ;

        boolean result = true ;
        oObj.setTime(new Time(0, (short)0, (short)15, (short)11, false));

        result = oObj.getTime() == new Time(0, (short)0, (short)15, (short)11, false) ;

        if (! result ) {
            System.out.println("Getting " + oObj.getTime() + " expected 11:15");
        }

        tRes.tested("setTime()", result) ;
    }

    /**
    * Gets the current value. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _getTime() {

        boolean result = true ;
        oObj.getTime() ;

        tRes.tested("getTime()", result) ;
    }

    /**
    * Sets a new value and checks if it was correctly set. <p>
    * Has <b> OK </b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getMin </code> </li>
    * </ul>
    */
    public void _setMin() {

        boolean result = true ;
        oObj.setMin(new Time(0, (short)14, (short)18, (short)6, false));

        result = oObj.getMin() == new Time(0, (short)14, (short)18, (short)6, false) ;

        tRes.tested("setMin()", result) ;
    }

    /**
    * Gets the current value. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _getMin() {

        boolean result = true ;
        oObj.getMin() ;

        tRes.tested("getMin()", result) ;
    }

    /**
    * Sets a new value and checks if it was correctly set. <p>
    * Has <b> OK </b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getMax </code> </li>
    * </ul>
    */
    public void _setMax() {

        boolean result = true ;
        oObj.setMax(new Time(855447, (short)31, (short)23, (short)22, false)) ;

        result = oObj.getMax() == new Time(855447, (short)31, (short)23, (short)22, false) ;

        tRes.tested("setMax()", result) ;
    }

    /**
    * Gets the current value. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _getMax() {

        boolean result = true ;
        oObj.getMax() ;

        tRes.tested("getMax()", result) ;
    }

    /**
    * Sets a new value and checks if it was correctly set. <p>
    * Has <b> OK </b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getFirst </code> </li>
    * </ul>
    */
    public void _setFirst() {

        boolean result = true ;
        oObj.setFirst(new Time(0, (short)0, (short)30, (short)7, false)) ;

        result = oObj.getFirst() == new Time(0, (short)0, (short)30, (short)7, false);

        if (!result) {
            log.println("Set to 07:30 but returned " + oObj.getFirst()) ;
        }

        tRes.tested("setFirst()", result) ;
    }

    /**
    * Gets the current value. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _getFirst() {

        boolean result = true ;
        com.sun.star.util.Time val = oObj.getFirst() ;

        log.println("getFirst() = " + val) ;

        tRes.tested("getFirst()", result) ;
    }

    /**
    * Sets a new value and checks if it was correctly set. <p>
    * Has <b> OK </b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getLast </code> </li>
    * </ul>
    */
    public void _setLast() {

        boolean result = true ;
        oObj.setLast(new Time(500000, (short)31, (short)30, (short)18, false)) ;

        result = oObj.getLast() == new Time(500000, (short)31, (short)30, (short)18, false);

        if (!result) {
            log.println("Set to 18:30:31.5 but returned " + oObj.getLast()) ;
        }

        tRes.tested("setLast()", result) ;
    }

    /**
    * Gets the current value. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _getLast() {

        boolean result = true ;
        com.sun.star.util.Time val = oObj.getLast() ;

        log.println("getLast() = " + val) ;

        tRes.tested("getLast()", result) ;
    }

    /**
    * Sets the value to empty. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setTime </code> : value must be not empty </li>
    * </ul>
    */
    public void _setEmpty() {
        requiredMethod("setTime()") ;

        boolean result = true ;
        oObj.setEmpty() ;

        tRes.tested("setEmpty()", result) ;
    }

    /**
    * Checks if the field is empty. <p>
    * Has <b> OK </b> status if the value is empty.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setEmpty() </code>  </li>
    * </ul>
    */
    public void _isEmpty() {
        requiredMethod("setEmpty()") ;

        boolean result = true ;
        result = oObj.isEmpty() ;

        tRes.tested("isEmpty()", result) ;
    }

    /**
    * Checks strict state. <p>
    * Has <b> OK </b> status if strict format is set.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> isStrictFormat </code> </li>
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
    * Gets strict state and stores it. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred.
    */
    public void _isStrictFormat() {

        boolean result = true ;
        strict = oObj.isStrictFormat() ;

        tRes.tested("isStrictFormat()", result) ;
    }
}


