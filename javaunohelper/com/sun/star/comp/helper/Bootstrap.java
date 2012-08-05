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

package com.sun.star.comp.helper;

import com.sun.star.bridge.UnoUrlResolver;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.comp.loader.JavaLoader;
import com.sun.star.container.XSet;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.util.NativeLibraryLoader;
import com.sun.star.loader.XImplementationLoader;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.util.Iterator;
import java.util.Hashtable;
import java.util.Random;

/** Bootstrap offers functionality to obtain a context or simply
    a service manager.
    The service manager can create a few basic services, whose implementations  are:
    <ul>
    <li>com.sun.star.comp.loader.JavaLoader</li>
    <li>com.sun.star.comp.urlresolver.UrlResolver</li>
    <li>com.sun.star.comp.bridgefactory.BridgeFactory</li>
    <li>com.sun.star.comp.connections.Connector</li>
    <li>com.sun.star.comp.connections.Acceptor</li>
    <li>com.sun.star.comp.servicemanager.ServiceManager</li>
    </ul>

    Other services can be inserted into the service manager by
    using its XSet interface:
    <pre>
        XSet xSet = UnoRuntime.queryInterface( XSet.class, aMultiComponentFactory );
        // insert the service manager
        xSet.insert( aSingleComponentFactory );
    </pre>
*/
public class Bootstrap {

    private static void insertBasicFactories(
        XSet xSet, XImplementationLoader xImpLoader )
        throws Exception
    {
        // insert the factory of the loader
        xSet.insert( xImpLoader.activate(
            "com.sun.star.comp.loader.JavaLoader", null, null, null ) );

        // insert the factory of the URLResolver
        xSet.insert( xImpLoader.activate(
            "com.sun.star.comp.urlresolver.UrlResolver", null, null, null ) );

        // insert the bridgefactory
        xSet.insert( xImpLoader.activate(
            "com.sun.star.comp.bridgefactory.BridgeFactory", null, null, null ) );

        // insert the connector
        xSet.insert( xImpLoader.activate(
            "com.sun.star.comp.connections.Connector", null, null, null ) );

        // insert the acceptor
        xSet.insert( xImpLoader.activate(
            "com.sun.star.comp.connections.Acceptor", null, null, null ) );
    }

    /** Bootstraps an initial component context with service manager and basic
        jurt components inserted.
        @param context_entries the hash table contains mappings of entry names (type string) to
        context entries (type class ComponentContextEntry).
        @return a new context.
    */
    static public XComponentContext createInitialComponentContext( Hashtable<String, Object> context_entries )
        throws Exception
    {
        XImplementationLoader xImpLoader = UnoRuntime.queryInterface(
            XImplementationLoader.class, new JavaLoader() );

        // Get the factory of the ServiceManager
        XSingleComponentFactory smgr_fac = UnoRuntime.queryInterface(
            XSingleComponentFactory.class, xImpLoader.activate(
                "com.sun.star.comp.servicemanager.ServiceManager", null, null, null ) );

        // Create an instance of the ServiceManager
        XMultiComponentFactory xSMgr = UnoRuntime.queryInterface(
            XMultiComponentFactory.class, smgr_fac.createInstanceWithContext( null ) );

        // post init loader
        XInitialization xInit = UnoRuntime.queryInterface(
            XInitialization.class, xImpLoader );
        Object[] args = new Object [] { xSMgr };
        xInit.initialize( args );

        // initial component context
        if (context_entries == null)
            context_entries = new Hashtable<String,Object>( 1 );
        // add smgr
        context_entries.put(
            "/singletons/com.sun.star.lang.theServiceManager",
            new ComponentContextEntry( null, xSMgr ) );
        // ... xxx todo: add standard entries
        XComponentContext xContext = new ComponentContext( context_entries, null );

        // post init smgr
        xInit = UnoRuntime.queryInterface(
            XInitialization.class, xSMgr );
        args = new Object [] { null, xContext }; // no registry, default context
        xInit.initialize( args );

        XSet xSet = UnoRuntime.queryInterface( XSet.class, xSMgr );
        // insert the service manager
        xSet.insert( smgr_fac );
        // and basic jurt factories
        insertBasicFactories( xSet, xImpLoader );

        return xContext;
    }

    /**
     * Bootstraps a servicemanager with the jurt base components registered.
     * <p>
     * @return     a freshly boostrapped service manager
     * @see        com.sun.star.lang.ServiceManager
     */
    static public XMultiServiceFactory createSimpleServiceManager() throws Exception
    {
        return UnoRuntime.queryInterface(
            XMultiServiceFactory.class, createInitialComponentContext( null ).getServiceManager() );
    }


    /** Bootstraps the initial component context from a native UNO installation.

        @see "cppuhelper/defaultBootstrap_InitialComponentContext()"
    */
    static public final XComponentContext defaultBootstrap_InitialComponentContext()
        throws Exception
    {
        return defaultBootstrap_InitialComponentContext( null, null );
    }
    /** Bootstraps the initial component context from a native UNO installation.

        @param ini_file
               ini_file (may be null: uno.rc besides cppuhelper lib)
        @param bootstrap_parameters
               bootstrap parameters (maybe null)

        @see "cppuhelper/defaultBootstrap_InitialComponentContext()"
    */
    static public final XComponentContext defaultBootstrap_InitialComponentContext(
        String ini_file, Hashtable<String,String> bootstrap_parameters )
        throws Exception
    {
        // jni convenience: easier to iterate over array than calling Hashtable
        String pairs [] = null;
        if (null != bootstrap_parameters)
        {
            pairs = new String [ 2 * bootstrap_parameters.size() ];
            Iterator<String> keys = bootstrap_parameters.keySet().iterator();
            int n = 0;
            while (keys.hasNext())
            {
                String name = keys.next();
                pairs[ n++ ] = name;
                pairs[ n++ ] = bootstrap_parameters.get( name );
            }
        }

        if (! m_loaded_juh)
        {
            NativeLibraryLoader.loadLibrary( Bootstrap.class.getClassLoader(), "juh" );
            m_loaded_juh = true;
        }
        return UnoRuntime.queryInterface(
            XComponentContext.class,
            cppuhelper_bootstrap(
                ini_file, pairs, Bootstrap.class.getClassLoader() ) );
    }

    static private boolean m_loaded_juh = false;
    static private native Object cppuhelper_bootstrap(
        String ini_file, String bootstrap_parameters [], ClassLoader loader )
        throws Exception;

    /**
     * Bootstraps the component context from a UNO installation.
     *
     * @return a bootstrapped component context.
     *
     * @since UDK 3.1.0
     */
    public static final XComponentContext bootstrap()
        throws BootstrapException {

        XComponentContext xContext = null;

        try {
            // create default local component context
            XComponentContext xLocalContext =
                createInitialComponentContext( null );
            if ( xLocalContext == null )
                throw new BootstrapException( "no local component context!" );

            // find office executable relative to this class's class loader
            String sOffice =
                System.getProperty( "os.name" ).startsWith( "Windows" ) ?
                "soffice.exe" : "soffice";
            File fOffice = NativeLibraryLoader.getResource(
                Bootstrap.class.getClassLoader(), sOffice );
            if ( fOffice == null )
                throw new BootstrapException( "no office executable found!" );

            // create random pipe name
            String sPipeName = "uno" +
                Long.toString( (new Random()).nextLong() & 0x7fffffffffffffffL );

            // create call with arguments
            String[] cmdArray = new String[7];
            cmdArray[0] = fOffice.getPath();
            cmdArray[1] = "--nologo";
            cmdArray[2] = "--nodefault";
            cmdArray[3] = "--norestore";
            cmdArray[4] = "--nocrashreport";
            cmdArray[5] = "--nolockcheck";
            cmdArray[6] = "--accept=pipe,name=" + sPipeName + ";urp;";

            // start office process
            Process p = Runtime.getRuntime().exec( cmdArray );
            pipe( p.getInputStream(), System.out, "CO> " );
            pipe( p.getErrorStream(), System.err, "CE> " );

            // initial service manager
            XMultiComponentFactory xLocalServiceManager =
                xLocalContext.getServiceManager();
            if ( xLocalServiceManager == null )
                throw new BootstrapException( "no initial service manager!" );

            // create a URL resolver
            XUnoUrlResolver xUrlResolver =
                UnoUrlResolver.create( xLocalContext );

            // connection string
            String sConnect = "uno:pipe,name=" + sPipeName +
                ";urp;StarOffice.ComponentContext";

            // wait until office is started
            for (int i = 0;; ++i) {
                try {
                    // try to connect to office
                    Object context = xUrlResolver.resolve( sConnect );
                    xContext = UnoRuntime.queryInterface(
                        XComponentContext.class, context);
                    if ( xContext == null )
                        throw new BootstrapException( "no component context!" );
                    break;
                } catch ( com.sun.star.connection.NoConnectException ex ) {
                    // Wait 500 ms, then try to connect again, but do not wait
                    // longer than 5 min (= 600 * 500 ms) total:
                    if (i == 600) {
                        throw new BootstrapException(ex.toString());
                    }
                    Thread.sleep( 500 );
                }
            }
        } catch ( BootstrapException e ) {
            throw e;
        } catch ( java.lang.RuntimeException e ) {
            throw e;
        } catch ( java.lang.Exception e ) {
            throw new BootstrapException( e );
        }

        return xContext;
    }

    private static void pipe(
        final InputStream in, final PrintStream out, final String prefix ) {

        new Thread( "Pipe: " + prefix) {
            public void run() {
                BufferedReader r = new BufferedReader(
                    new InputStreamReader( in ) );
                try {
                    for ( ; ; ) {
                        String s = r.readLine();
                        if ( s == null ) {
                            break;
                        }
                        out.println( prefix + s );
                    }
                } catch ( java.io.IOException e ) {
                    e.printStackTrace( System.err );
                }
            }
        }.start();
    }
}
