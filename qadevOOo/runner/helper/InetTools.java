/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InetTools.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:19:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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