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


import com.sun.star.uno.XComponentContext;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.test.XSomethingB;
import com.sun.star.test.SomethingB;
import com.sun.star.lang.XSingleComponentFactory;
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
        XSet set = (XSet ) UnoRuntime.queryInterface( XSet.class, serviceManager );
        set.insert( singleFactory );
    }

    public static void removeFromServiceManager(
        XMultiComponentFactory serviceManager, Object singleFactory )
        throws com.sun.star.uno.Exception
    {
        XSet set = (XSet ) UnoRuntime.queryInterface( XSet.class, serviceManager );
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
            e.printStackTrace(System.out);
        }

        // quit, even when a remote bridge is running
        System.exit(0);
    }
}
