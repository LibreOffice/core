/*************************************************************************
 *
 *  $RCSfile: cli_bridgetest_inprocess.cs,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-06 12:59:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

using System;
using System.Collections;
using uno;
using uno.util;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.lang;
using unoidl.com.sun.star.container;


//==============================================================================
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
        try
        {
            // bootstrap native UNO
            XComponentContext xContext =
                Bootstrap.defaultBootstrap_InitialComponentContext(
                    "cli_bridgetest_inprocess.ini", null );
        
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
                Object test_client = new cs_testobj.BridgeTest( xContext );
                XMain xClient = (XMain) test_client;
                Console.WriteLine(
                    "[cli bridgetest] client: {0}", xClient.ToString() );
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
                    "[cli bridgetest] client: {0}", xClient.ToString() );
                // run with CLI target object
                xClient.run(
                    new String [] {
                    "com.sun.star.test.bridge.cli_uno.CsTestObject" } );
            
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
                    "[cli bridgetest] client: {0}", xClient.ToString() );
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
                    "[cli bridgetest] Visual Basic client: {0}", xClient.ToString() );
                // run with native target object
                xClient.run(
                    new String [] { "com.sun.star.test.bridge.CppTestObject" } );

                // V:
                // uno -ro uno_services.rdb -ro uno_types.rdb
                //     -s com.sun.star.test.bridge.BridgeTest
                //     -- com.sun.star.test.bridge.cli_uno.VbTestObject
                // get CLI client
                test_client =
                    xContext.getServiceManager().createInstanceWithContext(
                        "com.sun.star.test.bridge.BridgeTest", xContext );
                xClient = (XMain) test_client;
                Console.WriteLine(
                    "[cli bridgetest] Visual Basic client: {0}",
                    xClient.ToString() );
                // run with native target object
                xClient.run(
                    new String [] {
                    "com.sun.star.test.bridge.cli_uno.VbTestObject" } );

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
                    "[cli bridgetest] Visual Basic client: {0}",
                    xClient.ToString() );
                // run with native target object
                xClient.run(
                    new String [] { "com.sun.star.test.bridge.CppTestObject" } );
            }
        }
        catch (System.Exception exc)
        {
            GC.WaitForPendingFinalizers();
            System.Console.WriteLine( exc );
            return 1;
        }
        
        GC.WaitForPendingFinalizers();
        System.Console.WriteLine( "====> all tests ok." );
        return 0;
    }
}
