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

import com.sun.star.lang.XServiceInfo;

/**
* Testing <code>com.sun.star.lang.XServiceInfo</code>
* interface methods :
* <ul>
*  <li><code> getImplementationName()</code></li>
*  <li><code> supportsService()</code></li>
*  <li><code> getSupportedServiceNames()</code></li>
* </ul> <p>
* Test is multithread compliant. <p>
* @see com.sun.star.lang.XServiceInfo
*/
public class _XServiceInfo extends MultiMethodTest {
    public static XServiceInfo oObj = null;
    public static String[] names = null;

    /**
    * Just calls the method.<p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getImplementationName() {
        boolean result = true;
        log.println("testing getImplementationName() ... ");

        log.println("The ImplementationName is "+oObj.getImplementationName());
        result=true;

        tRes.tested("getImplementationName()", result);

    } // end getImplementationName()


    /**
    * Just calls the method.<p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public void _getSupportedServiceNames() {
        boolean result = true;
        log.println("getting supported Services...");
        names = oObj.getSupportedServiceNames();
        for (int i=0;i<names.length;i++) {
           int k = i+1;
           log.println(k+". Supported Service is "+names[i]);
        }
        result=true;

        tRes.tested("getSupportedServiceNames()", result);

    } // end getSupportedServiceNames()

    /**
    * Gets one of the service names returned by
    * <code>getSupportedServiceNames</code> method and
    * calls the <code>supportsService</code> method with this
    * name. <p>
    * Has <b>OK</b> status if <code>true</code> value is
    * returned.
    */
    public void _supportsService() {
        log.println("testing supportsService");
        names = oObj.getSupportedServiceNames();
        tRes.tested("supportsService()", oObj.supportsService(names[0]));
    } // end supportsService()
}

