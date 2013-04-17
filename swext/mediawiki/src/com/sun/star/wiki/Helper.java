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

import com.sun.star.awt.MessageBoxButtons;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XMessageBox;
import com.sun.star.awt.XMessageBoxFactory;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.NamedValue;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XContainerQuery;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.document.XDocumentPropertiesSupplier;
import com.sun.star.document.XDocumentProperties;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XModuleManager;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XSeekable;
import com.sun.star.io.XStream;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.system.SystemShellExecuteFlags;
import com.sun.star.system.XSystemShellExecute;
import com.sun.star.task.UrlRecord;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.task.XMasterPasswordHandling;
import com.sun.star.task.XPasswordContainer;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.XChangesBatch;
import java.net.*;
import java.io.*;
import javax.net.ssl.SSLException;
import javax.swing.text.html.HTMLEditorKit;

import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HostConfiguration;
import org.apache.commons.httpclient.HttpMethodBase;
import org.apache.commons.httpclient.cookie.CookiePolicy;
import org.apache.commons.httpclient.MultiThreadedHttpConnectionManager;
import org.apache.commons.httpclient.protocol.Protocol;
import org.apache.commons.httpclient.URI;
import org.apache.commons.httpclient.methods.GetMethod;
import org.apache.commons.httpclient.methods.PostMethod;

public class Helper
{
    public final static int GENERALSEND_ERROR = 0;
    public final static int NOWIKIFILTER_ERROR = 1;
    public final static int NOURLCONNECTION_ERROR = 2;
    public final static int WRONGLOGIN_ERROR = 3;
    public final static int INVALIDURL_ERROR = 4;
    public final static int NOURL_ERROR = 5;

    public final static int DLG_SENDTITLE = 6;
    public final static int DLG_WIKIARTICLE = 7;
    public final static int DLG_NO = 8;
    public final static int DLG_OK = 9;
    public final static int DLG_YES = 10;
    // 11 is reserved
    public final static int DLG_ADDBUTTON = 12;
    public final static int DLG_EDITBUTTON = 13;
    public final static int DLG_SENDBUTTON = 14;
    public final static int DLG_REMOVEBUTTON = 15;

    public final static int DLG_EDITSETTING_URLLABEL = 16;
    public final static int DLG_EDITSETTING_USERNAMELABEL = 17;
    public final static int DLG_EDITSETTING_PASSWORDLABEL = 18;
    public final static int DLG_NEWWIKIPAGE_LABEL1 = 19;
    public final static int DLG_SENDTOMEDIAWIKI_LABEL1 = 20;
    public final static int DLG_SENDTOMEDIAWIKI_LABEL2 = 21;
    public final static int DLG_SENDTOMEDIAWIKI_LABEL3 = 22;
    public final static int DLG_SENDTOMEDIAWIKI_MINORCHECK = 23;
    public final static int DLG_SENDTOMEDIAWIKI_BROWSERCHECK = 24;
    public final static int UNKNOWNCERT_ERROR = 25;
    public final static int DLG_MEDIAWIKI_TITLE = 26;
    public final static int DLG_EDITSETTING_ACCOUNTLINE = 27;
    public final static int DLG_EDITSETTING_WIKILINE = 28;
    public final static int DLG_EDITSETTING_SAVEBOX = 29;
    public final static int CANCELSENDING_ERROR = 30;
    public final static int DLG_MEDIAWIKIEXTENSION_STRING = 31;
    public final static int DLG_WIKIPAGEEXISTS_LABEL1 = 32;

    public final static int STRINGS_NUM = 33;

    private final static String[] m_pEntryNames = { "GeneralSendError",
                                                    "NoWikiFilter",
                                                    "NoConnectionToURL",
                                                    "WrongLogin",
                                                    "InvalidURL",
                                                    "NoURL",
                                                    "Dlg_SendTitle",
                                                    "Dlg_WikiArticle",
                                                    "Dlg_No",
                                                    "Dlg_OK",
                                                    "Dlg_Yes",
                                                    null, // reserved
                                                    "Dlg_AddButton",
                                                    "Dlg_EditButton",
                                                    "Dlg_SendButton",
                                                    "Dlg_RemoveButton",
                                                    "Dlg_EditSetting_UrlLabel",
                                                    "Dlg_EditSetting_UsernameLabel",
                                                    "Dlg_EditSetting_PasswordLabel",
                                                    "Dlg_NewWikiPage_Label1",
                                                    "Dlg_SendToMediaWiki_Label1",
                                                    "Dlg_SendToMediaWiki_Label2",
                                                    "Dlg_SendToMediaWiki_Label3",
                                                    "Dlg_SendToMediaWiki_MinorCheck",
                                                    "Dlg_SendToMediaWiki_BrowserCheck",
                                                    "UnknownCert",
                                                    "Dlg_MediaWiki_Title",
                                                    "Dlg_EditSetting_AccountLine",
                                                    "Dlg_EditSetting_WikiLine",
                                                    "Dlg_EditSetting_SaveBox",
                                                    "CancelSending",
                                                    "Dlg_MediaWiki_Extension_String",
                                                    "Dlg_WikiPageExists_Label1" };

    private static String[] m_pConfigStrings;

    private static final String sHTMLHeader = "<HTML><HEAD><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><TITLE></TITLE></HEAD><BODY>";
    private static final String sHTMLFooter = "</BODY></HTML>";

    private static MultiThreadedHttpConnectionManager m_aConnectionManager;
    private static HttpClient m_aClient;
    private static boolean m_bAllowConnection = true;

    private static Boolean m_bShowInBrowser = null;

    private static XPasswordContainer m_xPasswordContainer;
    private static XInteractionHandler m_xInteractionHandler;

    synchronized protected static String GetLocalizedString( XComponentContext xContext, int nID )
        throws com.sun.star.uno.Exception
    {
        if ( nID >= STRINGS_NUM )
            throw new com.sun.star.uno.RuntimeException();

        if ( m_pConfigStrings == null )
        {
            XNameAccess xNameAccess = GetConfigNameAccess( xContext, "org.openoffice.Office.Custom.WikiExtension/Strings" );

            String[] pStrings = new String[STRINGS_NUM];
            for ( int nInd = 0; nInd < STRINGS_NUM; nInd++ )
                if ( m_pEntryNames[nInd] != null )
                    pStrings[nInd] = AnyConverter.toString( xNameAccess.getByName( m_pEntryNames[nInd] ) );
                else
                    pStrings[nInd] = "";

            m_pConfigStrings = pStrings;
        }

        return m_pConfigStrings[nID];
    }

    synchronized protected static HttpClient GetHttpClient()
        throws WikiCancelException
    {
        if ( !m_bAllowConnection )
            throw new WikiCancelException();

        if ( m_aConnectionManager == null )
            m_aConnectionManager = new MultiThreadedHttpConnectionManager();

        if ( m_aClient == null )
        {
            m_aClient = new HttpClient( m_aConnectionManager );
            m_aClient.getParams().setParameter( "http.protocol.cookie-policy", CookiePolicy.BROWSER_COMPATIBILITY );
            m_aClient.getParams().setParameter( "http.protocol.single-cookie-header", Boolean.TRUE );
            m_aClient.getParams().setParameter( "http.protocol.content-charset", "UTF-8" );
        }

        return m_aClient;
    }

    synchronized protected static void AllowConnection( boolean bAllow )
    {
        m_bAllowConnection = bAllow;
        if ( !bAllow && m_aConnectionManager != null )
        {
            m_aClient = null;
            m_aConnectionManager.shutdown();
            m_aConnectionManager = null;
        }
    }

    synchronized protected static boolean IsConnectionAllowed()
    {
        return m_bAllowConnection;
    }

    synchronized protected static boolean GetShowInBrowserByDefault( XComponentContext xContext )
    {
        if ( m_bShowInBrowser == null )
        {
            try
            {
                XNameAccess xAccess = Helper.GetConfigNameAccess( xContext, "org.openoffice.Office.Custom.WikiExtension/Settings" );
                m_bShowInBrowser = new Boolean( AnyConverter.toBoolean( xAccess.getByName( "PreselectShowBrowser" ) ) );
            }
            catch( com.sun.star.uno.Exception e )
            {
                e.printStackTrace();
            }
        }

        return m_bShowInBrowser.booleanValue();
    }

    synchronized protected static void SetShowInBrowserByDefault( XComponentContext xContext, boolean bValue )
    {
        try
        {
            m_bShowInBrowser = new Boolean( bValue );

            XPropertySet xProps = Helper.GetConfigProps( xContext, "org.openoffice.Office.Custom.WikiExtension/Settings" );
            xProps.setPropertyValue( "PreselectShowBrowser", new Boolean( bValue ) );
            XChangesBatch xBatch = UnoRuntime.queryInterface( XChangesBatch.class, xProps );
            if ( xBatch != null )
                xBatch.commitChanges();
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }
    }

    synchronized protected static XPasswordContainer GetPasswordContainer( XComponentContext xContext )
        throws com.sun.star.uno.Exception
    {
        if ( m_xPasswordContainer == null && xContext != null )
        {
            XMultiComponentFactory xFactory = xContext.getServiceManager();
            if ( xFactory != null )
                m_xPasswordContainer = UnoRuntime.queryInterface(
                                        XPasswordContainer.class,
                                        xFactory.createInstanceWithContext( "com.sun.star.task.PasswordContainer", xContext ) );
        }

        if ( m_xPasswordContainer == null )
            throw new com.sun.star.uno.RuntimeException();

        return m_xPasswordContainer;
    }

    synchronized protected static XInteractionHandler GetInteractionHandler( XComponentContext xContext )
        throws com.sun.star.uno.Exception
    {
        if ( m_xInteractionHandler == null && xContext != null )
        {
            XMultiComponentFactory xFactory = xContext.getServiceManager();
            if ( xFactory != null )
                m_xInteractionHandler = UnoRuntime.queryInterface(
                                        XInteractionHandler.class,
                                        xFactory.createInstanceWithContext( "com.sun.star.task.InteractionHandler", xContext ) );
        }

        if ( m_xInteractionHandler == null )
            throw new com.sun.star.uno.RuntimeException();

        return m_xInteractionHandler;
    }

    protected static Protocol GetOwnHttps( int nPort )
    {
        return new Protocol( "https", new WikiProtocolSocketFactory(), ( ( nPort < 0 ) ? 443 : nPort ) );
    }

    protected static String GetMainURL( String sWebPage, String sVURL )
    {
        String sResultURL = "";
        try
        {
            StringReader aReader = new StringReader( sWebPage );
            HTMLEditorKit.Parser aParser = GetHTMLParser();
            EditPageParser aCallback = new EditPageParser();

            aParser.parse( aReader, aCallback, true );
            sResultURL = aCallback.m_sMainURL;

            if ( !sResultURL.startsWith( "http" ) )
            {
                //if the url is only relative then complete it
                URL aURL = new URL( sVURL );
                sResultURL = aURL.getProtocol() + "://" + aURL.getHost() + sResultURL;
            }
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }

        if ( sResultURL.length() == 0 )
        {
            // usually that should not happen
            // workaround: try to get index.php from the provided URL
            int nIndex = sVURL.indexOf( "index.php" );
            if ( nIndex >= 0 )
                sResultURL = sVURL.substring( 0, nIndex );
        }

        return sResultURL;
    }

    protected static String GetRedirectURL( String sWebPage, String sURL )
    {
        //scrape the HTML source and find the EditURL
        // TODO/LATER: Use parser in future

        String sResultURL = "";
        int nInd = sWebPage.indexOf( "http-equiv=\"refresh\"" );
        if ( nInd != -1 )
        {
            int nContent = sWebPage.indexOf( "content=", nInd );
            if ( nContent > 0 )
            {
                int nURL = sWebPage.indexOf( "URL=", nContent );
                if ( nURL > 0 )
                {
                    int nEndURL = sWebPage.indexOf( "\"", nURL );
                    if ( nEndURL > 0 )
                        sResultURL = sWebPage.substring( nURL + 4, nEndURL );
                }
            }

            try
            {
                URL aURL = new URL( sURL );
                if ( !sResultURL.startsWith( aURL.getProtocol() ))
                {
                    //if the url is only relative then complete it
                    if ( sResultURL.startsWith( "/" ) )
                        sResultURL = aURL.getProtocol() + "://" + aURL.getHost() + sResultURL;
                    else
                        sResultURL = aURL.getProtocol() + "://" + aURL.getHost() + aURL.getPath() + sResultURL;
                }
            }
            catch ( MalformedURLException ex )
            {
                ex.printStackTrace();
            }
        }

        return sResultURL;

    }

    protected static XInputStream SaveHTMLTemp( XComponentContext xContext, String sArticle )
    {
        XInputStream xResult = null;

        if ( xContext != null )
        {
            try
            {
                Object oTempFile = xContext.getServiceManager().createInstanceWithContext( "com.sun.star.io.TempFile", xContext );
                XStream xStream = UnoRuntime.queryInterface( XStream.class, oTempFile );
                XSeekable xSeekable = UnoRuntime.queryInterface( XSeekable.class, oTempFile );
                if ( xStream != null && xSeekable != null )
                {
                    XOutputStream xOutputStream = xStream.getOutputStream();
                    XInputStream xInputStream = xStream.getInputStream();
                    if ( xOutputStream != null && xInputStream != null )
                    {
                        String sHTML = sHTMLHeader.concat( sArticle );
                        sHTML = sHTML.concat( sHTMLFooter );
                        xOutputStream.writeBytes( sHTML.getBytes( "UTF-8" ) );
                        // xOutputStream.closeOutput();
                        xSeekable.seek( 0 );

                        xResult = xInputStream;
                    }
                }
            }
            catch ( Exception ex )
            {
                ex.printStackTrace();
            }
        }

        return xResult;
    }


    protected static String CreateTempFile( XComponentContext xContext )
    {
        String sURL = "";
        try
        {
            Object oTempFile = xContext.getServiceManager().createInstanceWithContext( "com.sun.star.io.TempFile", xContext );
            XPropertySet xPropertySet = UnoRuntime.queryInterface( XPropertySet.class, oTempFile );
            xPropertySet.setPropertyValue( "RemoveFile", Boolean.FALSE );
            sURL = ( String ) xPropertySet.getPropertyValue( "Uri" );

            XInputStream xInputStream = UnoRuntime.queryInterface( XInputStream.class, oTempFile );
            xInputStream.closeInput();
            XOutputStream xOutputStream = UnoRuntime.queryInterface( XOutputStream.class, oTempFile );
            xOutputStream.closeOutput();
        } catch ( com.sun.star.uno.Exception ex )
        {
            ex.printStackTrace();
        }
        return sURL;
    }

    protected static String EachLine( String sURL )
    {
        String sText = "";
        try
        {
            URL aURL = new URL( sURL );
            File aFile = new File( aURL.getFile() );
            InputStreamReader aInputReader = new InputStreamReader( new FileInputStream( aFile ), "UTF-8" );
            BufferedReader aBufReader = new BufferedReader( aInputReader );

            StringBuffer aBuf = new StringBuffer();
            String sEachLine = aBufReader.readLine();

            while( sEachLine != null )
            {
                aBuf.append( sEachLine );
                aBuf.append( "\n" );

                sEachLine = aBufReader.readLine();
            }
            sText = aBuf.toString();

            aBufReader.close();
        } catch ( Exception e )
        {
            e.printStackTrace();
        }
        return sText;
    }

    protected static String GetDocTitle( XModel xDoc )
    {
        XDocumentPropertiesSupplier xDocPropSup =
            UnoRuntime.queryInterface(XDocumentPropertiesSupplier.class, xDoc);
        XDocumentProperties xDocProps = xDocPropSup.getDocumentProperties();
        return xDocProps.getTitle();
    }

    protected static void SetDocTitle( XModel xDoc, String sTitle )
    {
        XDocumentPropertiesSupplier xDocPropSup =
            UnoRuntime.queryInterface(XDocumentPropertiesSupplier.class, xDoc);
        XDocumentProperties xDocProps = xDocPropSup.getDocumentProperties();
        xDocProps.setTitle(sTitle);
    }

    protected static String GetDocServiceName( XComponentContext xContext, XModel xModel )
    {
        String aDocServiceName = "";
        if ( xModel != null && xContext != null )
        {
            try
            {
                XMultiComponentFactory xFactory = xContext.getServiceManager();
                if ( xFactory == null )
                    throw new com.sun.star.uno.RuntimeException();

                Object oModuleManager = xFactory.createInstanceWithContext( "com.sun.star.frame.ModuleManager", xContext );
                XModuleManager xModuleManager = UnoRuntime.queryInterface( XModuleManager.class, oModuleManager );
                if ( xModuleManager != null )
                    aDocServiceName = xModuleManager.identify( xModel );
            }
            catch( java.lang.Exception e )
            {
                e.printStackTrace();
            }
        }

        return aDocServiceName;
    }

    protected static String GetFilterName( XComponentContext xContext, String aTypeName, String aDocServiceName )
    {
        String aFilterName = "";
        if ( xContext != null && aTypeName != null && aTypeName.length() != 0
          && aDocServiceName != null && aDocServiceName.length() != 0 )
        {
            try
            {
                Object oFilterFactory = xContext.getServiceManager().createInstanceWithContext( "com.sun.star.document.FilterFactory", xContext );
                XContainerQuery xQuery = UnoRuntime.queryInterface( XContainerQuery.class, oFilterFactory );
                if ( xQuery != null )
                {
                    NamedValue[] aRequest = new NamedValue[2];
                    aRequest[0] = new NamedValue( "Type", aTypeName );
                    aRequest[1] = new NamedValue( "DocumentService", aDocServiceName );

                    XEnumeration xSet = xQuery.createSubSetEnumerationByProperties( aRequest );
                    if ( xSet != null )
                    {
                        boolean bAcceptable = false;
                        while ( xSet.hasMoreElements() && !bAcceptable )
                        {
                            PropertyValue[] pFilterProps = ( PropertyValue[] )AnyConverter.toArray( xSet.nextElement() );
                            if ( pFilterProps != null )
                            {
                                int nLen = pFilterProps.length;
                                String aTmpFilter = null;

                                for ( int nInd = 0; nInd < nLen; nInd++ )
                                {
                                    if ( pFilterProps[nInd].Name.equals( "Name" ) )
                                        aTmpFilter = AnyConverter.toString( pFilterProps[nInd].Value );
                                    else if ( pFilterProps[nInd].Name.equals( "Flags" ) )
                                        bAcceptable = ( ( AnyConverter.toInt( pFilterProps[nInd].Value ) & 2 ) == 2 ); // must allow export
                                }

                                if ( bAcceptable )
                                    aFilterName = aTmpFilter;
                            }
                        }
                    }
                }
            }
            catch( java.lang.Exception e )
            {
                e.printStackTrace();
            }
        }

        return aFilterName;
    }

    protected static XMultiServiceFactory GetConfigurationProvider( XComponentContext xContext )
        throws com.sun.star.uno.Exception
    {
        XMultiServiceFactory xConfigurationProvider = null;
        if ( xContext != null )
        {
            XMultiComponentFactory xFactory = xContext.getServiceManager();
            Object oConfigProvider = xFactory.createInstanceWithContext( "com.sun.star.configuration.ConfigurationProvider", xContext );
            xConfigurationProvider = UnoRuntime.queryInterface( XMultiServiceFactory.class, oConfigProvider );
        }

        if ( xConfigurationProvider == null )
            throw new com.sun.star.uno.RuntimeException();

        return xConfigurationProvider;
    }

    protected static Object GetConfig( XComponentContext xContext, String sNodepath, boolean bWriteAccess )
        throws com.sun.star.uno.Exception
    {
        if ( xContext == null || sNodepath == null )
            throw new com.sun.star.uno.RuntimeException();

        PropertyValue aVal = new PropertyValue();
        aVal.Name = "nodepath";
        aVal.Value = sNodepath;
        Object[] aArgs = new Object[1];
        aArgs[0] = aVal;

        return GetConfigurationProvider( xContext ).createInstanceWithArguments(
                                    ( bWriteAccess ? "com.sun.star.configuration.ConfigurationUpdateAccess"
                                                   : "com.sun.star.configuration.ConfigurationAccess" ),
                                    aArgs );
    }

    protected static XPropertySet GetConfigProps( XComponentContext xContext, String sNodepath )
        throws com.sun.star.uno.Exception
    {
        XPropertySet xProps = UnoRuntime.queryInterface( XPropertySet.class, GetConfig( xContext, sNodepath, true ) );
        if ( xProps == null )
            throw new com.sun.star.uno.RuntimeException();

        return xProps;
    }


    protected static XNameContainer GetConfigNameContainer( XComponentContext xContext, String sNodepath )
        throws com.sun.star.uno.Exception
    {
        XNameContainer xContainer = UnoRuntime.queryInterface( XNameContainer.class, GetConfig( xContext, sNodepath, true ) );
        if ( xContainer == null )
            throw new com.sun.star.uno.RuntimeException();

        return xContainer;
    }

    protected static XNameAccess GetConfigNameAccess( XComponentContext xContext, String sNodepath )
        throws com.sun.star.uno.Exception
    {
        XNameAccess xNameAccess = UnoRuntime.queryInterface( XNameAccess.class, GetConfig( xContext, sNodepath, false ) );
        if ( xNameAccess == null )
            throw new com.sun.star.uno.RuntimeException();

        return xNameAccess;
    }

    protected static void SetConfigurationProxy( HostConfiguration aHostConfig, XComponentContext xContext )
    {
        if ( aHostConfig == null || xContext == null )
            return;

        try
        {
            XNameAccess xNameAccess = GetConfigNameAccess( xContext, "org.openoffice.Inet/Settings" );

            int nProxyType = AnyConverter.toInt( xNameAccess.getByName( "ooInetProxyType" ) );
            if ( nProxyType == 0 )
                aHostConfig.setProxyHost( null );
            else
            {
                if ( nProxyType == 1 )
                {
                    // system proxy
                }
                else if ( nProxyType == 2 )
                {
                    String aProxyNameProp = "ooInetHTTPProxyName";
                    String aProxyPortProp = "ooInetHTTPProxyPort";

                    if ( aHostConfig.getProtocol().getScheme().equals( "https" ) )
                    {
                        aProxyNameProp = "ooInetHTTPSProxyName";
                        aProxyPortProp = "ooInetHTTPSProxyPort";
                    }

                    String aNoProxyList = AnyConverter.toString( xNameAccess.getByName( "ooInetNoProxy" ) );
                    String aProxyName = AnyConverter.toString( xNameAccess.getByName( aProxyNameProp ) );

                    int nProxyPort = 80;

                    Object aPortNo = xNameAccess.getByName( aProxyPortProp );
                    if ( !AnyConverter.isVoid( aPortNo ) )
                        nProxyPort = AnyConverter.toInt( aPortNo );

                    if ( nProxyPort == -1 )
                        nProxyPort = 80;

                    // TODO: check whether the URL is in the NoProxy list
                    aHostConfig.setProxy( aProxyName, nProxyPort );
                }
            }
        }
        catch( java.lang.Exception e )
        {
            e.printStackTrace();
        }
    }

    protected static void ShowURLInBrowser( XComponentContext xContext, String sURL )
    {
        if ( xContext != null && sURL != null && sURL.length() > 0 )
        {
            try
            {
                Object oSystemShell = xContext.getServiceManager().createInstanceWithContext( "com.sun.star.system.SystemShellExecute", xContext );
                XSystemShellExecute xSystemShell = UnoRuntime.queryInterface( XSystemShellExecute.class, oSystemShell );
                if ( xSystemShell != null )
                    xSystemShell.execute( sURL, "", SystemShellExecuteFlags.URIS_ONLY );
            }
            catch( Exception e )
            {
                e.printStackTrace();
            }
        }
    }

    protected static void ExecuteMethod( HttpMethodBase aMethod, HostConfiguration aHostConfig, URI aURI, XComponentContext xContext, boolean bSetHost )
        throws WikiCancelException, IOException, SSLException
    {
        if ( aMethod != null && aHostConfig != null && aURI != null && xContext != null )
        {
            if ( bSetHost )
            {
                aHostConfig.setHost( aURI );
                SetConfigurationProxy( aHostConfig, xContext );
            }

            if ( aHostConfig.getProtocol().getScheme().equals( "https" )
              && AllowUnknownCert( xContext, aURI.getHost() ) )
            {
                // let unknown certificates be accepted
                {
                    {
                        aHostConfig.setHost( aHostConfig.getHost(), ( aURI.getPort() < 0 ? 443 : aURI.getPort() ), Helper.GetOwnHttps( aURI.getPort() ) );
                        Helper.GetHttpClient().executeMethod( aHostConfig, aMethod );
                    }
                }
            }
            else
            {
                Helper.GetHttpClient().executeMethod( aHostConfig, aMethod );
            }
        }
    }

    static private class HTMLParse extends HTMLEditorKit
    {

        public HTMLEditorKit.Parser getParser()
        {
            return super.getParser();
        }
    }

    static protected HTMLEditorKit.Parser GetHTMLParser()
    {
        return new HTMLParse().getParser();
    }

    static protected boolean LoginReportsError( String sRespond )
    {
        boolean bResult = true;
        if ( sRespond != null )
        {
            try
            {
                StringReader aReader = new StringReader( sRespond );
                HTMLEditorKit.Parser aParser = GetHTMLParser();
                EditPageParser aCallback = new EditPageParser();

                aParser.parse( aReader, aCallback, true );
                bResult = ( aCallback.m_nErrorInd >= 0 );
            }
            catch( Exception e )
            {
                e.printStackTrace();
            }
        }

        return bResult;
    }

    static protected String GetLoginToken( String sLoginPage )
    {
        String sResult = "";
        if ( sLoginPage != null && sLoginPage.length() > 0 )
        {
            try
            {
                StringReader aReader = new StringReader( sLoginPage );
                HTMLEditorKit.Parser aParser = Helper.GetHTMLParser();
                EditPageParser aCallbacks = new EditPageParser();

                aParser.parse( aReader, aCallbacks, true );
                sResult = aCallbacks.m_sLoginToken;
            }
            catch( Exception e )
            {
                e.printStackTrace();
            }
        }

        return sResult;
    }

    static protected HostConfiguration Login( URI aMainURL, String sWikiUser, String sWikiPass, XComponentContext xContext )
        throws com.sun.star.uno.Exception, java.io.IOException, WikiCancelException
    {
        HostConfiguration aHostConfig = null;

        if ( sWikiUser != null && sWikiPass != null && xContext != null )
        {
            HostConfiguration aNewHostConfig = new HostConfiguration();

            URI aURI = new URI( aMainURL.toString() + "index.php?title=Special:Userlogin" );
            GetMethod aGetCookie = new GetMethod( aURI.getEscapedPathQuery() );

            ExecuteMethod( aGetCookie, aNewHostConfig, aURI, xContext, true );

            int nResultCode = aGetCookie.getStatusCode();
            String sLoginPage = null;
            if ( nResultCode == 200 )
                sLoginPage = aGetCookie.getResponseBodyAsString();

            aGetCookie.releaseConnection();

            if ( sLoginPage != null )
            {
                String sLoginToken = GetLoginToken( sLoginPage );

                PostMethod aPost = new PostMethod();
                URI aPostURI = new URI( aMainURL.getPath() + "index.php?title=Special:Userlogin&action=submitlogin" );
                aPost.setPath( aPostURI.getEscapedPathQuery() );

                aPost.addParameter( "wpName", sWikiUser );
                aPost.addParameter( "wpRemember", "1" );
                aPost.addParameter( "wpPassword", sWikiPass );
                if ( sLoginToken.length() > 0 )
                    aPost.addParameter( "wpLoginToken", sLoginToken );

                String[][] pArgs = GetSpecialArgs( xContext, aMainURL.getHost() );
                if ( pArgs != null )
                    for ( int nArgInd = 0; nArgInd < pArgs.length; nArgInd++ )
                        if ( pArgs[nArgInd].length == 2 && pArgs[nArgInd][0] != null && pArgs[nArgInd][1] != null )
                            aPost.addParameter( pArgs[nArgInd][0], pArgs[nArgInd][1] );

                ExecuteMethod( aPost, aNewHostConfig, aPostURI, xContext, false );

                nResultCode = aPost.getStatusCode();

                while( nResultCode >= 301 && nResultCode <= 303 || nResultCode == 307 )
                {
                    String sRedirectURL = aPost.getResponseHeader( "Location" ).getValue();
                    aPost.releaseConnection();

                    aURI = new URI( sRedirectURL );
                    aPost = new PostMethod();
                    aPost.setPath( aURI.getEscapedPathQuery() );
                    ExecuteMethod( aPost, aNewHostConfig, aURI, xContext, false );

                    nResultCode = aPost.getStatusCode();
                }

                if ( nResultCode == 200 )
                {
                    String sResult = aPost.getResponseBodyAsString();
                    if ( !LoginReportsError( sResult ) )
                        aHostConfig = aNewHostConfig;
                }

                aPost.releaseConnection();
            }
        }

        return aHostConfig;
    }

    private static XControl GetControlFromDialog( XDialog xDialog, String aControlName )
    {
        XControl xResult = null;
        XControlContainer xControlCont = UnoRuntime.queryInterface( XControlContainer.class, xDialog );

        if ( xControlCont != null )
        {
            Object oControl = xControlCont.getControl( aControlName );
            xResult = UnoRuntime.queryInterface( XControl.class, oControl );
        }

        return xResult;
    }

    private static XPropertySet GetSubControlPropSet( XDialog xDialog, String aControlName )
    {
        XControl xControl = GetControlFromDialog( xDialog, aControlName );
        if ( xControl != null )
            return UnoRuntime.queryInterface( XPropertySet.class, xControl.getModel() );

        return null;
    }

    protected static void SetControlPropInDialog( XDialog xDialog, String aControlName, String aPropName, Object aPropValue )
    {
        if ( xDialog != null && aControlName != null && aPropName != null && aPropValue != null )
        {
            try
            {
                XPropertySet xPropSet = GetSubControlPropSet( xDialog, aControlName );
                if ( xPropSet != null )
                    xPropSet.setPropertyValue( aPropName, aPropValue );
            }
            catch ( com.sun.star.uno.Exception e )
            {
                e.printStackTrace();
            }
        }
    }

    protected static String[] GetPasswordsForURLAndUser( XComponentContext xContext, String sURL, String sUserName )
    {
        String[] aResult = null;

        try
        {
            if ( xContext != null && sURL != null && sURL.length() > 0 && sUserName != null && sUserName.length() > 0 )
            {
                UrlRecord aRec = GetPasswordContainer( xContext ).findForName( sURL, sUserName, GetInteractionHandler( xContext ) );
                if ( aRec != null && aRec.UserList != null && aRec.UserList.length > 0
                  && aRec.UserList[0].UserName.equals( sUserName ) )
                    aResult = aRec.UserList[0].Passwords;
            }
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }

        return aResult;
    }

    protected static boolean PasswordStoringIsAllowed( XComponentContext xContext )
    {
        boolean bResult = false;
        try
        {
            XMasterPasswordHandling xMasterHdl = UnoRuntime.queryInterface( XMasterPasswordHandling.class, GetPasswordContainer( xContext ) );
            if ( xMasterHdl != null )
                bResult = xMasterHdl.isPersistentStoringAllowed();
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }

        return bResult;
    }

    protected static void ShowError( XComponentContext xContext, XDialog xDialog, int nTitleID, int nErrorID, String sArg, boolean bQuery )
    {
        XWindowPeer xPeer = null;
        XControl xControl = UnoRuntime.queryInterface( XControl.class, xDialog );
        if ( xControl != null )
            xPeer = xControl.getPeer();
        ShowError( xContext, xPeer, nTitleID, nErrorID, sArg, bQuery );
    }

    protected static boolean ShowError( XComponentContext xContext, XWindowPeer xParentPeer, int nTitleID, int nErrorID, String sArg, boolean bQuery )
    {
        boolean bResult = false;

        if ( xContext != null && nErrorID >= 0 && nErrorID < STRINGS_NUM )
        {
            String sError = null;
            String sTitle = "";

            try
            {
                sError = GetLocalizedString( xContext, nErrorID );
                if ( sError != null && sArg != null )
                    sError = sError.replaceAll( "\\$ARG1", sArg );

                sTitle = GetLocalizedString( xContext, nTitleID );
            }
            catch( Exception e )
            {
                e.printStackTrace();
            }

            if ( sError == null )
                sError = "Error: " + nErrorID;

            if ( xParentPeer != null )
            {
                XMessageBoxFactory xMBFactory = null;
                XMessageBox xMB = null;
                try
                {
                    XMultiComponentFactory xFactory = xContext.getServiceManager();
                    if ( xFactory != null )
                        xMBFactory = UnoRuntime.queryInterface(
                                     XMessageBoxFactory.class,
                                     xFactory.createInstanceWithContext( "com.sun.star.awt.Toolkit", xContext ) );

                    if ( xMBFactory != null )
                    {
                        if ( bQuery )
                        {
                            xMB = xMBFactory.createMessageBox(
                                                     xParentPeer,
                                                     new com.sun.star.awt.Rectangle(),
                                                     "querybox",
                                                     MessageBoxButtons.BUTTONS_YES_NO | MessageBoxButtons.DEFAULT_BUTTON_NO,
                                                     sTitle,
                                                     sError );
                        }
                        else
                        {
                            xMB = xMBFactory.createMessageBox(
                                                     xParentPeer,
                                                     new com.sun.star.awt.Rectangle(),
                                                     "errorbox",
                                                     MessageBoxButtons.BUTTONS_OK,
                                                     sTitle,
                                                     sError );
                        }
                        if ( xMB != null )
                        {
                            bResult = MainThreadDialogExecutor.Execute( xContext, xMB );
                        }
                    }
                }
                catch( Exception e )
                {
                    e.printStackTrace();
                }
                finally
                {
                    if ( xMB != null )
                        Dispose( xMB );
                }
            }
        }

        return bResult;
    }

    private static boolean AllowUnknownCert( XComponentContext xContext, String aURL )
    {
        try
        {
            XNameAccess xNameAccess = GetConfigNameAccess( xContext, "org.openoffice.Office.Custom.WikiExtension/SpecialData" );
            if ( xNameAccess.hasByName( aURL ) )
            {
                XNameAccess xEntry = UnoRuntime.queryInterface( XNameAccess.class, xNameAccess.getByName( aURL ) );
                if ( xEntry != null && xEntry.hasByName( "AllowUnknownCertificate" ) )
                    return AnyConverter.toBoolean( xEntry.getByName( "AllowUnknownCertificate" ) );
            }
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }

        return false;
    }

    private static String[][] GetSpecialArgs( XComponentContext xContext, String aURL )
    {
        try
        {
            XNameAccess xNameAccess = GetConfigNameAccess( xContext, "org.openoffice.Office.Custom.WikiExtension/SpecialData" );
            if ( xNameAccess.hasByName( aURL ) )
            {
                XNameAccess xEntry = UnoRuntime.queryInterface( XNameAccess.class, xNameAccess.getByName( aURL ) );
                if ( xEntry != null )
                {
                    XNameAccess xArgs = UnoRuntime.queryInterface( XNameAccess.class, xEntry.getByName( "AdditionalLoginArguments" ) );
                    if ( xArgs != null )
                    {
                        String[] pNames = xArgs.getElementNames();
                        if ( pNames != null && pNames.length > 0 )
                        {
                            String[][] pResult = new String[pNames.length][2];
                            for ( int nInd = 0; nInd < pNames.length; nInd++ )
                            {
                                XNameAccess xArgument = UnoRuntime.queryInterface( XNameAccess.class, xArgs.getByName( pNames[nInd] ) );
                                if ( xArgument == null )
                                    throw new com.sun.star.uno.RuntimeException();

                                pResult[nInd][0] = pNames[nInd];
                                pResult[nInd][1] = AnyConverter.toString( xArgument.getByName( "Value" ) );
                            }

                            return pResult;
                        }
                    }
                }
            }
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }

        return null;
    }

    protected static boolean AllowThreadUsage( XComponentContext xContext )
    {
        if ( xContext != null )
        {
            try
            {
                XMultiComponentFactory xFactory = xContext.getServiceManager();
                if ( xFactory == null )
                    throw new com.sun.star.uno.RuntimeException();

                Object oCheckCallback = xFactory.createInstanceWithContext( "com.sun.star.awt.AsyncCallback", xContext );
                return ( oCheckCallback != null );
            }
            catch( Exception e )
            {
                e.printStackTrace();
            }
        }

        return false;
    }

    public static void Dispose( Object oObject )
    {
        if ( oObject != null )
        {
            try
            {
                XComponent xComp = UnoRuntime.queryInterface( XComponent.class, oObject );
                if ( xComp != null )
                    xComp.dispose();
            }
            catch( Exception e )
            {
                e.printStackTrace();
            }
        }
    }
}

