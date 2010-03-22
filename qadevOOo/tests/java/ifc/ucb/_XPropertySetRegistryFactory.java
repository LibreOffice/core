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
* Test is <b> NOT </b> multithread compilant. <p>
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

