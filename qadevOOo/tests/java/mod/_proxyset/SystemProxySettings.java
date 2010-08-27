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

package mod._proxyset;

import java.io.PrintWriter;
import java.util.Hashtable;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.system.SystemProxySettings</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
 *  <li> <code>com::sun::star::system::XProxySettings</code></li>
 *  <li> <code>com::sun::star::lang::XTypeProvider</code></li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.lang.XServiceInfo
 * @see com.sun.star.system.XProxySettings
 * @see com.sun.star.lang.XTypeProvider
 * @see ifc.lang._XServiceInfo
 * @see ifc.system._XProxySettings
 * @see ifc.lang._XTypeProvider
 */
public class SystemProxySettings extends TestCase {

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of
     * <code>com.sun.star.system.SystemProxySettings</code>,for testing.
     *
     *     Object relations created :
     * <ul>
     *  <li> <code>'XProxySettings.proxySettings'</code> for
     *      {@link ifc.system._XProxySettings} : </li>
     *   <p>It passes a Hashtable with expected proxy settings as object
     *   relation "XProxySettings.proxySettings", to verify results. The expected
     *   settings are taken from parameters. The following parameters are recognized:
     *   <ul>
     *   <li>test.proxy.soffice52.ftpProxyAddress</li>
     *   <li>test.proxy.soffice52.ftpProxyPort</li>
     *   <li>test.proxy.soffice52.gopherProxyAddress</li>
     *   <li>test.proxy.soffice52.gopherProxyPort</li>
     *   <li>test.proxy.soffice52.httpProxyAddress</li>
     *   <li>test.proxy.soffice52.httpProxyPort</li>
     *   <li>test.proxy.soffice52.httpsProxyAddress</li>
     *   <li>test.proxy.soffice52.httpsProxyPort</li>
     *   <li>test.proxy.soffice52.socksProxyAddress</li>
     *   <li>test.proxy.soffice52.socksProxyPort</li>
     *   <li>test.proxy.soffice52.proxyBypassAddress</li>
     *   <li>test.proxy.soffice52.proxyEnabled</li>
     *   </ul>.
     * </ul>
     */
    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {
        XInterface oObj = null;
        Object oInterface = null;

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
            oInterface = xMSF.createInstance
                ( "com.sun.star.system.SystemProxySettings" );
        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Service not available" );
        }

        oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // extracting parameters to proxy settings
        Hashtable proxySettings = new Hashtable(12);

        String prefix = "test.proxy.system.";

        final String[] names = {
            "ftpProxyAddress",
            "ftpProxyPort",
            "gopherProxyAddress",
            "gopherProxyPort",
            "httpProxyAddress",
            "httpProxyPort",
            "httpsProxyAddress",
            "httpsProxyPort",
            "socksProxyAddress",
            "socksProxyPort",
            "proxyBypassAddress",
            "proxyEnabled"
        };

        for (int i = 0; i < names.length; i++) {
            String name = prefix + names[i];
            String value = (String) tParam.get(name);

            if (value == null) {
                value = "";
            }

            proxySettings.put(names[i], value);
        }

        tEnv.addObjRelation("XProxySettings.proxySettings", proxySettings);

        return tEnv;
    }
}

