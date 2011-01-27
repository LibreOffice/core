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

import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;


/**
* Testing <code>com.sun.star.container.XEnumerationAccess</code>
* interface methods. <p>
* Test is multithread compilant. <p>
*/
public class _XEnumerationAccess extends MultiMethodTest{
    public XEnumerationAccess oObj = null;

    /**
    * Test calls the method and checks return value. <p>
    * Has <b> OK </b> status if the method returns not null value. <p>
    */
    public void _createEnumeration(){
        log.println( "createing Enumeration" );
        XEnumeration oEnum = oObj.createEnumeration();
        tRes.tested( "createEnumeration()", oEnum != null );
        return;
    } // end createEnumeration()
}  // finish class _XEnumerationAccess



