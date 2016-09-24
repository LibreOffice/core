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

package ifc.sheet;

import lib.MultiMethodTest;
import util.ValueChanger;

import com.sun.star.sheet.ConditionOperator;
import com.sun.star.sheet.XSheetCondition;
import com.sun.star.table.CellAddress;

/**
* Testing <code>com.sun.star.sheet.XSheetCondition</code>
* interface methods :
* <ul>
*  <li><code> getOperator()</code></li>
*  <li><code> setOperator()</code></li>
*  <li><code> getFormula1()</code></li>
*  <li><code> setFormula1()</code></li>
*  <li><code> getFormula2()</code></li>
*  <li><code> setFormula2()</code></li>
*  <li><code> getSourcePosition()</code></li>
*  <li><code> setSourcePosition()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetCondition
*/
public class _XSheetCondition extends MultiMethodTest {
    public XSheetCondition oObj = null;
    public String Formula1 = null;
    public String Formula2 = null;
    public ConditionOperator Operator = null;
    public CellAddress SourcePosition = null;

    /**
    * Test calls the method, checks and stores returned value. <p>
    * Has <b> OK </b> status if returned vakue isn't null. <p>
    */
    public void _getFormula1() {
        Formula1 = oObj.getFormula1();
        tRes.tested("getFormula1()", Formula1 != null);
    }

    /**
    * Test calls the method, checks and stores returned value. <p>
    * Has <b> OK </b> status if returned vakue isn't null. <p>
    */
    public void _getFormula2() {
        Formula2 = oObj.getFormula2();
        tRes.tested("getFormula2()", Formula2 != null);
    }

    /**
    * Test calls the method, checks and stores returned value. <p>
    * Has <b> OK </b> status if returned vakue isn't null. <p>
    */
    public void _getOperator() {
        Operator = oObj.getOperator();
        tRes.tested("getOperator()", Operator != null);
    }

    /**
    * Test calls the method, checks and stores returned value. <p>
    * Has <b> OK </b> status if returned vakue isn't null. <p>
    */
    public void _getSourcePosition() {
        SourcePosition = oObj.getSourcePosition();
        tRes.tested("getSourcePosition()", SourcePosition != null);
    }

    /**
    * Test sets new value of formula1, gets formula1 again and compares
    * returned value with value that was stored by method
    * <code>getFormula1()</code>. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getFormula1() </code> : to have value of 'Formula1' </li>
    * </ul>
    */
    public void _setFormula1() {
        requiredMethod("getFormula1()");
        oObj.setFormula1("$Sheet1.$C$" + Thread.activeCount());
        tRes.tested("setFormula1()", !Formula1.equals( oObj.getFormula1() ) );
    }

    /**
    * Test sets new value of formula2, gets formula2 again and compares
    * returned value with value that was stored by method
    * <code>getFormula2()</code>. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getFormula2() </code> : to have value of 'Formula2' </li>
    * </ul>
    */
    public void _setFormula2() {
        requiredMethod("getFormula2()");
        oObj.setFormula2("$Sheet1.$A$" + Thread.activeCount());
        tRes.tested("setFormula2()", !Formula2.equals( oObj.getFormula2() ) );
    }

    /**
    * Test sets new value of operator, gets operator and compares
    * returned value with value that was set. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    */
    public void _setOperator() {
        oObj.setOperator(ConditionOperator.BETWEEN);
        tRes.tested("setOperator()", !Operator.equals( oObj.getOperator() ) );
    }

    /**
    * Test change value that was stored by method
    * <code>getSourcePosition()</code>, sets this new value, gets source
    * position again and compares returned value with value that was set. <p>
    * Has <b> OK </b> status if values aren't equal. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getSourcePosition() </code> : to have value of source
    *  position </li>
    * </ul>
    */
    public void _setSourcePosition() {
        requiredMethod("getSourcePosition()");
        oObj.setSourcePosition(
                    (CellAddress)ValueChanger.changePValue(SourcePosition));
        tRes.tested(
            "setSourcePosition()",
            !SourcePosition.equals( oObj.getSourcePosition() ) );
    }

}  // finish class _XSheetCondition

