/*************************************************************************
 *
 *  $RCSfile: Bootstrap.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 15:15:37 $
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

package com.sun.star.comp.helper;

import com.sun.star.beans.XPropertySet;
import com.sun.star.bridge.UnoUrlResolver;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.comp.loader.JavaLoader;
import com.sun.star.container.XSet;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.util.NativeLibraryLoader;
import com.sun.star.loader.XImplementationLoader;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

import java.io.File;
import java.util.Enumeration;
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
        XSet xSet = (XSet)UnoRuntime.queryInterface( XSet.class, aMultiComponentFactory );
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
    static public XComponentContext createInitialComponentContext( Hashtable context_entries )
        throws Exception
    {
        XImplementationLoader xImpLoader = (XImplementationLoader)UnoRuntime.queryInterface(
            XImplementationLoader.class, new JavaLoader() );

        // Get the factory of the ServiceManager
        XSingleComponentFactory smgr_fac = (XSingleComponentFactory)UnoRuntime.queryInterface(
            XSingleComponentFactory.class, xImpLoader.activate(
                "com.sun.star.comp.servicemanager.ServiceManager", null, null, null ) );

        // Create an instance of the ServiceManager
        XMultiComponentFactory xSMgr = (XMultiComponentFactory)UnoRuntime.queryInterface(
            XMultiComponentFactory.class, smgr_fac.createInstanceWithContext( null ) );

        // post init loader
        XInitialization xInit = (XInitialization)UnoRuntime.queryInterface(
            XInitialization.class, xImpLoader );
        Object[] args = new Object [] { xSMgr };
        xInit.initialize( args );

        // initial component context
        if (context_entries == null)
            context_entries = new Hashtable( 1 );
        // add smgr
        context_entries.put(
            "/singletons/com.sun.star.lang.theServiceManager",
            new ComponentContextEntry( null, xSMgr ) );
        // ... xxx todo: add standard entries
        XComponentContext xContext = new ComponentContext( context_entries, null );

        // post init smgr
        xInit = (XInitialization)UnoRuntime.queryInterface(
            XInitialization.class, xSMgr );
        args = new Object [] { null, xContext }; // no registry, default context
        xInit.initialize( args );

        XSet xSet = (XSet)UnoRuntime.queryInterface( XSet.class, xSMgr );
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
     * @since       UDK2.08
     */
    static public XMultiServiceFactory createSimpleServiceManager() throws Exception
    {
        return (XMultiServiceFactory)UnoRuntime.queryInterface(
            XMultiServiceFactory.class, createInitialComponentContext( null ).getServiceManager() );
    }


    /** Bootstraps the initial component context from a native UNO installation.

        @see cppuhelper/defaultBootstrap_InitialComponentContext()
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

        @see cppuhelper/defaultBootstrap_InitialComponentContext()
    */
    static public final XComponentContext defaultBootstrap_InitialComponentContext(
        String ini_file, Hashtable bootstrap_parameters )
        throws Exception
    {
        // jni convenience: easier to iterate over array than calling Hashtable
        String pairs [] = null;
        if (null != bootstrap_parameters)
        {
            pairs = new String [ 2 * bootstrap_parameters.size() ];
            Enumeration enum = bootstrap_parameters.keys();
            int n = 0;
            while (enum.hasMoreElements())
            {
                String name = (String)enum.nextElement();
                pairs[ n++ ] = name;
                pairs[ n++ ] = (String)bootstrap_parameters.get( name );
            }
        }

        if (! m_loaded_juh)
        {
            System.loadLibrary( "juh" );
            m_loaded_juh = true;
        }
        return (XComponentContext)UnoRuntime.queryInterface(
            XComponentContext.class, cppuhelper_bootstrap( ini_file, pairs ) );
    }

    static private boolean m_loaded_juh = false;
    static private native final Object cppuhelper_bootstrap(
        String ini_file, String bootstrap_parameters [] )
        throws Exception;

    /**
     * Bootstraps the component context from a UNO installation.
     *
     * @return a bootstrapped component context.
     */
    public static final XComponentContext bootstrap()
        throws BootstrapException {

        final String SOFFICE =
            System.getProperty( "os.name" ).startsWith( "Windows" ) ?
            "soffice.exe" : "soffice";
        final String NOLOGO = "-nologo";
        final String NODEFAULT = "-nodefault";
        final String PIPENAME =
            "uno" + Integer.toString( (new Random()).nextInt() & 0xffff );
        final String URL =
            "uno:pipe,name=" + PIPENAME + ";urp;StarOffice.ServiceManager";
        final String CONNECTION =
            "-accept=pipe,name=" + PIPENAME + ";urp;StarOffice.ServiceManager";

        final long SLEEPMILLIS = 500;

        XComponentContext xContext = null;

        try {
            // create default local component context
            XComponentContext xLocalContext =
                createInitialComponentContext( null );

            // initial service manager
            XMultiComponentFactory xLocalServiceManager =
                xLocalContext.getServiceManager();

            // create a URL resolver
            XUnoUrlResolver xUrlResolver =
                UnoUrlResolver.create( xLocalContext );

            // try to connect to office
            Object remoteServiceManager = null;
            try {
                remoteServiceManager = xUrlResolver.resolve( URL );
            }
            catch ( com.sun.star.connection.NoConnectException e ) {
                // find office executable relative to this class's class loader
                File fOffice = NativeLibraryLoader.getResource(
                    Bootstrap.class.getClassLoader(), SOFFICE );

                if ( fOffice != null ) {
                    // create call with arguments
                    String[] cmdArray = new String[4];
                    cmdArray[0] = fOffice.getPath();
                    cmdArray[1] = NOLOGO;
                    cmdArray[2] = NODEFAULT;
                    cmdArray[3] = CONNECTION;

                    // start office process
                    Runtime.getRuntime().exec( cmdArray );

                    // wait until office is started
                    while ( remoteServiceManager == null ) {
                        try {
                            // try to connect to office
                            Thread.currentThread().sleep( SLEEPMILLIS );
                            remoteServiceManager = xUrlResolver.resolve( URL );
                        } catch ( com.sun.star.connection.NoConnectException ex ) {
                            // try to connect again
                        }
                    }
                } else {
                    throw new BootstrapException(
                        "no office executable found!" );
                }
            }

            // XComponentContext
            if ( remoteServiceManager != null ) {
                XPropertySet xPropertySet =
                    (XPropertySet)UnoRuntime.queryInterface(
                    XPropertySet.class, remoteServiceManager );
                Object context =
                    xPropertySet.getPropertyValue( "DefaultContext" );
                xContext = (XComponentContext) UnoRuntime.queryInterface(
                    XComponentContext.class, context);
            }
        } catch ( java.lang.RuntimeException e ) {
            throw e;
        } catch ( java.lang.Exception e ) {
            throw new BootstrapException( e );
        }

        return xContext;
    }
}
