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

package ifc.util;

import lib.MultiMethodTest;

import java.util.Arrays;

import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloneable;

/**
* Testing <code>com.sun.star.util.XCloneable</code>
* interface methods :
* <ul>
*  <li><code> createClone()</code></li>
* </ul> <p>
* @see com.sun.star.util.XCloneable
*/
public class _XCloneable extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XCloneable oObj = null ;
    protected XCloneable clone = null;

    /**
     * calls the method. <p>
     * Has <b>OK</b> status if no exception has occurred. <p>
     */
    public void _createClone() {
        boolean result = true;
        clone = oObj.createClone();

        //check if the implementationname equals
        result &= checkImplementationName(oObj,clone);

        //check ImplementationID
        result &= checkImplementationID(oObj, clone);

        tRes.tested("createClone()", result) ;
    }

    protected byte[] getImplementationID(XInterface ifc) {
        byte[] res = new byte[0];
        XTypeProvider provider = UnoRuntime.queryInterface(XTypeProvider.class, ifc);
        if (provider != null) {
            res = provider.getImplementationId();
        }
        return res;
    }

    protected boolean checkImplementationID(XInterface org, XInterface clone) {
        boolean res = Arrays.equals(getImplementationID(org), getImplementationID(clone));
        if (res && getImplementationID(org).length > 0) {
            log.println("ImplementationID equals the clone has the same id as the original Object");
            log.println("------------------------------------------------------------------------");
        }
        return !res;
    }

    protected String getImplementationName(XInterface ifc) {
        String res = "";
        XServiceInfo info = UnoRuntime.queryInterface(XServiceInfo.class, ifc);
        if (info != null) {
            res = info.getImplementationName();
        }
        return res;
    }

    protected boolean checkImplementationName(XInterface org, XInterface clone) {
        boolean res = getImplementationName(org).equals(
                                            getImplementationName(clone));
        if (!res) {
            log.println("ImplementationName differs: ");
            log.println("Expected: "+getImplementationName(org));
            log.println("Gained: "+getImplementationName(clone));
            log.println("----------------------------------------");
        }
        return res;
    }

}  // finish class _XCloneable

