/*************************************************************************
 *
 *  $RCSfile: Helper.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:21:22 $
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

// __________ Imports __________

// base classes
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;

// factory for creating components
import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.frame.XComponentLoader;

// property access
import com.sun.star.beans.*;

public class Helper
{
    // __________ static helper methods __________

    /** Connect to a running office that is accepting connections
        and return the ServiceManager to instantiate office components
     */
    static public XMultiServiceFactory connect( String sConnection )
        throws Exception
    {
        XMultiServiceFactory xMultiServiceFactory = null;
        XMultiServiceFactory xLocalServiceManager =
            com.sun.star.comp.helper.Bootstrap.createSimpleServiceManager();

        XUnoUrlResolver aURLResolver = (XUnoUrlResolver)UnoRuntime.queryInterface(
            XUnoUrlResolver.class,
            xLocalServiceManager.createInstance( "com.sun.star.bridge.UnoUrlResolver" ) );

        xMultiServiceFactory = (XMultiServiceFactory)UnoRuntime.queryInterface(
            XMultiServiceFactory.class,
            aURLResolver.resolve( sConnection ) );
        if ( xMultiServiceFactory == null )
            throw new Exception( "couldn't not connect to:'" + sConnection + "'" );
        return xMultiServiceFactory;
    }

    /** creates and instantiates new document
    */
    static public XComponent createDocument( XMultiServiceFactory xMultiServiceFactory,
        String sURL, String sTargetFrame, int nSearchFlags, PropertyValue[] aArgs )
            throws Exception
    {
        XComponent xComponent = null;
        XComponentLoader aLoader = (XComponentLoader)UnoRuntime.queryInterface(
            XComponentLoader.class,
                xMultiServiceFactory.createInstance( "com.sun.star.frame.Desktop" ) );

        xComponent = (XComponent)UnoRuntime.queryInterface( XComponent.class,
            aLoader.loadComponentFromURL(
                sURL, sTargetFrame, nSearchFlags, aArgs ) );
        if ( xComponent == null )
            throw new Exception( "could not create document: " + sURL );
        return xComponent;
    }
}
