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

import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiComponentFactory;

import com.sun.star.comp.helper.ComponentContext;
import com.sun.star.comp.helper.ComponentContextEntry;
import com.sun.star.uno.UnoRuntime;

import java.util.Hashtable;


public class ComponentContext_Test {
    public static void main(String args[]) {
        try {
            Hashtable<String,Object> table = new Hashtable<String,Object>();
            table.put( "bla1", new ComponentContextEntry( null, new Integer( 1 ) ) );
            XComponentContext xInitialContext = Bootstrap.createInitialComponentContext( table );

            table = new Hashtable<String,Object>();
            table.put( "bla2", new ComponentContextEntry( new Integer( 2 ) ) );
            table.put( "bla3", new Integer( 3 ) );
            XComponentContext xContext = new ComponentContext( table, xInitialContext );

            XMultiComponentFactory xSMgr = xContext.getServiceManager();
            Object o = xSMgr.createInstanceWithContext( "com.sun.star.loader.Java", xContext );
            if (o == null)
                System.err.println( "### failed raising service: 1!" );
            o = xSMgr.createInstanceWithContext( "com.sun.star.bridge.BridgeFactory", xContext );
            if (o == null)
                System.err.println( "### failed raising service: 2!" );
            o = xSMgr.createInstanceWithContext( "com.sun.star.bridge.UnoUrlResolver", xContext );
            if (o == null)
                System.err.println( "### failed raising service: 3!" );
            o = xSMgr.createInstanceWithContext( "com.sun.star.connection.Connector", xContext );
            if (o == null)
                System.err.println( "### failed raising service: 4!" );
            o = xSMgr.createInstanceWithContext( "com.sun.star.connection.Acceptor", xContext );
            if (o == null)
                System.err.println( "### failed raising service: 5!" );
            o = xSMgr.createInstanceWithContext( "com.sun.star.lang.ServiceManager", xContext );
            if (o == null)
                System.err.println( "### failed raising service: 6!" );

            if (xContext.getValueByName( "bla1" ) == null ||
                xContext.getValueByName( "bla2" ) == null ||
                xContext.getValueByName( "bla3" ) == null ||
                xInitialContext.getValueByName( "bla2" ) != null ||
                xInitialContext.getValueByName( "bla3" ) != null)
            {
                System.err.println( "### bootstrap context test failed: 1!" );
            }
            if (((Integer)xContext.getValueByName( "bla1" )).intValue() != 1 ||
                ((Integer)xContext.getValueByName( "bla2" )).intValue() != 2 ||
                ((Integer)xContext.getValueByName( "bla3" )).intValue() != 3 ||
                ((Integer)xInitialContext.getValueByName( "bla1" )).intValue() != 1)
            {
                System.err.println( "### bootstrap context test failed: 2!" );
            }

            XComponent xComp = UnoRuntime.queryInterface(
                XComponent.class, xInitialContext );
            xComp.dispose();
        }
        catch(Exception exception) {
            System.err.println("exception occurred:" + exception);
            exception.printStackTrace();
        }
    }
}


