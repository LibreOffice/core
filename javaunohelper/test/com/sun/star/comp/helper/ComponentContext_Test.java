/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ComponentContext_Test.java,v $
 * $Revision: 1.3 $
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
            Hashtable table = new Hashtable();
            table.put( "bla1", new ComponentContextEntry( null, new Integer( 1 ) ) );
            XComponentContext xInitialContext = Bootstrap.createInitialComponentContext( table );

            table = new Hashtable();
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

            XComponent xComp = (XComponent)UnoRuntime.queryInterface(
                XComponent.class, xInitialContext );
            xComp.dispose();
        }
        catch(Exception exception) {
            System.err.println("exception occurred:" + exception);
            exception.printStackTrace();
        }
    }
}


