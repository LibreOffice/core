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

import com.sun.star.awt.XDateField;

/**
* Testing <code>com.sun.star.awt.XDateField</code>
* interface methods :
* <ul>
*  <li><code> setDate()</code></li>
*  <li><code> getDate()</code></li>
*  <li><code> setMin()</code></li>
*  <li><code> getMin()</code></li>
*  <li><code> setMax()</code></li>
*  <li><code> getMax()</code></li>
*  <li><code> setFirst()</code></li>
*  <li><code> getFirst()</code></li>
*  <li><code> setLast()</code></li>
*  <li><code> getLast()</code></li>
*  <li><code> setLongFormat()</code></li>
*  <li><code> isLongFormat()</code></li>
*  <li><code> setEmpty()</code></li>
*  <li><code> isEmpty()</code></li>
*  <li><code> setStrictFormat()</code></li>
*  <li><code> isStrictFormat()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XDateField
*/
public class _XDateField extends MultiMethodTest {

    public XDateField oObj = null;

    private boolean strict = false ;
    private boolean longFormat = false ;

    /**
    * Sets a new value and checks if it was correctly set. <p>
    * Has <b> OK </b> status if set and get values are equal.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getTime </code> </li>
    * </ul>
    */
    public void _setDate() {
        requiredMethod("getDate()") ;

        boolean result = true ;
        oObj.setDate(19000101) ;

        result = oObj.getDate() == 19000101 ;

        if (! result ) {
            System.out.println("getDate: "+oObj.getDate()+" , expected 19000101");
        }

        tRes.tested("setDate()", result) ;
    }

    /**
    * Gets the current value. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _getDate() {

        boolean result = true ;
        oObj.getDate() ;

        tRes.tested("getDate()", result) ;
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
        oObj.setMin(4978) ;

        result = oObj.getMin() == 4978 ;

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
        oObj.setMax(27856) ;

        result = oObj.getMax() == 27856 ;

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
        oObj.setFirst(5118) ;

        result = oObj.getFirst() == 5118 ;

        if (!result) {
            log.println("Set to " + 5118 + " but returned " + oObj.getFirst()) ;
        }

        tRes.tested("setFirst()", result) ;
    }

    /**
    * Gets the current value. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _getFirst() {

        boolean result = true ;
        int val = oObj.getFirst() ;

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
        oObj.setLast(23450) ;

        result = oObj.getLast() == 23450 ;

        if (!result) {
            log.println("Set to " + 23450 + " but returned " + oObj.getLast()) ;
        }

        tRes.tested("setLast()", result) ;
    }

    /**
    * Gets the current value. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _getLast() {

        boolean result = true ;
        int val = oObj.getLast() ;

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
        requiredMethod("setDate()") ;

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
    * Has <b> OK </b> status if strict format is properly set.
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

        if (!result) {
            log.println("Was '" + strict + "', set to '" + !strict +
                "' but returned '" + oObj.isStrictFormat() + "'") ;
        }

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


    /**
    * Checks long format state. <p>
    * Has <b> OK </b> status if long format is properly set.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> isLongFormat </code> </li>
    * </ul>
    */
    public void _setLongFormat() {

        boolean result = true ;
        oObj.setLongFormat(!longFormat) ;

        result = oObj.isLongFormat() == !longFormat ;

        if (!result) {
            log.println("Was '" + longFormat + "', set to '" + !longFormat +
                "' but returned '" + oObj.isLongFormat() + "'") ;
        }

        tRes.tested("setLongFormat()", result) ;
    }

    /**
    * Gets long format state and stores it. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred.
    */
    public void _isLongFormat() {

        boolean result = true ;
        longFormat = oObj.isLongFormat() ;

        tRes.tested("isLongFormat()", result) ;
    }
}


