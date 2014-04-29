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
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Any;

import java.lang.String;


// __________ Implementation __________

/**
 * support ONE singleton uno connection to an running office installation!
 * Can be used to open/use/close connection to uno environment of an office. If
 * necessary a new office instance is started.
 * ctor isn't available from outside. You should call static function
 * "getConnection()" to open or use internal set connection which is created one
 * times only.
 *
 * @author      Andreas Schl&uuml;ns
 * @created     7. Februar 2002
 * @modified    05.02.2002 12:10
 */
public class OfficeConnect
{
    // ____________________

    /**
     * At first call we create static connection object and open connection to an
     * office - anew offic einstance is started if necessary
     * Then - and for all further requests we return these static connection member.
     */
    public static synchronized void createConnection()
    {
        if (maConnection == null)
            maConnection = new OfficeConnect();
    }

    // ____________________

    /**
     * close connection to remote office if it exist
     */
    public static synchronized void disconnect()
    {
        if(maConnection!=null)
        {
            mxServiceManager=null;
            mxOfficeContext=null;
            maConnection=null;
        }
    }

    // ____________________

    /**
     * ctor
     * We try to open the connection in our ctor ... transparently for user.
     * After it was successfully you will find an internal set member
     * m_xRemoteContext which means remote component context of the connected office.
     * The context can be used to get the remote service manager from the office.
     * We made it private to support singleton pattern of these implementation.
     * see getConnection() for further informations
     */
    private OfficeConnect()
    {
        try
        {
            // get the remote office context. If necessary a new office
            // process is started
            mxOfficeContext = com.sun.star.comp.helper.Bootstrap.bootstrap();
            System.out.println("Connected to a running office ...");
            mxServiceManager = mxOfficeContext.getServiceManager();
        }
        catch (java.lang.Exception ex)
        {
            System.err.println("connection failed" + ex);
            ex.printStackTrace(System.out);
            System.exit(1);

        }
    }

    // ____________________

    /**
     * create uno components inside remote office process
     * After connection of these process to a running office we have access to
     * remote service manager of it.
     * So we can use it to create all existing services. Use this method to create
     * components by name and get her interface. Casting of it to right target
     * interface is part of your implementation.
     *
     * @param  aType  describe class type of created service
     *                Returned object can be casted directly to this one.
     *                Uno query was done by this method automatically.
     * @param  sServiceSpecifier  name of service which should be created
     * @return  the new created service object
     */
    public static synchronized Object createRemoteInstance(
        Class aType, String sServiceSpecifier)
    {
        Object aResult = null;
        try
        {
            aResult = UnoRuntime.queryInterface(aType,
                    mxServiceManager.createInstanceWithContext(
                        sServiceSpecifier, mxOfficeContext));
        }
        catch (com.sun.star.uno.Exception ex)
        {
            System.err.println("Couldn't create Service of type "
                               + sServiceSpecifier + ": " + ex);
            System.exit(0);
        }
        return aResult;
    }

    // ____________________

    /**
     * same as "createRemoteInstance()" but supports additional parameter for
     * initializing created object
     *
     * @param  lArguments         optional arguments
     *                            They are used to initialize new created service.
     * @param  aType              Description of Parameter
     * @param  sServiceSpecifier  Description of Parameter
     * @return                    the new create service object
     */
    public static synchronized Object createRemoteInstanceWithArguments(
        Class aType, String sServiceSpecifier, Any[] lArguments)
    {
        Object aResult = null;
        try
        {
            aResult = UnoRuntime.queryInterface(aType,
                    mxServiceManager.createInstanceWithArgumentsAndContext(
                        sServiceSpecifier, lArguments, mxOfficeContext));
        }
        catch (com.sun.star.uno.Exception ex)
        {
            System.err.println("Couldn't create Service of type "
                               + sServiceSpecifier + ": " + ex);
            System.exit(0);
        }
        return aResult;
    }

    // ____________________

    /**
     * returns remote component context of the connected office
     */
    public static synchronized com.sun.star.uno.XComponentContext getOfficeContext()
    {
        return mxOfficeContext;
    }

    // ____________________

    /**
     * member
     */
    // singleton connection instance
    private static OfficeConnect maConnection;

    // reference to the office component context
    private static com.sun.star.uno.XComponentContext  mxOfficeContext;
    // reference to remote service manager of singleton connection object
    private static com.sun.star.lang.XMultiComponentFactory  mxServiceManager;
}

