/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InetTools.java,v $
 * $Revision: 1.4 $
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
package helper;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

// access the implementations via names
import com.sun.star.uno.XInterface;
import com.sun.star.util.XChangesBatch;

import lib.TestParameters;

import util.DesktopTools;


public class InetTools {
    /**
     * Helper method: sets the HTTP-Proxy to values from
     * <code>lib.TestParameters</code>
     */
    public static boolean setHTTPProxy(TestParameters param){
        XMultiServiceFactory xMSF = (XMultiServiceFactory) param.getMSF();
        PropertyValue[] ProvArgs = new PropertyValue[1];
        PropertyValue Arg = new PropertyValue();
        Arg.Name = "nodepath";
        Arg.Value = "/org.openoffice.Inet/Settings";
        ProvArgs[0] = Arg;

        try {
            Object oProvider = xMSF.createInstance(
                                       "com.sun.star.configuration.ConfigurationProvider");

            XMultiServiceFactory oProviderMSF = (XMultiServiceFactory) UnoRuntime.queryInterface(
                                                        XMultiServiceFactory.class,
                                                        oProvider);

            Object oInet = oProviderMSF.createInstanceWithArguments(
                                   "com.sun.star.configuration.ConfigurationUpdateAccess",
                                   ProvArgs);

            XPropertySet oInetProps = (XPropertySet) UnoRuntime.queryInterface(
                                              XPropertySet.class, oInet);

            String HTTPProxyName = (String)param.get("HTTPProxyName");
            String HTTPProxyPort = (String)param.get("HTTPProxyPort");

            if ((HTTPProxyName == null) || (HTTPProxyPort == null)) {
                return false;
            }

            oInetProps.setPropertyValue("ooInetHTTPProxyName", HTTPProxyName);
            oInetProps.setPropertyValue("ooInetHTTPProxyPort", HTTPProxyPort);
            oInetProps.setPropertyValue("ooInetProxyType", new Long(2));

            XChangesBatch oSecureChange = (XChangesBatch) UnoRuntime.queryInterface(
                                                  XChangesBatch.class, oInet);
            oSecureChange.commitChanges();
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }
        return true;
    }
}