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

package ifc.document;

import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.document.LinkTarget</code>
* service properties :
* <ul>
*  <li><code> LinkDisplayName </code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.document.LinkTarget
*/
public class _LinkTarget extends MultiPropertyTest {

/*
    public XPropertySet oObj = null;  // oObj filled by MultiMethodTest

    public void _LinkDisplayName() {
        // check if Service is available
        XServiceInfo xInfo = (XServiceInfo)
            UnoRuntime.queryInterface(XServiceInfo.class, oObj);
        String [] sa = xInfo.getSupportedServiceNames();

        for (int i = 0; i < sa.length; i++) {
            System.out.println(sa[i]);
        }

        if ( ! xInfo.supportsService("com.sun.star.document.LinkTarget")) {
            log.println("Service not available !!!!!!!!!!!!!");
            tRes.tested("Supported", false);
        }
        //the only property is read only
        try {
            String LDN = (String) oObj.getPropertyValue("LinkDisplayName");
            tRes.tested("LinkDisplayName",LDN!=null);
        }
        catch (Exception ex) {
            log.println("Exception occurred during LinkDisplayName");
            ex.printStackTrace(log);
            tRes.tested("LinkDisplayName",false);
        }

    }// EOF LinkDisplayName
*/

}

