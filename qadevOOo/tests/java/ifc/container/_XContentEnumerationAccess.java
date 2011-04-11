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

import com.sun.star.container.XContentEnumerationAccess;
import com.sun.star.container.XEnumeration;

/**
* Testing <code>com.sun.star.container.XContentEnumerationAccess</code>
* interface methods :
* <ul>
*  <li><code> createContentEnumeration()</code></li>
*  <li><code> getAvailableServiceNames()</code></li>
* </ul> <p>
* @see com.sun.star.container.XContentEnumerationAccess
*/
public class _XContentEnumerationAccess extends MultiMethodTest{
    public XContentEnumerationAccess oObj = null;
    String[] serviceNames = null;

    /**
    * Retrieves service names and stores them. <p>
    * Has <b> OK </b> status if not <code>null</code>
    * value returned.
    */
    public void _getAvailableServiceNames(){
        boolean bResult = true;
        try {
            serviceNames = oObj.getAvailableServiceNames();
            bResult = serviceNames != null ;
        } catch (Exception e) {
            log.println("Exception occurred. " + e);
            bResult = false;
        }
        tRes.tested("getAvailableServiceNames()", bResult);
    }

    /**
    * If at least one service available then an enumeration for
    * it created. <p>
    * Has <b> OK </b> status if no services available or enumeration
    * created for available service is not <code>null</code>.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code>getAvailableServiceNames()</code> :
    *    to have at least one service name for enumeration to create for.</li>
    * </ul>
    */
    public void _createContentEnumeration(){
        requiredMethod("getAvailableServiceNames()");

        if (serviceNames.length == 0) {
            log.println("No service name available") ;
            tRes.tested("createContentEnumeration()", true) ;
            return ;
        }

        boolean bResult = true;

        log.println( "creating Enumeration" );
        XEnumeration oEnum = oObj.createContentEnumeration(serviceNames[0]);
        bResult &= oEnum != null;

        tRes.tested( "createContentEnumeration()",  bResult);
    }
}


