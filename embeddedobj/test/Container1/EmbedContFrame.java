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

package embeddedobj.test;

import java.awt.*;
import java.awt.event.*;

import com.sun.star.comp.servicemanager.ServiceManager;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.connection.XConnector;
import com.sun.star.connection.XConnection;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.XNamingService;
import com.sun.star.uno.XComponentContext;

import com.sun.star.container.*;
import com.sun.star.beans.*;
import com.sun.star.lang.*;


public class EmbedContFrame extends Frame
{
    private EmbedContApp m_aApp;

    WindowListener m_aCloser = new WindowAdapter()
    {
        public void windowClosing( WindowEvent e )
        {
            if ( m_aApp != null )
            {
                m_aApp.disposeObject();
                m_aApp = null;
            }

            dispose();
            System.exit( 0 );
        }
    };

    public EmbedContFrame( String sName )
    {
        super( sName );
        addWindowListener( m_aCloser );
    }

    public static void start()
    {
        EmbedContFrame aFrame = new EmbedContFrame( "Testing container." );

        // connect to the office
        XMultiServiceFactory aServiceFactory = null;
        try {
            aServiceFactory = connectOfficeGetServiceFactory();
        }
        catch( Exception e )
        {}

        if ( aServiceFactory == null )
        {
            System.out.println( "Can't get service manager!\n" );
            System.exit( 1 );
        }

        aFrame.m_aApp = new EmbedContApp( aFrame, aServiceFactory );
        aFrame.m_aApp.init();
        aFrame.m_aApp.start();

        Dimension aSize = aFrame.m_aApp.getSize();

        aFrame.add( "Center", aFrame.m_aApp );
        aFrame.pack();
        aFrame.setSize( aSize );

        aFrame.setVisible( true );
    }

    public static void main( String args[] )
    {
        EmbedContFrame.start();
    }

    public static XMultiServiceFactory connectOfficeGetServiceFactory()
    throws com.sun.star.uno.Exception,
    com.sun.star.uno.RuntimeException,
    Exception
    {
        String sConnectionString = "uno:socket,host=localhost,port=8100;urp;StarOffice.NamingService";

        // Get component context
        XComponentContext xComponentContext =
            com.sun.star.comp.helper.Bootstrap.createInitialComponentContext( null );

        // initial serviceManager
        XMultiComponentFactory xLocalServiceManager = xComponentContext.getServiceManager();

        // create a connector, so that it can contact the office
        Object  oUrlResolver  = xLocalServiceManager.createInstanceWithContext( "com.sun.star.bridge.UnoUrlResolver",
                                                                                xComponentContext );
        XUnoUrlResolver xUrlResolver = (XUnoUrlResolver)UnoRuntime.queryInterface( XUnoUrlResolver.class, oUrlResolver );

        Object oInitialObject = xUrlResolver.resolve( sConnectionString );
        XNamingService xName = (XNamingService)UnoRuntime.queryInterface( XNamingService.class, oInitialObject );

        XMultiServiceFactory xMSF = null;
        if( xName != null ) {
            Object oMSF = xName.getRegisteredObject( "StarOffice.ServiceManager" );
            xMSF = (XMultiServiceFactory)UnoRuntime.queryInterface( XMultiServiceFactory.class, oMSF );
        }
        else
            System.out.println( "Error: Can't get XNamingService interface from url resolver!" );

        return xMSF;
    }
}

