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

// base classes
import com.sun.star.uno.UnoRuntime;

// factory for creating components
import com.sun.star.beans.PropertyValue;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

// Exceptions
import com.sun.star.uno.RuntimeException;


/** @descr  This class establishes a connection to a StarOffice application.
 */
public class OfficeConnection
{
    public OfficeConnection (int nPortNumber)
    {
        mnDefaultPort = nPortNumber;
        connect ();
    }

    /** @descr Return the service manager that represents the connected
                StarOffice application
    */
    public XMultiServiceFactory getServiceManager ()
    {
        if ( ! mbInitialized)
            connect ();
        return maServiceManager;
    }

    /** @descr  Return a flag that indicates if the constructor has been able to
                establish a valid connection.
    */
    public boolean connectionIsValid ()
    {
        return getServiceManager() != null;
    }

    /** @descr  Connect to a already running StarOffice application.
    */
    private void connect ()
    {
        connect (msDefaultHost, mnDefaultPort);
    }

    private void connect (String hostname)
    {
        connect (hostname, mnDefaultPort);
    }

    /** @descr  Connect to a already running StarOffice application that has
                been started with a command line argument like
                "-accept=socket,host=localhost,port=5678;urp;"
    */
    private void connect (String hostname, int portnumber)
    {
        mbInitialized = true;
        //  Set up connection string.
        String sConnectString = "uno:socket,host=" + hostname + ",port=" + portnumber
            + ";urp;StarOffice.ServiceManager";


        // connect to a running office and get the ServiceManager
        try
        {
            //  Create a URL Resolver.
            XMultiServiceFactory aLocalServiceManager =
                com.sun.star.comp.helper.Bootstrap.createSimpleServiceManager();
            XUnoUrlResolver aURLResolver = (XUnoUrlResolver) UnoRuntime.queryInterface (
                XUnoUrlResolver.class,
                aLocalServiceManager.createInstance ("com.sun.star.bridge.UnoUrlResolver")
                );

            maServiceManager = (XMultiServiceFactory) UnoRuntime.queryInterface (
                    XMultiServiceFactory.class,
                    aURLResolver.resolve (sConnectString)
                    );
        }

        catch (Exception e)
        {
            MessageArea.println ("Could not connect with " + sConnectString + " : " + e);
            MessageArea.println ("Please start OpenOffice/StarOffice with "
                + "\"-accept=socket,host=localhost,port=5678;urp;\"");
        }
    }

    private int mnDefaultPort = 5678;
    private final String msDefaultHost = "localhost";
    private XMultiServiceFactory  maServiceManager = null;

    /** A value of true just indicates that it has been tried to establish a connection,
        not that that has been successful.
    */
    private boolean mbInitialized = false;
}
