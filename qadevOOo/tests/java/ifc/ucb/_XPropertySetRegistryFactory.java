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

package ifc.ucb;

import lib.MultiMethodTest;

import com.sun.star.ucb.XPropertySetRegistry;
import com.sun.star.ucb.XPropertySetRegistryFactory;

/**
* Testing <code>com.sun.star.ucb.XPropertySetRegistryFactory</code>
* interface methods :
* <ul>
*  <li><code> createPropertySetRegistry()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.ucb.XPropertySetRegistryFactory
*/
public class _XPropertySetRegistryFactory extends MultiMethodTest {

    public static XPropertySetRegistryFactory oObj = null;

    /**
     * Test calls the method with empty string argument. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _createPropertySetRegistry() {
            XPropertySetRegistry PSR = oObj.createPropertySetRegistry("");
            tRes.tested("createPropertySetRegistry()",PSR != null);
    }

}

