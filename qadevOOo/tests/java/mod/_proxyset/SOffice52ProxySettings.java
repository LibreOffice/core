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



package mod._proxyset;

import java.io.PrintWriter;
import java.util.Hashtable;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.system.SOffice52ProxySettings</code>. <p>
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
public class SOffice52ProxySettings extends TestCase {

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of
     * <code>com.sun.star.system.SOffice52ProxySettings</code>,for testing.
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
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        XInterface oObj = null;

        try {
            oObj = (XInterface)xMSF.createInstance(
                        "com.sun.star.system.SOffice52ProxySettings");
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

        String Iname = util.utils.getImplName(oObj);
        log.println("Implementation Name: "+Iname);
        TestEnvironment tEnv = new TestEnvironment(oObj);

        // extracting parameters to proxy settings
        Hashtable proxySettings = new Hashtable(12);

        String prefix = "test.proxy.soffice52.";

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

}    // finish class TestCase

