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

package com.sun.star.wizards.db;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.sdbc.SQLException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.NamedValueCollection;
import com.sun.star.wizards.common.Properties;
import com.sun.star.wizards.ui.WizardDialog;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * is a base class for a wizard creating a database object
 */
public abstract class DatabaseObjectWizard extends WizardDialog
{
    protected final PropertyValue[]     m_wizardContext;
    protected final XDatabaseDocumentUI m_docUI;
    protected final XFrame              m_frame;

    protected DatabaseObjectWizard( final XMultiServiceFactory i_orb, final int i_helpIDBase, final PropertyValue[] i_wizardContext )
    {
        super( i_orb, i_helpIDBase );
        m_wizardContext = i_wizardContext;

        final NamedValueCollection wizardContext = new NamedValueCollection( m_wizardContext );
        m_docUI = wizardContext.queryOrDefault( "DocumentUI", (XDatabaseDocumentUI)null, XDatabaseDocumentUI.class );

        if ( m_docUI != null )
        {
            XController docController = UnoRuntime.queryInterface( XController.class, m_docUI );
            m_frame = docController.getFrame();
        }
        else
        {
            XFrame parentFrame = wizardContext.queryOrDefault( "ParentFrame", (XFrame)null, XFrame.class );
            if ( parentFrame != null )
                m_frame = parentFrame;
            else
                m_frame = Desktop.getActiveFrame( xMSF );
        }
    }

    protected final void loadSubComponent( final int i_type, final String i_name, final boolean i_forEditing )
    {
        try
        {
            if ( m_docUI != null )
                m_docUI.loadComponent( i_type, i_name, i_forEditing );
        }
        catch ( IllegalArgumentException ex )
        {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, null, ex );
        }
        catch ( NoSuchElementException ex )
        {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, null, ex );
        }
        catch ( SQLException ex )
        {
            Logger.getLogger( this.getClass().getName() ).log( Level.SEVERE, null, ex );
        }
    }

    public interface WizardFromCommandLineStarter
    {
        void start(XMultiServiceFactory factory, PropertyValue[] curproperties);
    }

    protected static void executeWizardFromCommandLine( final String i_args[], WizardFromCommandLineStarter starter )
    {
        final String settings[] = new String[] { null, null, null };
        final int IDX_PIPE_NAME = 0;
        final int IDX_LOCATION = 1;
        final int IDX_DSN = 2;

        // some simple parsing
        boolean failure = false;
        int settingsIndex = -1;
        for ( int i=0; i<i_args.length; ++i )
        {
            if ( settingsIndex >= 0 )
            {
                settings[ settingsIndex ] = i_args[i];
                settingsIndex = -1;
                continue;
            }

            if ( i_args[i].equals( "--pipe-name" ) )
            {
                settingsIndex = IDX_PIPE_NAME;
                continue;
            }

            if ( i_args[i].equals( "--database-location" ) )
            {
                settingsIndex = IDX_LOCATION;
                continue;
            }

            if ( i_args[i].equals( "--data-source-name" ) )
            {
                settingsIndex = IDX_DSN;
                continue;
            }

            failure = true;
        }

        if ( settings[ IDX_PIPE_NAME ] == null )
            failure = true;

        if ( ( settings[ IDX_DSN ] == null ) && ( settings[ IDX_LOCATION ] == null ) )
            failure = true;

        if ( failure )
        {
            System.err.println( "supported arguments: " );
            System.err.println( "  --pipe-name <name>           : specifies the name of the pipe to connect to the running OOo instance" );
            System.err.println( "  --database-location <url>    : specifies the URL of the database document to work with" );
            System.err.println( "  --data-source-name <name>    : specifies the name of the data source to work with" );
            return;
        }

        final String ConnectStr = "uno:pipe,name=" + settings[IDX_PIPE_NAME] + ";urp;StarOffice.ServiceManager";
        try
        {
            final XMultiServiceFactory serviceFactory = Desktop.connect(ConnectStr);
            if (serviceFactory != null)
            {
                PropertyValue[] curproperties = new PropertyValue[1];
                if ( settings[ IDX_LOCATION ] != null )
                    curproperties[0] = Properties.createProperty( "DatabaseLocation", settings[ IDX_LOCATION ] );
                else
                    curproperties[0] = Properties.createProperty( "DataSourceName", settings[ IDX_DSN ] );

                starter.start(serviceFactory, curproperties);
            }
        }
        catch (java.lang.Exception jexception)
        {
            jexception.printStackTrace(System.err);
        }
    }
}
