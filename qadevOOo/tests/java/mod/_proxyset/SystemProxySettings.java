/*************************************************************************
 *
 *  $RCSfile: SystemProxySettings.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:00:24 $
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

