/*************************************************************************
 *
 *  $RCSfile: Helper.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-06-30 15:23:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

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
