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
package helper;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;

// access the implementations via names
import com.sun.star.util.XChangesBatch;

import lib.TestParameters;


public class InetTools {
    /**
     * Helper method: sets the HTTP-Proxy to values from
     * <code>lib.TestParameters</code>
     */
    public static boolean setHTTPProxy(TestParameters param){
        XMultiServiceFactory xMSF = param.getMSF();
        PropertyValue[] ProvArgs = new PropertyValue[1];
        PropertyValue Arg = new PropertyValue();
        Arg.Name = "nodepath";
        Arg.Value = "/org.openoffice.Inet/Settings";
        ProvArgs[0] = Arg;

        try {
            Object oProvider = xMSF.createInstance(
                                       "com.sun.star.configuration.ConfigurationProvider");

            XMultiServiceFactory oProviderMSF = UnoRuntime.queryInterface(
                                                        XMultiServiceFactory.class,
                                                        oProvider);

            Object oInet = oProviderMSF.createInstanceWithArguments(
                                   "com.sun.star.configuration.ConfigurationUpdateAccess",
                                   ProvArgs);

            XPropertySet oInetProps = UnoRuntime.queryInterface(
                                              XPropertySet.class, oInet);

            String HTTPProxyName = (String)param.get("HTTPProxyName");
            String HTTPProxyPort = (String)param.get("HTTPProxyPort");

            if ((HTTPProxyName == null) || (HTTPProxyPort == null)) {
                return false;
            }

            oInetProps.setPropertyValue("ooInetHTTPProxyName", HTTPProxyName);
            oInetProps.setPropertyValue("ooInetHTTPProxyPort", HTTPProxyPort);
            oInetProps.setPropertyValue("ooInetProxyType", new Long(2));

            XChangesBatch oSecureChange = UnoRuntime.queryInterface(
                                                  XChangesBatch.class, oInet);
            oSecureChange.commitChanges();
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace();
        }
        return true;
    }
}
