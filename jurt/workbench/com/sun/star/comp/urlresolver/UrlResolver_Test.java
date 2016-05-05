/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

package com.sun.star.comp.urlresolver;

import com.sun.star.uno.XComponentContext;
import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.bridge.UnoUrlResolver;
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.UnoRuntime;

/** start the office with these options <br>
    soffice --accept=socket,host=localhost,port=8100;urp;
*/
public class UrlResolver_Test
{
    public static void main(String[] args) {
        try {
            XComponentContext xcomponentcontext = Bootstrap.createInitialComponentContext( null );

            // initial serviceManager
            xcomponentcontext.getServiceManager();

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
            xOfficeFactory.createInstanceWithContext("com.sun.star.frame.Desktop",
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
