/*************************************************************************
 *
 *  $RCSfile: Bootstrap.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kr $ $Date: 2001-02-13 18:05:38 $
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

import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.loader.XImplementationLoader;

import com.sun.star.uno.UnoRuntime;


public class Bootstrap {
    /**
     * Bootstraps a servicemanager with the jurt base components registered.
     * <p>
     * @return     a freshly boostrapped service manager
     * @see        com.sun.star.lang.ServiceManager
     * @since       UDK2.08
     */
    static public XMultiServiceFactory createSimpleServiceManager() throws Exception {
        JavaLoader loader = new JavaLoader();

        XImplementationLoader xImpLoader = (XImplementationLoader)UnoRuntime.queryInterface(XImplementationLoader.class, loader);

        // Get the factory for the ServiceManager
        Object loaderobj = xImpLoader.activate("com.sun.star.comp.servicemanager.ServiceManager", null, null, null);

        // Ensure that we have got a factory
        XSingleServiceFactory xManagerFac = (XSingleServiceFactory) UnoRuntime.queryInterface(XSingleServiceFactory.class, loaderobj);
        // Create an instance of the ServiceManager
        XMultiServiceFactory xMultiFac = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class,
                                                                                          xManagerFac.createInstance());

        // set the ServiceManager at the JavaLoader with the XInitialization interface
        XInitialization xInit = (XInitialization) UnoRuntime.queryInterface(XInitialization.class, xImpLoader);
        Object[] iniargs = { xMultiFac };
        xInit.initialize( iniargs );


        // now use the XSet interface at the ServiceManager to add the factory of the loader
        XSet xSet = (XSet) UnoRuntime.queryInterface(XSet.class, xMultiFac);

        // Get the factory of the loader
        XSingleServiceFactory xSingleServiceFactory = (XSingleServiceFactory) UnoRuntime.queryInterface(XSingleServiceFactory.class,
                                                                                                        xImpLoader.activate("com.sun.star.comp.loader.JavaLoader", null, null, null));

        // add the javaloader
        xSet.insert(xSingleServiceFactory);

        // add the service manager
        xSet.insert(xManagerFac);

        // Get the factory of the URLResolver
        xSingleServiceFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(XSingleServiceFactory.class,
                                                                                 xImpLoader.activate("com.sun.star.comp.urlresolver.UrlResolver", null, null, null));
        xSet.insert(xSingleServiceFactory);

        // add the bridgefactory
        xSingleServiceFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(XSingleServiceFactory.class,
                                                                                 xImpLoader.activate("com.sun.star.comp.bridgefactory.BridgeFactory", null, null, null));
        xSet.insert(xSingleServiceFactory);

        // add the connector
        xSingleServiceFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(XSingleServiceFactory.class,
                                                                                 xImpLoader.activate("com.sun.star.comp.connections.Connector", null, null, null));
        xSet.insert(xSingleServiceFactory);

        // add the acceptor
        xSingleServiceFactory = (XSingleServiceFactory)UnoRuntime.queryInterface(XSingleServiceFactory.class,
                                                                                 xImpLoader.activate("com.sun.star.comp.connections.Acceptor", null, null, null));
        xSet.insert(xSingleServiceFactory);

        return xMultiFac;
    }
}
