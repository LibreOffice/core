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

