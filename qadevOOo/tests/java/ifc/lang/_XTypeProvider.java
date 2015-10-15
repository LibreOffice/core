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

package ifc.lang;

import lib.MultiMethodTest;

import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;

/**
* Testing <code>com.sun.star.lang.XTypeProvider</code>
* interface methods :
* <ul>
*  <li><code> getTypes()</code></li>
*  <li><code> getImplementationId()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.lang.XTypeProvider
*/
public class _XTypeProvider extends MultiMethodTest {

    public static XTypeProvider oObj = null;
    public static Type[] types = null;

    /**
    * Just calls the method.<p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getImplementationId() {
        log.println("testing getImplementationId() ... ");

        boolean result = oObj.getImplementationId().length > 0;

        tRes.tested("getImplementationId()", result);

    } // end getImplementationId()


    /**
    * Calls the method and checks the return value.<p>
    * Has <b>OK</b> status if one of the return value equals to the
    * type <code>com.sun.star.lang.XTypeProvider</code>.
    */
    public void _getTypes() {
        boolean result = false;
        log.println("getting Types...");
        types = oObj.getTypes();
        for (int i=0;i<types.length;i++) {
            int k = i+1;
            log.println(k+". Type is "+types[i].toString());
            if (types[i].toString().equals
                    ("Type[com.sun.star.lang.XTypeProvider]")) {
                result = true;
            }
        }
        if (!result) {
            log.println("Component must provide Type "
                +"<com.sun.star.lang.XTypeProvider>");
        }

        tRes.tested("getTypes()", result);

    } // end getTypes()

}

