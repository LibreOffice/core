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

package ifc.container;

import lib.MultiMethodTest;

import com.sun.star.container.XElementAccess;

/**
 * Testing <code>com.sun.star.container.XElementAccess</code>
 * interface methods :
 * <ul>
 *  <li><code> getElementType()</code></li>
 *  <li><code> hasElements()</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.container.XElementAccess
 */
public class _XElementAccess extends MultiMethodTest {
    public XElementAccess oObj = null;

    /**
    * Test calls the method and checks return value. <p>
    * Has <b> OK </b> status if the method returns <code>true</code>,
    * only in the case the container is not empty.<p>
    */
    public void _hasElements(){
        // Testing hasElements() method
        tRes.tested( "hasElements()", oObj.hasElements() );
    } // end hasElements

    /**
    * Test calls the method and checks return value. <p>
    * Has <b> OK </b> status if the method returns not null value.<p>
    */
    public void _getElementType(){
        // Testing getElementType() method
        com.sun.star.uno.Type type = oObj.getElementType();
        tRes.tested( "getElementType()", type != null );
    } // end ElementType

} /// finished class _XElementAccess


