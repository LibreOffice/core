/*************************************************************************
 *
 *  $RCSfile: Bootstrap.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2001-06-14 11:58:23 $
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


import com.sun.star.comp.loader.JavaLoader;

import com.sun.star.container.XSet;

import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XSingleComponentFactory;

import com.sun.star.loader.XImplementationLoader;

import com.sun.star.uno.UnoRuntime;

import java.util.Hashtable;


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
            "com.sun.star.lang.ServiceManager",
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
}
