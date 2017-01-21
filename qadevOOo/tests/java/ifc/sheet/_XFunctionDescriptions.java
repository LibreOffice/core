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

import java.util.Random;

import lib.MultiMethodTest;

import com.sun.star.beans.PropertyValue;
import com.sun.star.sheet.XFunctionDescriptions;

/**
* Testing <code>com.sun.star.sheet.XFunctionDescriptions</code>
* interface methods :
* <ul>
*  <li><code> getById()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XFunctionDescriptions
*/
public class _XFunctionDescriptions extends MultiMethodTest {

    public XFunctionDescriptions oObj = null;

    /**
    * Test finds available id, calls method using this id, checks returned
    * value and then tries to get description with wrong id. <p>
    * Has <b>OK</b> status if returned value is equal to value obtained by the
    * method <code>getByIndex()</code> in first call and exception
    * <code>IllegalArgumentException</code> was thrown in second call.<p>
    * @see com.sun.star.lang.IllegalArgumentException
    */
    public void _getById() {
        boolean bResult = true;
        // Finding available id...

        int count = oObj.getCount();
        if (count > 0) {
            Random rnd = new Random();
            int nr = rnd.nextInt(count);

            PropertyValue[] PVals = null;
            try {
                PVals = (PropertyValue[])oObj.getByIndex(nr);
            } catch(com.sun.star.lang.WrappedTargetException e) {
                e.printStackTrace(log);
                tRes.tested("getById()", false);
                return;
            } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
                e.printStackTrace(log);
                tRes.tested("getById()", false);
                return;
            }

            String FName = null;
            Integer FId = null;

            for (int i = 0; i < PVals.length; i++) {
                if (PVals[i].Name.equals("Name"))
                    FName = (String)PVals[i].Value;
                if (PVals[i].Name.equals("Id"))
                    FId = (Integer)PVals[i].Value;
            }

            log.println("The id of function '" + FName + "' is " + FId);

            PropertyValue[] PVals2 = null;
            try {
                PVals2 = oObj.getById(FId.intValue());
            } catch(com.sun.star.lang.IllegalArgumentException e) {
                e.printStackTrace(log);
                tRes.tested("getById()", false);
                return;
            }

            String objFName = null;
            Integer objFId = null;
            for (int i = 0; i < PVals2.length; i++) {
                if (PVals2[i].Name.equals("Name"))
                    objFName = (String)PVals[i].Value;
                if (PVals2[i].Name.equals("Id"))
                    objFId = (Integer)PVals[i].Value;
            }

            log.println("The id of returned function '" +
                objFName + "' is " + objFId);

            bResult &= FName.equals(objFName);
            bResult &= FId.equals(objFId);
        }

        log.println("OK.");

        if (bResult) {
            try {
                log.println("Now trying to get description with wrong id ... ");
                oObj.getById(-1);
                bResult = false;
                log.println("Exception expected! - FAILED");
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("Expected exception " + e + " - OK!");
            }
        }

        tRes.tested("getById()", bResult);
    }
}  // finish class _XFunctionDescriptions


