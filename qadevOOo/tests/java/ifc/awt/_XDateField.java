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

import com.sun.star.awt.XDateField;
import com.sun.star.util.Date;

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
        oObj.setDate(new Date((short)1, (short)1, (short)1900)) ;

        result = oObj.getDate() == new Date((short)1, (short)1, (short)1900); ;

        if (! result ) {
            System.out.println("getDate: " + oObj.getDate() + " , expected 1900-01-01");
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
        oObj.setMin(new Date((short)5, (short)2, (short)1963));

        result = oObj.getMin() == new Date((short)5, (short)2, (short)1963);

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
        oObj.setMax(new Date((short)18, (short)9, (short)2117)) ;

        result = oObj.getMax() == new Date((short)18, (short)9, (short)2117) ;

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
        oObj.setFirst(new Date((short)7, (short)12, (short)1972)) ;

        result = oObj.getFirst() == new Date((short)7, (short)12, (short)1972) ;

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
        com.sun.star.util.Date val = oObj.getFirst() ;

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
        oObj.setLast(new Date((short)23, (short)8, (short)2053)) ;

        result = oObj.getLast() == new Date((short)23, (short)8, (short)2053) ;

        if (!result) {
            log.println("Set to 2053-08-23 but returned " + oObj.getLast()) ;
        }

        tRes.tested("setLast()", result) ;
    }

    /**
    * Gets the current value. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _getLast() {

        boolean result = true ;
        com.sun.star.util.Date val = oObj.getLast() ;

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


