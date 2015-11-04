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

package ifc.table;

import lib.MultiMethodTest;
import util.utils;

import com.sun.star.table.CellContentType;
import com.sun.star.table.XCell;


/**
* Testing <code>com.sun.star.table.XCell</code>
* interface methods :
* <ul>
*  <li><code> getFormula()</code></li>
*  <li><code> setFormula()</code></li>
*  <li><code> getValue()</code></li>
*  <li><code> setValue()</code></li>
*  <li><code> getType()</code></li>
*  <li><code> getError()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.table.XCell
*/
public class _XCell extends MultiMethodTest {
  public XCell oObj = null;

    /**
    * First time errors checked when a proper formula is entered.
    * Second time an incorrect formula entered and errors are checked.<p>
    * Has <b> OK </b> status if in the first case error code 0 returned,
    * and in the second case none-zerro code returned. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setFormula() </code> : the method must set proper
    *   formula into cell, so there must be no errors </li>
    * </ul>
    */
    public void _getError() {
        requiredMethod("setFormula()") ;

        boolean result = true;

        if (oObj.getError() != 0) {
            result = false ;
            log.println("getError(): Expected error code is 0, but returned " +
                 oObj.getError()) ;
        }
        oObj.setFormula("=sqrt(-2)") ; // incorrect formula
        if (oObj.getError() == 0) {
            result = false ;
            log.println("getError(): # Non zero error code expected,"+
                " but 0 returned") ;
        }

        tRes.tested("getError()", result);
    } // end getError()

    /**
    * Sets a formula and then gets it. <p>
    * Has <b> OK </b> status if the formula set are the same as get. <p>
    */
    public void _getFormula() {
        boolean result = true;

        String formula = "";
        log.println("getFormula()");
        oObj.setFormula("=2+2");

        formula = oObj.getFormula();

        result &= formula.endsWith("2+2");
        tRes.tested("getFormula()", result);
    } // end getFormula()

    /**
    * Gets the type and check it. <p>
    * Has <b> OK </b> status if the type is one of valid values. <p>
    */
    public void _getType() {
        boolean result = true;
        result = true ;
        log.println("getType() ...");

        if(oObj.getType() == CellContentType.EMPTY) result &= true ;
        else if (oObj.getType() == CellContentType.VALUE) result &= true ;
        else if (oObj.getType() == CellContentType.TEXT) result &= true ;
        else if (oObj.getType() == CellContentType.FORMULA) result &= true ;
        else result = false;

        tRes.tested ("getType()", result) ;
    } // end getType()

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _getValue() {
        boolean result = true;
        log.println("getValue() ...");

        oObj.getValue();

        tRes.tested("getValue()",result);
    } // end getValue()

    /**
    * Sets a formula and then gets it. <p>
    * Has <b> OK </b> status if the formula set are the same as get. <p>
    */
    public void _setFormula() {
        boolean result = true;
        String formula = "";
        log.println("setFormula() ...");

        oObj.setFormula("=2/6") ;

        formula = oObj.getFormula();

        result &= formula.endsWith("2/6");
        tRes.tested ("setFormula()", result) ;
    } // end setFormula

    /**
    * Sets a value and then gets it. <p>
    * Has <b> OK </b> status if the value set is equal to value get. <p>
    */
    public void _setValue() {
        log.println("setValue() ...");

        double inValue = 222.333;
        oObj.setValue(inValue) ;
        double cellValue = oObj.getValue() ;

        boolean result = utils.approxEqual(cellValue, inValue);
        tRes.tested("setValue()", result);
    } // end setValue()
}

