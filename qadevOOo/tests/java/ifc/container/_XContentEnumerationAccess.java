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


