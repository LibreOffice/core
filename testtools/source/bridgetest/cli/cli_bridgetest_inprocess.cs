/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

using System;
using System.Collections;
using uno;
using uno.util;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.container;



internal class Factory :
    WeakComponentBase, XSingleComponentFactory, XServiceInfo
{
    private String m_service;
    private Type m_type;
    private System.Reflection.ConstructorInfo m_ctor;
    
    public Factory( Type type, String service )
    {
        m_service = service;
        m_type = type;
        m_ctor = type.GetConstructor(
            new Type [] { typeof (XComponentContext) } );
    }
    
    public Object createInstanceWithContext( XComponentContext xContext )
    {
        return m_ctor.Invoke( new Object [] { xContext } );
    }
    
    public Object createInstanceWithArgumentsAndContext(
        uno.Any [] args, XComponentContext xContext )
    {
        return m_ctor.Invoke( new Object [] { xContext } );
    }

    public bool supportsService( String name )
    {
        return m_service.Equals( name );
    }

    public String [] getSupportedServiceNames()
    {
        return new String [] { m_service };
    }

    public String getImplementationName()
    {
        return m_type.ToString();
    }
}


/** This executable does the same as the batch file starting via uno.exe,
    but via bootstrapping native UNO.
*/
public class BridgeTest
{
    public static int Main( String [] args )
    {
//       System.Diagnostics.Debugger.Launch();
        try
		{
			string bootstrap_ini = "cli_bridgetest_inprocess.ini";
			if (args.Length > 0)
			{
				if (args[0] == "/?")
				{
					Console.WriteLine(
						"\n\ncli_bridgetest_inprocess [bootstrap file] \n\n"
						+ "bootstrap file \n"
						+ "\t contains the entries UNO_TYPES and UNO_SERVICES.\n"
						+ "\t If a file is not provided than it is assumed that a\n" 
						+ "\t cli_bridgetest_inprocess.ini file can be found in the\n " 
						+ "\t current working directory.\n"
						);
					return 0;
				}
				else
				{
					bootstrap_ini = args[0];
				}
			}

            // bootstrap native UNO
            XComponentContext xContext =
                Bootstrap.defaultBootstrap_InitialComponentContext(
                    bootstrap_ini, null );
        
            using (new uno.util.DisposeGuard( (XComponent) xContext ))
            {
                XSet xSet = (XSet) xContext.getServiceManager();
                xSet.insert(
                    new uno.Any(
                        typeof (XSingleComponentFactory),
                        new Factory(
                            typeof (cs_testobj.BridgeTestObject),
                            "com.sun.star.test.bridge.cli_uno.CsTestObject" ) ) );
                xSet.insert(
                    new uno.Any(
                        typeof (XSingleComponentFactory),
                        new Factory(
                            typeof (vb_testobj.VBBridgeTestObject),
                            "com.sun.star.test.bridge.cli_uno.VbTestObject" ) ) );
                xSet.insert(
                    new uno.Any(
                        typeof (XSingleComponentFactory),
                        new Factory(
                            typeof (cpp_bridgetest.BridgeTest),
                            "com.sun.star.test.bridge.cli_uno.CppBridgeTest" ) ) );
                xSet.insert(
                    new uno.Any(
                        typeof (XSingleComponentFactory),
                        new Factory(
                            typeof (cs_testobj.BridgeTest),
                            "com.sun.star.test.bridge.cli_uno.CsBridgeTest" ) ) );
                xSet.insert(
                    new uno.Any(
                        typeof (XSingleComponentFactory),
                        new Factory(
                            typeof (vb_bridetest.BridgeTest),
                            "com.sun.star.test.bridge.cli_uno.VbBridgeTest" ) ) );
            
                // I.
                // direct unbridged test
                // get client object via singleton entry
                Object test_client;
                XMain xClient;
                test_client = new cs_testobj.BridgeTest( xContext );
                xClient = (XMain) test_client;
                Console.WriteLine(
                    "\n[cli bridgetest] 1. C# client calls C# object");
                // run with CLI target object
                xClient.run(
                    new String [] {
                    "com.sun.star.test.bridge.cli_uno.CsTestObject" } );
                
                // II:
                // uno -ro uno_services.rdb -ro uno_types.rdb
                //     -s com.sun.star.test.bridge.BridgeTest
                //     -- com.sun.star.test.bridge.cli_uno.TestObject
            
                // get native client
                test_client =
                    xContext.getServiceManager().createInstanceWithContext(
                        "com.sun.star.test.bridge.BridgeTest", xContext );
                xClient = (XMain) test_client;
                Console.WriteLine(
                    "\n[cli bridgetest] 2. C++ client (native) calls C# object");
                // run with CLI target object
                xClient.run(
                    new String [] {
                    "com.sun.star.test.bridge.cli_uno.CsTestObject",
					"noCurrentContext"} );
            
                // III:        
                // uno -ro uno_services.rdb -ro uno_types.rdb
                //     -s com.sun.star.test.bridge.cli_uno.BridgeTest
                //     -- com.sun.star.test.bridge.CppTestObject
            
                // get CLI client
                test_client =
                    xContext.getServiceManager().createInstanceWithContext(
                        "com.sun.star.test.bridge.cli_uno.CsBridgeTest",
                        xContext );
                xClient = (XMain) test_client;
                Console.WriteLine(
                    "\n[cli bridgetest] 3. C# client calls C++ object (native)");
                // run with native target object
                xClient.run(
                    new String [] { "com.sun.star.test.bridge.CppTestObject" } );

                // IV:        
                // uno -ro uno_services.rdb -ro uno_types.rdb
                //     -s com.sun.star.test.bridge.cli_uno.VbBridgeTest
                //     -- com.sun.star.test.bridge.CppTestObject
                // get CLI client
                test_client =
                    xContext.getServiceManager().createInstanceWithContext(
                        "com.sun.star.test.bridge.cli_uno.VbBridgeTest",
                        xContext );
                xClient = (XMain) test_client;
                Console.WriteLine(
                    "\n[cli bridgetest] 4. Visual Basic client calls C++ (native) object" );
                // run with native target object
                xClient.run(
                    new String [] { "com.sun.star.test.bridge.CppTestObject" } );

                // V:
                // uno -ro uno_services.rdb -ro uno_types.rdb
                //     -s com.sun.star.test.bridge.BridgeTest
                //     -- com.sun.star.test.bridge.cli_uno.VbTestObject
                // get CLI client
//                 test_client =
//                     xContext.getServiceManager().createInstanceWithContext(
//                         "com.sun.star.test.bridge.BridgeTest", xContext );
//                 xClient = (XMain) test_client;
//                 Console.WriteLine(
//                     "[cli bridgetest] Visual Basic client: {0}",
//                     xClient.ToString() );
//                 // run with native target object
//                 xClient.run(
//                     new String [] {
//                     "com.sun.star.test.bridge.cli_uno.VbTestObject" } );

                // VI:
                // uno -ro uno_services.rdb -ro uno_types.rdb 
                // -s com.sun.star.test.bridge.cli_uno.CppBridgeTest 
                // -- com.sun.star.test.bridge.CppTestObject
                test_client =
                    xContext.getServiceManager().createInstanceWithContext(
                        "com.sun.star.test.bridge.cli_uno.CppBridgeTest",
                        xContext );
                xClient = (XMain) test_client;
                Console.WriteLine(
                    "\n[cli bridgetest] 6. CLI C++ client calls C++ object (native)");
                // run with native target object
                xClient.run(
                    new String [] { "com.sun.star.test.bridge.CppTestObject" } );
            }
        }
        catch (System.Exception exc)
        {
            GC.WaitForPendingFinalizers();
            System.Console.WriteLine( exc );
            return -1;
        }
        
        GC.WaitForPendingFinalizers();
        System.Console.WriteLine( "====> all tests ok." );
        return 0;
    }
}
