/*************************************************************************
 *
 *  $RCSfile: _XServiceInfo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:46:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
* Test is multithread compilant. <p>
* @see com.sun.star.lang.XServiceInfo
*/
public class _XServiceInfo extends MultiMethodTest {
    public static XServiceInfo oObj = null;
    public static String[] names = null;

    /**
    * Just calls the method.<p>
    * Has <b>OK</b> status if no runtime exceptions occured.
    */
    public void _getImplementationName() {
        boolean result = true;
        log.println("testing getImplementationName() ... ");

        log.println("The ImplementationName ist "+oObj.getImplementationName());
        result=true;

        tRes.tested("getImplementationName()", result);

    } // end getImplementationName()


    /**
    * Just calls the method.<p>
    * Has <b>OK</b> status if no runtime exceptions occured.
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
    * calls the <code>supportsService</code> methos with this
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

