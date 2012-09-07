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

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;


class UrlResolver
{
    public static void main( String [] args ) throws java.lang.Exception
    {
        if( args.length != 1 )
        {
            System.out.println( "usage: UrlResolver uno-url\n" +
                                "       uno-url  The uno-url identifying the object to\n" +
                                "                be imported, for instance\n" +
                                "                uno:socket,host=localhost,port=2083;urp;StarOffice.ServiceManager" +
                                "                (use \" on unix shells to avoid ;-problems" );
            System.exit( 1 );
        }

        // create default local component context
        XComponentContext xLocalContext =
            com.sun.star.comp.helper.Bootstrap.createInitialComponentContext(null);

        // initial serviceManager
        XMultiComponentFactory xLocalServiceManager = xLocalContext.getServiceManager();

        // create a urlresolver
        Object urlResolver  = xLocalServiceManager.createInstanceWithContext(
            "com.sun.star.bridge.UnoUrlResolver", xLocalContext );

        // query for the XUnoUrlResolver interface
        XUnoUrlResolver xUrlResolver =
            (XUnoUrlResolver) UnoRuntime.queryInterface( XUnoUrlResolver.class, urlResolver );

        try
        {
            // Import the object
            Object rInitialObject = xUrlResolver.resolve( args[0] );

            // XComponentContext
            if( null != rInitialObject )
            {
                System.out.println( "initial object successfully retrieved" );
            }
            else
            {
                System.out.println( "given initial-object name unknown at server side" );
            }
        }
        catch( com.sun.star.connection.NoConnectException e )
        {
            System.out.println( "Couldn't connect to remote server" );
            System.out.println( e.getMessage() );
        }
        catch( com.sun.star.connection.ConnectionSetupException e )
        {
            System.out.println( "Couldn't access necessary local resource to establish the interprocess connection" );
            System.out.println( e.getMessage() );
        }
        catch( com.sun.star.lang.IllegalArgumentException e )
        {
            System.out.println( "uno-url is syntactical illegal ( " + args[0] + " )" );
            System.out.println( e.getMessage() );
        }
        catch( com.sun.star.uno.RuntimeException e )
        {
            System.out.println( "RuntimeException" );
            System.out.println( e.getMessage() );
        }
        finally {
            System.exit(0);
        }
    }
}
