/*************************************************************************
 *
 *  $RCSfile: _XSheetCondition.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:04:23 $
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

