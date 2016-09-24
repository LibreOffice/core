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
    * Has <b> OK </b> status if returned value is equal or greater than zero
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

