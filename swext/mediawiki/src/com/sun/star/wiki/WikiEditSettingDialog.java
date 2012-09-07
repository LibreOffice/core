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

package com.sun.star.wiki;

import com.sun.star.awt.XDialog;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.EventObject;
import java.util.Hashtable;
import javax.net.ssl.SSLException;

import org.apache.commons.httpclient.*;
import org.apache.commons.httpclient.methods.*;

public class WikiEditSettingDialog extends WikiDialog
{

    private final String sOKMethod = "OK";

    String[] Methods =
    {sOKMethod };
    private Hashtable setting;
    private boolean addMode;
    private boolean m_bAllowURLChange = true;

    public WikiEditSettingDialog( XComponentContext xContext, String DialogURL )
    {
        super( xContext, DialogURL );
        super.setMethods( Methods );
        setting = new Hashtable();
        addMode = true;

        InsertThrobber( 184, 20, 10, 10 );
        InitStrings( xContext );
        InitSaveCheckbox( xContext, false );
    }

    public WikiEditSettingDialog( XComponentContext xContext, String DialogURL, Hashtable ht, boolean bAllowURLChange )
    {
        super( xContext, DialogURL );
        super.setMethods( Methods );
        setting = ht;

        boolean bInitSaveCheckBox = false;

        try
        {
            XPropertySet xUrlField = GetPropSet( "UrlField" );

            xUrlField.setPropertyValue( "Text", ht.get( "Url" ) );

            GetPropSet( "UsernameField" ).setPropertyValue( "Text", ht.get( "Username" ) );

            if ( Helper.PasswordStoringIsAllowed( m_xContext ) )
            {
                String[] pPasswords = Helper.GetPasswordsForURLAndUser( m_xContext, (String)ht.get( "Url" ), (String)ht.get( "Username" ) );
                bInitSaveCheckBox = ( pPasswords != null && pPasswords.length > 0 && pPasswords[0].equals( (String)ht.get( "Password" ) ) );
            }

            // the password should be entered by the user or the Cancel should be pressed
            // GetPropSet( "PasswordField" ).setPropertyValue( "Text", ht.get( "Password" ));
        }
        catch ( Exception ex )
        {
            ex.printStackTrace();
        }

        addMode = false;
        m_bAllowURLChange = bAllowURLChange;

        InsertThrobber( 184, 20, 10, 10 );
        InitStrings( xContext );
        InitSaveCheckbox( xContext, bInitSaveCheckBox );
    }

    public boolean show( )
    {
        SetThrobberVisible( false );
        EnableControls( true );
        boolean bResult = super.show();

        try
        {
            if ( bResult && Helper.PasswordStoringIsAllowed( m_xContext )
              && ( (Short)( GetPropSet( "SaveBox" ).getPropertyValue("State") ) ).shortValue() != (short)0 )
            {
                String sURL = (String)setting.get( "Url" );
                String sUserName = (String)setting.get( "Username" );
                String sPassword = (String)setting.get( "Password" );

                if ( sURL != null && sURL.length() > 0 && sUserName != null && sUserName.length() > 0 && sPassword != null && sPassword.length() > 0 )
                {
                    String[] pPasswords = { sPassword };
                    Helper.GetPasswordContainer( m_xContext ).addPersistent( sURL, sUserName, pPasswords, Helper.GetInteractionHandler( m_xContext ) );
                }
            }
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }

        return bResult;
    }

    public void EnableControls( boolean bEnable )
    {
        if ( !bEnable )
            SetFocusTo( "CancelButton" );

        try
        {
            GetPropSet( "UsernameField" ).setPropertyValue( "Enabled", new Boolean( bEnable ) );
            GetPropSet( "PasswordField" ).setPropertyValue( "Enabled", new Boolean( bEnable ) );
            GetPropSet( "OkButton" ).setPropertyValue( "Enabled", new Boolean( bEnable ) );
            GetPropSet( "HelpButton" ).setPropertyValue( "Enabled", new Boolean( bEnable ) );

            if ( bEnable )
            {
                GetPropSet( "UrlField" ).setPropertyValue( "Enabled", new Boolean( m_bAllowURLChange ) );
                GetPropSet( "SaveBox" ).setPropertyValue( "Enabled", new Boolean( Helper.PasswordStoringIsAllowed( m_xContext ) ) );
                if ( m_bAllowURLChange )
                    SetFocusTo( "UrlField" );
                else
                    SetFocusTo( "UsernameField" );
            }
            else
            {
                GetPropSet( "UrlField" ).setPropertyValue( "Enabled", Boolean.FALSE );
                GetPropSet( "SaveBox" ).setPropertyValue( "Enabled", Boolean.FALSE );
            }
        }
        catch ( Exception ex )
        {
            ex.printStackTrace();
        }
    }

    private void InitStrings( XComponentContext xContext )
    {
        try
        {
            SetTitle( Helper.GetLocalizedString( xContext, Helper.DLG_MEDIAWIKI_TITLE ) );
            GetPropSet( "UrlLabel" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_EDITSETTING_URLLABEL ) );
            GetPropSet( "UsernameLabel" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_EDITSETTING_USERNAMELABEL ) );
            GetPropSet( "PasswordLabel" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_EDITSETTING_PASSWORDLABEL ) );
            GetPropSet( "AccountLine" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_EDITSETTING_ACCOUNTLINE ) );
            GetPropSet( "WikiLine" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_EDITSETTING_WIKILINE ) );
            GetPropSet( "SaveBox" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_EDITSETTING_SAVEBOX ) );
            GetPropSet( "OkButton" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_OK ) );
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }
    }

    private void InitSaveCheckbox( XComponentContext xContext, boolean bInitSaveCheckBox )
    {
        XPropertySet xSaveCheck = GetPropSet( "SaveBox" );
        try
        {
            xSaveCheck.setPropertyValue( "State", new Short( bInitSaveCheckBox ? (short)1 : (short)0 ) );
            xSaveCheck.setPropertyValue( "Enabled", new Boolean( Helper.PasswordStoringIsAllowed( xContext ) ) );
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }
    }

    public void DoLogin( XDialog xDialog )
    {
        String sRedirectURL = "";
        String sURL = "";
        try
        {
            sURL = ( String ) GetPropSet( "UrlField" ).getPropertyValue( "Text" );
            String sUserName = ( String ) GetPropSet( "UsernameField" ).getPropertyValue( "Text" );
            String sPassword = ( String ) GetPropSet( "PasswordField" ).getPropertyValue( "Text" );

            HostConfiguration aHostConfig = new HostConfiguration();
            boolean bInitHost = true;
            boolean bAllowIndex = true;

            do
            {
                if ( sRedirectURL.length() > 0 )
                {
                    sURL = sRedirectURL;
                    sRedirectURL = "";
                }

                if ( sURL.length() > 0 )
                {
                    URI aURI = new URI( sURL );
                    GetMethod aRequest = new GetMethod( aURI.getEscapedPathQuery() );
                    aRequest.setFollowRedirects( false );
                    Helper.ExecuteMethod( aRequest, aHostConfig, aURI, m_xContext, bInitHost );
                    bInitHost = false;

                    int nResultCode = aRequest.getStatusCode();
                    String sWebPage = null;
                    if ( nResultCode == 200 )
                        sWebPage = aRequest.getResponseBodyAsString();
                    else if ( nResultCode >= 301 && nResultCode <= 303 || nResultCode == 307 )
                        sRedirectURL = aRequest.getResponseHeader( "Location" ).getValue();

                    aRequest.releaseConnection();

                    if ( sWebPage != null && sWebPage.length() > 0 )
                    {
                        //the URL is valid
                        String sMainURL = Helper.GetMainURL( sWebPage, sURL );

                        if ( sMainURL.equals( "" ) )
                        {
                            // TODO:
                            // it's not a Wiki Page, check first whether a redirect is requested
                            // happens usually in case of https
                            sRedirectURL = Helper.GetRedirectURL( sWebPage, sURL );
                            if ( sRedirectURL.equals( "" ) )
                            {
                                // show error
                                Helper.ShowError( m_xContext,
                                                  m_xDialog,
                                                  Helper.DLG_MEDIAWIKI_TITLE,
                                                  Helper.NOURLCONNECTION_ERROR,
                                                  sURL,
                                                  false );
                            }
                        }
                        else
                        {
                            URI aMainURI = new URI( sMainURL, true ); // it must be an escaped URL, otherwise an exception should be thrown

                            if ( ( sUserName.length() > 0 || sPassword.length() > 0 )
                              && Helper.Login( aMainURI, sUserName, sPassword, m_xContext ) == null )
                            {
                                // a wrong login information is provided
                                // show error
                                Helper.ShowError( m_xContext,
                                                  m_xDialog,
                                                  Helper.DLG_MEDIAWIKI_TITLE,
                                                  Helper.WRONGLOGIN_ERROR,
                                                  null,
                                                  false );
                            }
                            else
                            {
                                setting.put( "Url", aMainURI.getEscapedURI() );
                                setting.put( "Username", sUserName );
                                setting.put( "Password", sPassword );
                                if ( addMode )
                                {
                                    // no cleaning of the settings is necessary
                                    Settings.getSettings( m_xContext ).addWikiCon( setting );
                                    Settings.getSettings( m_xContext ).storeConfiguration();
                                }

                                m_bAction = true;
                            }
                        }
                    }
                    else if ( sRedirectURL == null || sRedirectURL.length() == 0 )
                    {
                        if ( sURL.length() > 0 && !sURL.endsWith( "index.php" ) && bAllowIndex )
                        {
                            // the used MainURL is not alwais directly accessible
                            // add the suffix as workaround, but only once
                            sRedirectURL = sURL + "/index.php";
                            bAllowIndex = false;
                        }
                        else
                        {
                            // URL invalid
                            // show error
                            Helper.ShowError( m_xContext,
                                              m_xDialog,
                                              Helper.DLG_MEDIAWIKI_TITLE,
                                              Helper.INVALIDURL_ERROR,
                                              null,
                                              false );
                        }
                    }
                }
                else
                {
                    // URL field empty
                    // show error
                    Helper.ShowError( m_xContext,
                                      m_xDialog,
                                      Helper.DLG_MEDIAWIKI_TITLE,
                                      Helper.NOURL_ERROR,
                                      null,
                                      false );
                }
            } while ( sRedirectURL.length() > 0 );
        }
        catch ( WikiCancelException ce )
        {
        }
        catch ( SSLException essl )
        {
            if ( Helper.IsConnectionAllowed() )
            {
                Helper.ShowError( m_xContext,
                                  m_xDialog,
                                  Helper.DLG_MEDIAWIKI_TITLE,
                                  Helper.UNKNOWNCERT_ERROR,
                                  null,
                                  false );
            }
            essl.printStackTrace();
        }
        catch ( Exception ex )
        {
            if ( Helper.IsConnectionAllowed() )
            {
                Helper.ShowError( m_xContext,
                                  m_xDialog,
                                  Helper.DLG_MEDIAWIKI_TITLE,
                                  Helper.NOURLCONNECTION_ERROR,
                                  sURL,
                                  false );
            }
            ex.printStackTrace();
        }
    }

    public boolean callHandlerMethod( XDialog xDialog, Object EventObject, String MethodName )
    {
        if ( MethodName.equals( sOKMethod ) )
        {
            EnableControls( false );
            SetThrobberVisible( true );
            SetThrobberActive( true );

            if ( Helper.AllowThreadUsage( m_xContext ) )
            {
                final XDialog xDialogForThread = xDialog;
                final XComponentContext xContext = m_xContext;
                final WikiEditSettingDialog aThis = this;

                // the thread name is used to allow the error dialogs
                m_bThreadFinished = false;
                m_aThread = new Thread( "com.sun.star.thread.WikiEditorSendingThread" )
                {
                    public void run()
                    {
                        try
                        {
                            Thread.yield();
                        } catch( java.lang.Exception e ){}

                        DoLogin( xDialogForThread );
                        aThis.EnableControls( true );
                        aThis.SetThrobberActive( false );
                        aThis.SetThrobberVisible( false );

                        ThreadStop( true );

                        if ( m_bAction )
                            MainThreadDialogExecutor.Close( xContext, xDialogForThread );
                    }
                };

                m_aThread.start();
            }
            else
            {
                try
                {
                    DoLogin( xDialog );
                } catch( java.lang.Exception e )
                {}
                finally
                {
                    EnableControls( true );
                    SetThrobberActive( false );
                    SetThrobberVisible( false );

                    if ( m_bAction )
                        xDialog.endExecute();

                    Helper.AllowConnection( true );
                }
            }

            return true;
        }

        return false;
    }

    public void windowClosed( EventObject e )
    {
        ThreadStop( false );
    }
}

