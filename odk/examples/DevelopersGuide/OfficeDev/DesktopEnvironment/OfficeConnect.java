/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

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
     * m_xRemoteContext wich means remote component context of the connected office.
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
            ex.printStackTrace(System.err);
            System.exit(1);

        }
    }

    // ____________________

    /**
     * create uno components inside remote office process
     * After connection of these proccess to a running office we have access to
     * remote service manager of it.
     * So we can use it to create all existing services. Use this method to create
     * components by name and get her interface. Casting of it to right target
     * interface is part of your implementation.
     *
     * @param  aType  describe class type of created service
     *                Returned object can be casted directly to this one.
     *                Uno query was done by this method automaticly.
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

