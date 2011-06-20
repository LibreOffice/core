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

import com.sun.star.awt.XPatternField;

/**
* Testing <code>com.sun.star.awt.XPatternField</code>
* interface methods :
* <ul>
*  <li><code> setMasks()</code></li>
*  <li><code> getMasks()</code></li>
*  <li><code> setString()</code></li>
*  <li><code> getString()</code></li>
*  <li><code> setStrictFormat()</code></li>
*  <li><code> isStrictFormat()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.awt.XPatternField
*/
public class _XPatternField extends MultiMethodTest {

    public XPatternField oObj = null ;
    private String editMask = null ;
    private String literalMask = null ;
    private String string = null ;
    private boolean strict = false ;

    /**
    * Sets masks to new values then gets them and compare. <p>
    * Has <b> OK </b> status if set and get masks are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getMasks </code> </li>
    * </ul>
    */
    public void _setMasks() {
        requiredMethod("getMasks()") ;

        boolean result = true ;
        String newEdit = editMask == null ? "ccc" : editMask + "ccc" ;
        String newLiteral = literalMask == null ? " " : literalMask + " " ;
        oObj.setMasks(newEdit, newLiteral) ;

        String[] edit = new String[1] ;
        String[] literal = new String[1] ;
        oObj.getMasks(edit, literal) ;

        result &= newEdit.equals(edit[0]) ;
        result &= newLiteral.equals(literal[0]) ;

        tRes.tested("setMasks()", result) ;
    }

    /**
    * Gets masks and stores them. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred.
    */
    public void _getMasks() {

        boolean result = true ;
        String[] edit = new String[1] ;
        String[] literal = new String[1] ;
        oObj.getMasks(edit, literal) ;

        log.println("Edit mask = '" + edit[0] + "', literal = '" +
            literal[0] + "'") ;

        editMask = edit[0] ;
        literalMask = literal[0] ;

        tRes.tested("getMasks()", result) ;
    }

    /**
    * Sets new string and then get it for verification. <p>
    * Has <b> OK </b> status if get and set strings are equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getString </code> </li>
    *  <li> <code> setMasks </code> : mask must be set for new string
    *   would be valid. </li>
    * </ul>
    */
    public void _setString() {
        requiredMethod("setMasks()") ;
        requiredMethod("getString()") ;

        boolean result = true ;
        String newString = string = "abc" ;
        oObj.setString(newString) ;
        String getString = oObj.getString() ;

        result = newString.equals(getString) ;

        if (!result) {
            log.println("Was '" + string + "', Set '" + newString
                + "', Get '" + getString + "'") ;
        }

        tRes.tested("setString()", result) ;
    }

    /**
    * Gets current string and stores it. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _getString() {

        boolean result = true ;
        string = oObj.getString() ;

        tRes.tested("getString()", result) ;
    }

    /**
    * Sets new strict state then checks it. <p>
    * Has <b> OK </b> status if the state was changed.
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
    * Gets the current strict state and stores it. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred.
    */
    public void _isStrictFormat() {

        boolean result = true ;
        strict = oObj.isStrictFormat() ;

        tRes.tested("isStrictFormat()", result) ;
    }
}


