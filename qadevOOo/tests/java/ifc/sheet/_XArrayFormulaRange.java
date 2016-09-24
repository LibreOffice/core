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
import lib.Status;
import lib.StatusException;

import com.sun.star.sheet.XArrayFormulaRange;
import com.sun.star.sheet.XCellRangeAddressable;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.table.XCell;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sheet.XArrayFormulaRange</code>
* interface methods :
* <ul>
*  <li><code> getArrayFormula()</code></li>
*  <li><code> setArrayFormula()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'SHEET'</code> (of type <code>XSpreadsheet</code>):
*   to check contents of spreadsheet </li>
*  <li> <code>'noArray'</code> (of type <code>Object</code>):
*   if the relation is null then given component doesn't really support
*   this interface </li>
* <ul> <p>
* Test object must implements interface <code>XCellRangeAddressable</code> also.
* @see com.sun.star.sheet.XArrayFormulaRange
* @see com.sun.star.sheet.XSpreadsheet
* @see com.sun.star.sheet.XCellRangeAddressable
*/
public class _XArrayFormulaRange extends MultiMethodTest {

    public XArrayFormulaRange oObj = null;
    String formula = "=1 + 2 * 5";

    /**
    * Test calls the method and then checks content sof spreadsheet using
    * object relation <code>'SHEET'</code>. <p>
    * Has <b> OK </b> status if values in cells of spreadsheet are equal to 11
    * or ArrayFormula not supported.<p>
    */
    public void _setArrayFormula() {
        Object noArray = tEnv.getObjRelation("noArray");
        if (noArray != null) {
            log.println("Component " + noArray.toString() +
                " doesn't really support this Interface");
            log.println("It doesn't make sense to set an ArrayFormula over"
                + " the whole sheet");
            tRes.tested("setArrayFormula()", true);
            return;
        }

        boolean result = true;
        double dresult = 11;

        log.println("setArrayFormula() ...");

        oObj.setArrayFormula(formula);

        log.println("checking that formula was set correctly...");
        XCellRangeAddressable crAddr =
            UnoRuntime.queryInterface(XCellRangeAddressable.class, oObj);
        CellRangeAddress addr = crAddr.getRangeAddress() ;
        XSpreadsheet oSheet = (XSpreadsheet)tEnv.getObjRelation("SHEET");
        if (oSheet == null) throw new StatusException(Status.failed
            ("Relation 'SHEET' not found"));

        XCell oCell = null;
        double value;

        for (int i = addr.StartColumn; i <= addr.EndColumn; i++)
            for (int j = addr.StartRow; j <= addr.EndRow; j++) {
                try {
                    oCell = oSheet.getCellByPosition(i, j);
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    e.printStackTrace(log);
                    result = false;
                    break;
                }

                value = oCell.getValue();
                result &= (value == dresult);
            }

        tRes.tested("setArrayFormula()", result) ;

    } // end setArrayFormula()

    /**
    * Test calls the method and compare formula that set by method
    * <code>setArrayFormula</code> with returned value ignoring spaces. <p>
    *
    * Has <b> OK </b> status if values are equal or
    * ArrayFormula not supported. <p>
    *
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setArrayFormula </code> : to set formula </li>
    * </ul>
    */
    public void _getArrayFormula() {

        Object noArray = tEnv.getObjRelation("noArray");
        if (noArray != null) {
            log.println("Component "+noArray.toString()+" doesn't really support this Interface");
            log.println("It doesn't make sense to set an ArrayFormula over the whole sheet");
            log.println("and therefore 'getArrayFormula()' won't work");
            tRes.tested("getArrayFormula()",true);
            return;
        }

        requiredMethod("setArrayFormula()");
        boolean result = true;
        log.println("Testing getArrayFormula() ...");
        String gFormula = oObj.getArrayFormula() ;
        result &= equalIgnoreSpaces("{" + formula + "}", gFormula);
        if (!result)
            log.println("Method returned : '" + oObj.getArrayFormula() + "'") ;
        tRes.tested("getArrayFormula()", result) ;

    } // end getArrayFormula()

    /**
     * Method compares two string ignoring spaces.
     * @return <code>true</code> if the argument
     * is not null and the Strings are equal,
     * ignoring spaces; <code>false</code> otherwise.
     */
    private boolean equalIgnoreSpaces(String s1, String s2) {
        int p1 = 0, p2 = 0 ;
        s1 = s1.trim() ;
        s2 = s2.trim() ;
        while (p1 < s1.length() && p2 < s2.length()) {
            while (s1.charAt(p1) == ' ') { p1 ++; }
            while (s2.charAt(p2) == ' ') { p2 ++; }
            if (s1.charAt(p1) != s2.charAt(p2)) return false ;
            p1 ++ ;
            p2 ++ ;
        }

        return p1 == s1.length() && p2 == s2.length() ;
    }

    /**
    * Forces environment recreation.
    */
    @Override
    protected void after() {
        disposeEnvironment();
    }
}

