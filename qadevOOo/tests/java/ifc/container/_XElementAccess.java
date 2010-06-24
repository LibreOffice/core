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


