/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UrlResolver_Test.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:19:12 $
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

            XMultiComponentFactory  xOfficeFactory= (XMultiComponentFactory) UnoRuntime.queryInterface(
                XMultiComponentFactory.class, initialObject );

            // retrieve the component context (it's not yet exported from the office)
            // Query for the XPropertySet interface.
            XPropertySet xProperySet = ( XPropertySet ) UnoRuntime.queryInterface(
                XPropertySet.class, xOfficeFactory);

            // Get the default context from the office server.
            Object oDefaultContext = xProperySet.getPropertyValue( "DefaultContext" );

            // Query for the interface XComponentContext.
            XComponentContext xOfficeComponentContext = ( XComponentContext ) UnoRuntime.queryInterface(
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
