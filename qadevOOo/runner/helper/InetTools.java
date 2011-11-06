/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
