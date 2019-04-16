/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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




// __________ Implementation __________

/**
 * support ONE singleton uno connection to an running office installation!
 * Can be used to open/use/close connection to uno environment of an office. If
 * necessary a new office instance is started.
 * ctor isn't available from outside. You should call static function
 * "getConnection()" to open or use internal set connection which is created one
 * times only.
 *
 */
public class OfficeConnect
{


    /**
     * At first call we create static connection object and open connection to an
     * office - a new office instance is started if necessary
     * Then - and for all further requests we return these static connection member.
     */
    public static synchronized void createConnection()
    {
        if (maConnection == null)
            maConnection = new OfficeConnect();
    }







    /**
     * ctor
     * We try to open the connection in our ctor ... transparently for user.
     * After it was successful you will find an internal set member
     * m_xRemoteContext which means remote component context of the connected office.
     * The context can be used to get the remote service manager from the office.
     * We made it private to support singleton pattern of these implementation.
     * see getConnection() for further information
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







    /**
     * returns remote component context of the connected office
     */
    public static synchronized com.sun.star.uno.XComponentContext getOfficeContext()
    {
        return mxOfficeContext;
    }



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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
