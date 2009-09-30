/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UrlResolver_Test.java,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package com.sun.star.comp.urlresolver;

import com.sun.star.uno.XComponentContext;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.bridge.UnoUrlResolver;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.UnoRuntime;
//import com.sun.star.connection.NoConnectionException;

/** start the office with these options <br>
    soffice -accept=socket,host=localhost,port=8100;urp;
*/
public class UrlResolver_Test
{
    public static void main(String[] args) {
        try {
            XComponentContext xcomponentcontext = Bootstrap.createInitialComponentContext( null );

            // initial serviceManager
            XMultiComponentFactory xLocalServiceManager = xcomponentcontext.getServiceManager();

            // create a connector, so that it can contact the office
            XUnoUrlResolver urlResolver
                = UnoUrlResolver.create( xcomponentcontext );

            Object initialObject = urlResolver.resolve(
                "uno:socket,host=localhost,port=8100;urp;StarOffice.ServiceManager" );

            XMultiComponentFactory  xOfficeFactory= UnoRuntime.queryInterface(
                XMultiComponentFactory.class, initialObject );

            // retrieve the component context (it's not yet exported from the office)
            // Query for the XPropertySet interface.
            XPropertySet xProperySet = UnoRuntime.queryInterface(
                XPropertySet.class, xOfficeFactory);

            // Get the default context from the office server.
            Object oDefaultContext = xProperySet.getPropertyValue( "DefaultContext" );

            // Query for the interface XComponentContext.
            XComponentContext xOfficeComponentContext = UnoRuntime.queryInterface(
                XComponentContext.class, oDefaultContext );

            // now create the desktop service
            // NOTE: use the office component context here !
            Object oDesktop = xOfficeFactory.createInstanceWithContext("com.sun.star.frame.Desktop",
                                                                       xOfficeComponentContext );
        } catch(com.sun.star.connection.NoConnectException e) {
            System.out.println(e.getMessage());
            e.printStackTrace();
        } catch(com.sun.star.connection.ConnectionSetupException ce) {
            System.out.println(ce.getMessage());
            ce.printStackTrace();
        } catch(com.sun.star.lang.IllegalArgumentException ie) {
            System.out.println(ie.getMessage());
            ie.printStackTrace();
        } catch(com.sun.star.beans.UnknownPropertyException ue) {
            System.out.println(ue.getMessage());
            ue.printStackTrace();
        } catch(java.lang.Exception ee) {
            System.out.println(ee.getMessage());
            ee.printStackTrace();
        }

        }
}
