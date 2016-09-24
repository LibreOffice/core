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
import com.sun.star.uno.XComponentContext;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.test.XSomethingB;
import com.sun.star.test.SomethingB;
import com.sun.star.container.XSet;

// sample starbasic code, you can execute it after you have connected to the office.
//  Sub Main
//      o = createUnoService( "com.sun.star.test.SomethingB" )
//      msgbox o.methodOne( "from the office !" )
//  End Sub

public class TestJavaComponent
{

    public static void insertIntoServiceManager(
        XMultiComponentFactory serviceManager, Object singleFactory )
        throws com.sun.star.uno.Exception
    {
        XSet set = UnoRuntime.queryInterface( XSet.class, serviceManager );
        set.insert( singleFactory );
    }

    public static void removeFromServiceManager(
        XMultiComponentFactory serviceManager, Object singleFactory )
        throws com.sun.star.uno.Exception
    {
        XSet set = UnoRuntime.queryInterface( XSet.class, serviceManager );
        set.remove( singleFactory );

    }

    public static void main(String[] args) throws java.lang.Exception
    {
        try {
            boolean bLocal = false;

            XMultiComponentFactory xUsedServiceManager = null;
            XComponentContext xUsedComponentContext = null;

            if( args.length == 1 && args[0].equals( "local" ))
            {
                XComponentContext xLocalComponentContext =
                    Bootstrap.createInitialComponentContext( null );

                // initial serviceManager
                XMultiComponentFactory xLocalServiceManager =
                    xLocalComponentContext.getServiceManager();

                bLocal = true;
                xUsedServiceManager = xLocalServiceManager;
                xUsedComponentContext = xLocalComponentContext;

                System.out.println( "Using local servicemanager" );
            } else {
                // get the remote office component context
                xUsedComponentContext =
                    com.sun.star.comp.helper.Bootstrap.bootstrap();
                System.out.println("Connected to a running office ...");

                xUsedServiceManager = xUsedComponentContext.getServiceManager();
                System.out.println( "Using remote servicemanager" );
            }

            if ( xUsedServiceManager == null )
            {
                System.out.println( "ERROR: no service manager" );
                System.exit(0);
            }

            Object factory = new Object();
            if ( bLocal )
            {
                // retrieve the factory for the component implementation
                factory = TestServiceProvider.__getServiceFactory(
                    "TestComponentB", null, null);

                // insert the factory into the local servicemanager
                // From now on, the service can be instantiated !
                insertIntoServiceManager( xUsedServiceManager, factory );
            }

            XSomethingB xSomethingB = SomethingB.create(xUsedComponentContext);

            // and call the test method.
            String s= xSomethingB.methodTwo("Hello World!");
            System.out.println(s);

            if ( bLocal )
            {
                // remove it again from the servicemanager,
                removeFromServiceManager( xUsedServiceManager, factory );
            }

        }
        catch ( Exception e )
        {
            System.out.println( "UNO Exception caught: " + e );
            System.out.println( "Message: " + e.getMessage() );
            e.printStackTrace(System.err);
        }

        // quit, even when a remote bridge is running
        System.exit(0);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
