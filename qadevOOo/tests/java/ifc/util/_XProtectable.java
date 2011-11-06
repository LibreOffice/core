/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package ifc.util;

import com.sun.star.util.XProtectable;
import lib.MultiMethodTest;

/**
 * Check the XProtectable interface.
 */
public class _XProtectable extends MultiMethodTest {
    public XProtectable oObj = null;
    String sPassWord = "TopSecret";

    /**
     * Check, if the sheet is protected.
     * Has OK status, if this is the case.
     */
    public void _isProtected() {
        requiredMethod("protect()");
        boolean result = oObj.isProtected();
        tRes.tested("isProtected()", result);
    }

    /**
     * Protect the sheet
     */
    public void _protect() {
        oObj.protect(sPassWord);
        tRes.tested("protect()", true);
    }

    /**
     * Unprotect with wrong password, see if it's still protected.
     * Unprotect with correct password, see if it's unprotected.
     */
    public void _unprotect() {
        requiredMethod("isProtected()");
        boolean result = true;
        try {
            oObj.unprotect("WrongPassword");
//            result = false;
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Correct Exception thrown.");
        }
        // just check if it's still protected
        result &= oObj.isProtected();
        try {
            oObj.unprotect(sPassWord);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Wrong Exception thrown: password is correct.");
            result = false;
        }
        result &= !oObj.isProtected();
        tRes.tested("unprotect()", result);
    }

}
