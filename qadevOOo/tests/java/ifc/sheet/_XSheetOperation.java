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

import com.sun.star.sheet.CellFlags;
import com.sun.star.sheet.GeneralFunction;
import com.sun.star.sheet.XSheetOperation;

/**
* Testing <code>com.sun.star.sheet.XSheetOperation</code>
* interface methods :
* <ul>
*  <li><code> computeFunction()</code></li>
*  <li><code> clearContents()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetOperation
*/
public class _XSheetOperation extends MultiMethodTest {

    public XSheetOperation oObj = null;

    /**
    * Test clears formula and value contents, calls the method
    * <code>computeFunction</code> and checks returned value. <p>
    * Has <b> OK </b> status if returned value is equal to zero
    * and no exceptions were thrown. <p>
    */
    public void _clearContents() {
        boolean result = true;
        double resultVal = -1;

        log.println("Testing clearContents() ...");

        int allFlags;
        allFlags = CellFlags.VALUE | CellFlags.FORMULA;

        oObj.clearContents (allFlags) ;

        try {
            resultVal = oObj.computeFunction(GeneralFunction.SUM);
            result &= (resultVal == 0.0) || (resultVal == 0);
        } catch (com.sun.star.uno.Exception e) {
            result &= false ;
            log.println(
                    "Exception occurred while checking results of method");
            e.printStackTrace(log);
        }

        tRes.tested("clearContents()", result);

    } // finished clearContents

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is equal or greate than zero
    * and no exceptions were thrown. <p>
    */
    public void _computeFunction() {

        log.println("Testing computeFunction() ...");
        double resultVal = -1;
        boolean result = true;

        try {
            resultVal = oObj.computeFunction (GeneralFunction.COUNT) ;
            result = resultVal >= 0;
        } catch (com.sun.star.uno.Exception e) {
            result = false;
            log.println("Exception occurred in method computeFunction.");
            e.printStackTrace(log);
        }

        tRes.tested("computeFunction()", result);
    } // finished computeFunction

} // finished class _XSheetOperation

