/*************************************************************************
 *
 *  $RCSfile: _XPatternField.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:12:23 $
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
    * Has <b> OK </b> status if no runtime exceptions occured.
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
    * Has <b> OK </b> status if no runtime exceptions occured
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
    * Has <b> OK </b> status if no runtime exceptions occured.
    */
    public void _isStrictFormat() {

        boolean result = true ;
        strict = oObj.isStrictFormat() ;

        tRes.tested("isStrictFormat()", result) ;
    }
}


