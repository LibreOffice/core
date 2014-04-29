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



// __________ Imports __________

// structs, const, ...
import com.sun.star.beans.PropertyValue;
import com.sun.star.bridge.XUnoUrlResolver;

// exceptions
import com.sun.star.container.NoSuchElementException;

// interfaces
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Any;
import com.sun.star.uno.Exception;

import com.sun.star.uno.UnoRuntime;

// others
import java.lang.String;

// __________ Implementation __________

/**
 * support ONE singleton uno connection to an running office installation!
 * Can be used to open/use/close connection to uno environment of an already running office.
 * ctor isn't available from outside. You should call static function "getConnection()"
 * to open or use internal set connection which is created one times only.
 *
 * @author      Andreas Schl&uuml;ns
 * @created     7. Februar 2002
 * @modified    05.02.2002 12:10
 */
public class OfficeConnect
{
    // ____________________

    /**
     * At first call we create static connection object and get the remote office
     * context and the remote office service manager. A new office process is
     * started if necessary.
     * Then - and for all further requests we return these static connection member.
     */
    public static synchronized OfficeConnect createConnection()
        throws java.lang.Exception
    {
        if (maConnection == null)
        {
            maConnection = new OfficeConnect();
        }
        return maConnection;
    }

    // ____________________

    public static synchronized OfficeConnect getConnection()
    {
        return maConnection;
    }

    // ____________________

    /**
     * ctor
     * We try to open the connection in our ctor ... transparently for the user.
     * We made it private to support singleton pattern of these implementation.
     * see getConnection() for further informations
     */
    private OfficeConnect() throws java.lang.Exception
    {
        // get the remote office context. If necessary a new office
        // process is started
        mxOfficeContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
        System.out.println("Connected to a running office ...");
        mxServiceManager = mxOfficeContext.getServiceManager();
    }

    // ____________________

    /**
     * create uno components inside remote office process
     * After connection of these process to a running office we have access to remote service manager of it.
     * So we can use it to create all existing services. Use this method to create components by name and
     * get her interface. Casting of it to right target interface is part of your implementation.
     *
     * @param  aType              describe class type of created service
     *                              Returned object can be casted directly to this one.
     *                              Uno query was done by this method automatically.
     * @param  sServiceSpecifier  name of service which should be created
     * @return                    Description of the Returned Value
     */
    public Object createRemoteInstance(Class aType, String sServiceSpecifier)
    {
        Object aResult = null;
        try
        {
            aResult = UnoRuntime.queryInterface(
                    aType, mxServiceManager.createInstanceWithContext(
                        sServiceSpecifier, mxOfficeContext));
        }
        catch (com.sun.star.uno.Exception ex)
        {
            System.err.println("Couldn't create Service of type " + sServiceSpecifier + ": " + ex);
            ex.printStackTrace();
            System.exit(0);
        }
        return aResult;
    }

    // ____________________

    /**
     * same as "createRemoteInstance()" but supports additional parameter for initializing created object
     *
     * @param  lArguments         optional arguments
     *                      They are used to initialize new created service.
     * @param  aType              Description of Parameter
     * @param  sServiceSpecifier  Description of Parameter
     * @return                    Description of the Returned Value
     */
    public Object createRemoteInstanceWithArguments(Class aType, String sServiceSpecifier, Any[] lArguments)
    {
        Object aResult = null;
        try
        {
            aResult = UnoRuntime.queryInterface(
                aType, mxServiceManager.createInstanceWithArgumentsAndContext(
                    sServiceSpecifier, lArguments, mxOfficeContext));
        }
        catch (com.sun.star.uno.Exception ex)
        {
            System.err.println("Couldn't create Service of type " + sServiceSpecifier + ": " + ex);
            ex.printStackTrace();
            System.exit(0);
        }
        return aResult;
    }

    // ____________________

    /**
     * member
     */
    // singleton connection instance
    private static OfficeConnect maConnection;

    // reference to remote office context
    private com.sun.star.uno.XComponentContext  mxOfficeContext;
    // reference to remote service manager
    private com.sun.star.lang.XMultiComponentFactory  mxServiceManager;
}

