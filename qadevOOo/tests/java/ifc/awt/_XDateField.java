/*************************************************************************
 *
 *  $RCSfile: _XDateField.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 10:11:00 $
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
    * Has <b> OK </b> status if no runtime exceptions occured
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
    * Has <b> OK </b> status if no runtime exceptions occured
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
    * Has <b> OK </b> status if no runtime exceptions occured
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
    * Has <b> OK </b> status if no runtime exceptions occured
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
    * Has <b> OK </b> status if no runtime exceptions occured
    */
    public void _getLast() {

        boolean result = true ;
        int val = oObj.getLast() ;

        log.println("getLast() = " + val) ;

        tRes.tested("getLast()", result) ;
    }

    /**
    * Sets the value to empty. <p>
    * Has <b> OK </b> status if no runtime exceptions occured
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
    * Has <b> OK </b> status if no runtime exceptions occured.
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
    * Has <b> OK </b> status if no runtime exceptions occured.
    */
    public void _isLongFormat() {

        boolean result = true ;
        longFormat = oObj.isLongFormat() ;

        tRes.tested("isLongFormat()", result) ;
    }
}


