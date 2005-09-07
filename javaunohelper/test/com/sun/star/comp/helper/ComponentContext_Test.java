/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ComponentContext_Test.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:44:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


