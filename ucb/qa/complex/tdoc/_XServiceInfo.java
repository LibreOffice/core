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

package complex.tdoc;

import lib.MultiMethodTest;

import com.sun.star.lang.XServiceInfo;
import share.LogWriter;

/**
* Testing <code>com.sun.star.lang.XServiceInfo</code>
* interface methods :
* <ul>
*  <li><code> getImplementationName()</code></li>
*  <li><code> supportsService()</code></li>
*  <li><code> getSupportedServiceNames()</code></li>
* </ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.lang.XServiceInfo
*/
public class _XServiceInfo {
    public static XServiceInfo oObj = null;
    public static String[] names = null;
    public LogWriter log = null;

    /**
    * Just calls the method.<p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public boolean _getImplementationName() {
        boolean result = true;
        log.println("testing getImplementationName() ... ");

        log.println("The ImplementationName ist "+oObj.getImplementationName());
        result=true;

        return result;

    } // end getImplementationName()


    /**
    * Just calls the method.<p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public boolean _getSupportedServiceNames() {
        boolean result = true;
        log.println("getting supported Services...");
        names = oObj.getSupportedServiceNames();
        for (int i=0;i<names.length;i++) {
           int k = i+1;
           log.println(k+". Supported Service is "+names[i]);
        }
        result=true;

        return result;

    } // end getSupportedServiceNames()

    /**
    * Gets one of the service names returned by
    * <code>getSupportedServiceNames</code> method and
    * calls the <code>supportsService</code> methos with this
    * name. <p>
    * Has <b>OK</b> status if <code>true</code> value is
    * returned.
    */
    public boolean _supportsService() {
        log.println("testing supportsService");
        names = oObj.getSupportedServiceNames();
        return oObj.supportsService(names[0]);
    } // end supportsService()
}

