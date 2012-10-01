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

import com.sun.star.uno.Any;
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
     * After connection of these proccess to a running office we have access to remote service manager of it.
     * So we can use it to create all existing services. Use this method to create components by name and
     * get her interface. Casting of it to right target interface is part of your implementation.
     *
     * @param  aType              describe class type of created service
     *                              Returned object can be casted directly to this one.
     *                              Uno query was done by this method automaticly.
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

