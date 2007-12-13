/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Helper.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mav $ $Date: 2007-12-13 10:34:07 $
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

package com.sun.star.wiki;

import com.sun.star.awt.XDialog;
import com.sun.star.beans.NamedValue;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XContainerQuery;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XNameAccess;
import com.sun.star.document.XDocumentInfoSupplier;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XModuleManager;
import com.sun.star.io.XInputStream;
import com.sun.star.io.XOutputStream;
import com.sun.star.io.XSeekable;
import com.sun.star.io.XStream;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.system.SystemShellExecuteFlags;
import com.sun.star.system.XSystemShellExecute;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import java.net.*;
import java.io.*;
import java.util.Hashtable;
import java.util.Random;
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

    private static final String sHTMLHeader = "<HTML><HEAD><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><TITLE></TITLE></HEAD><BODY>";
    private static final String sHTMLFooter = "</BODY></HTML>";

    private static Random m_aRandom;
    private static MultiThreadedHttpConnectionManager m_aConnectionManager;
    private static HttpClient m_aClient;
    private static boolean m_bAllowConnection = true;
    private static Hashtable m_aAcceptedUnknownCerts;

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

    protected static Protocol GetOwnHttps( int nPort )
    {
        return new Protocol( "https", new WikiProtocolSocketFactory(), ( ( nPort < 0 ) ? 443 : nPort ) );
    }

    protected static String GetMainURL( String sWebPage, String sVURL )
    {
        //scrape the HTML source and find the EditURL
        String sResultURL = "";
        int i = sWebPage.indexOf( "action=edit" );
        if ( i!=-1 )
        {
            int t = sWebPage.lastIndexOf( "a href=", i );
            int z = sWebPage.indexOf( "index.php", t );
            sResultURL = sWebPage.substring( t+8,z );
            try
            {
                if ( !sResultURL.startsWith( "http" ))
                {
                    //if the url is only relative then complete it
                    URL aURL = new URL( sVURL );
                    sResultURL = aURL.getProtocol()+"://"+aURL.getHost()+sResultURL;
                }
            }
            catch ( MalformedURLException ex )
            {
                ex.printStackTrace();
            }
        }

        return sResultURL;
    }

    protected static String GetRedirectURL( String sWebPage, String sURL )
    {
        //scrape the HTML source and find the EditURL
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
                XStream xStream = ( XStream ) UnoRuntime.queryInterface( XStream.class, oTempFile );
                XSeekable xSeekable = ( XSeekable ) UnoRuntime.queryInterface( XSeekable.class, oTempFile );
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
            XPropertySet xPropertySet = ( XPropertySet ) UnoRuntime.queryInterface( XPropertySet.class, oTempFile );
            xPropertySet.setPropertyValue( "RemoveFile", Boolean.FALSE );
            sURL = ( String ) xPropertySet.getPropertyValue( "Uri" );

            XInputStream xInputStream = ( XInputStream ) UnoRuntime.queryInterface( XInputStream.class, oTempFile );
            xInputStream.closeInput();
            XOutputStream xOutputStream = ( XOutputStream ) UnoRuntime.queryInterface( XOutputStream.class, oTempFile );
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
        } catch ( Exception e )
        {
            e.printStackTrace();
        }
        return sText;
    }

    protected static String GetDocTitle( XModel xDoc )
    {
        String sTitle = "";
        XDocumentInfoSupplier xDocInfoSup = ( XDocumentInfoSupplier ) UnoRuntime.queryInterface( XDocumentInfoSupplier.class, xDoc );
        XPropertySet xPropSet = ( XPropertySet ) UnoRuntime.queryInterface( XPropertySet.class, xDocInfoSup.getDocumentInfo() );
        try
        {
            sTitle = ( String ) xPropSet.getPropertyValue( "Title" );
        } catch ( Exception ex )
        {
            ex.printStackTrace();
        }
        return sTitle;
    }

    protected static void SetDocTitle( XModel xDoc, String sTitle )
    {
        XDocumentInfoSupplier xDocInfoSup = ( XDocumentInfoSupplier ) UnoRuntime.queryInterface( XDocumentInfoSupplier.class, xDoc );
        if ( xDocInfoSup != null )
        {
            XPropertySet xPropSet = ( XPropertySet ) UnoRuntime.queryInterface( XPropertySet.class, xDocInfoSup.getDocumentInfo() );
            if ( xPropSet != null )
            {
                try
                {
                    xPropSet.setPropertyValue( "Title", sTitle );
                }
                catch ( Exception ex )
                {
                    ex.printStackTrace();
                }
            }
        }
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
                XModuleManager xModuleManager = ( XModuleManager ) UnoRuntime.queryInterface( XModuleManager.class, oModuleManager );
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
                XContainerQuery xQuery = ( XContainerQuery )UnoRuntime.queryInterface( XContainerQuery.class, oFilterFactory );
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
            xConfigurationProvider = ( XMultiServiceFactory ) UnoRuntime.queryInterface( XMultiServiceFactory.class, oConfigProvider );
        }

        if ( xConfigurationProvider == null )
            throw new com.sun.star.uno.RuntimeException();

        return xConfigurationProvider;
    }

    protected static void SetConfigurationProxy( HostConfiguration aHostConfig, XComponentContext xContext )
    {
        if ( aHostConfig == null || xContext == null )
            return;

        try
        {
            PropertyValue aVal = new PropertyValue();
            aVal.Name = "nodepath";
            aVal.Value = "org.openoffice.Inet/Settings";
            Object[] aArgs = new Object[1];
            aArgs[0] = aVal;

            Object oSettings = Helper.GetConfigurationProvider( xContext ).createInstanceWithArguments(
                                        "com.sun.star.configuration.ConfigurationAccess",
                                        aArgs );
            XNameAccess xNameAccess = ( XNameAccess ) UnoRuntime.queryInterface( XNameAccess.class, oSettings );

            if ( xNameAccess == null )
                return;

            int nProxyType = AnyConverter.toInt( xNameAccess.getByName( "ooInetProxyType" ) );
            String aNoProxyList = AnyConverter.toString( xNameAccess.getByName( "ooInetNoProxy" ) );
            String aProxyName = AnyConverter.toString( xNameAccess.getByName( "ooInetHTTPProxyName" ) );

            int nProxyPort = AnyConverter.toInt( xNameAccess.getByName( "ooInetHTTPProxyPort" ) );

            if ( nProxyPort == -1 )
                nProxyPort = 80;

            if ( nProxyType == 0 )
                aHostConfig.setProxy( "", 0 );
            else
            {
                // TODO: check whether the URL is in the NoProxy list
                aHostConfig.setProxy( aProxyName, nProxyPort );
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
                XSystemShellExecute xSystemShell = (XSystemShellExecute)UnoRuntime.queryInterface( XSystemShellExecute.class, oSystemShell );
                if ( xSystemShell != null )
                    xSystemShell.execute( sURL, "", SystemShellExecuteFlags.DEFAULTS );
            }
            catch( Exception e )
            {
                e.printStackTrace();
            }
        }
    }

    protected static void ExecuteMethod( HttpMethodBase aMethod, HostConfiguration aHostConfig, URI aURI, XComponentContext xContext, boolean bSetHost )
        throws WikiCancelException, IOException
    {
        if ( aMethod != null && aHostConfig != null && aURI != null && xContext != null )
        {
            if ( bSetHost )
            {
                aHostConfig.setHost( aURI );
                SetConfigurationProxy( aHostConfig, xContext );
            }

            boolean bNoUnknownCertNotification = false;
            if ( aHostConfig.getProtocol().getScheme().equals( "https" ) &&  m_aAcceptedUnknownCerts != null )
            {
                Boolean bAccepted = (Boolean)m_aAcceptedUnknownCerts.get( aHostConfig.getHost() );
                bNoUnknownCertNotification = ( bAccepted != null && bAccepted.booleanValue() );
            }

            if ( !bNoUnknownCertNotification )
            {
                try
                {
                    Helper.GetHttpClient().executeMethod( aHostConfig, aMethod );
                }
                catch ( SSLException e )
                {
                    if ( aURI.getScheme().equals( "https" ) )
                    {
                        // the complete secure connection seems to be impossible
                        XDialog xAskDialog = WikiDialog.CreateSimpleDialog( xContext, "vnd.sun.star.script:WikiEditor.UnknownCertDialog?location=application" );
                        if ( xAskDialog != null && MainThreadDialogExecutor.Execute( xContext, xAskDialog ) )
                        {
                            if ( m_aAcceptedUnknownCerts == null )
                                m_aAcceptedUnknownCerts = new Hashtable();
                            m_aAcceptedUnknownCerts.put( aURI.getHost(), Boolean.TRUE );
                            bNoUnknownCertNotification = true;
                        }
                    }
                }
            }

            if ( bNoUnknownCertNotification )
            {
                {
                    {
                        aHostConfig.setHost( aHostConfig.getHost(), ( aURI.getPort() < 0 ? 443 : aURI.getPort() ), Helper.GetOwnHttps( aURI.getPort() ) );
                        Helper.GetHttpClient().executeMethod( aHostConfig, aMethod );
                    }
                }
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

    static protected HostConfiguration Login( URI aMainURL, String sWikiUser, String sWikiPass, String sWikiDomain, XComponentContext xContext )
        throws com.sun.star.uno.Exception, java.io.IOException, WikiCancelException
    {
        HostConfiguration aHostConfig = null;

        if ( sWikiUser != null && sWikiUser.length() > 0 && sWikiPass != null && sWikiDomain != null && xContext != null )
        {
            HostConfiguration aNewHostConfig = new HostConfiguration();

            URI aURI = new URI( aMainURL.toString() + "index.php?title=Special:Userlogin" );
            GetMethod aGetCookie = new GetMethod( aURI.getEscapedPathQuery() );

            ExecuteMethod( aGetCookie, aNewHostConfig, aURI, xContext, true );

            int nResultCode = aGetCookie.getStatusCode();
            aGetCookie.releaseConnection();

            if ( nResultCode == 200 )
            {
                PostMethod aPost = new PostMethod();
                URI aPostURI = new URI( aMainURL.getPath() + "index.php?title=Special:Userlogin&action=submitlogin" );
                aPost.setPath( aPostURI.getEscapedPathQuery() );

                aPost.addParameter( "wpName", sWikiUser );
                aPost.addParameter( "wpRemember", "1" );
                aPost.addParameter( "wpPassword", sWikiPass );
                aPost.addParameter( "wpDomain", sWikiDomain );

                ExecuteMethod( aPost, aNewHostConfig, aPostURI, xContext, false );

                nResultCode = aPost.getStatusCode();
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
        XControlContainer xControlCont = (XControlContainer) UnoRuntime.queryInterface( XControlContainer.class, xDialog );

        if ( xControlCont != null )
        {
            Object oControl = xControlCont.getControl( aControlName );
            xResult = ( XControl ) UnoRuntime.queryInterface( XControl.class, oControl );
        }

        return xResult;
    }

    private static XPropertySet GetSubControlPropSet( XDialog xDialog, String aControlName )
    {
        XControl xControl = GetControlFromDialog( xDialog, aControlName );
        if ( xControl != null )
            return ( XPropertySet ) UnoRuntime.queryInterface( XPropertySet.class, xControl.getModel() );

        return null;
    }

    private static void SetControlPropInDialog( XDialog xDialog, String aControlName, String aPropName, Object aPropValue )
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
}

