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

import java.io.File;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.HashMap;
import java.util.Map;

import javax.net.ssl.SSLException;

import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.DispatchDescriptor;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;


public final class WikiEditorImpl extends WeakBase
   implements com.sun.star.lang.XServiceInfo, XDispatchProvider, XDispatch, XInitialization
{

    private static final String m_implementationName = WikiEditorImpl.class.getName();
    private static final String[] m_serviceNames = {"com.sun.star.wiki.WikiEditor" };



    // protocol name that this protocol handler handles
    private static final String protocolName = "vnd.com.sun.star.wiki:";

    private final XComponentContext m_xContext;
    private final Map<String, com.sun.star.frame.XStatusListener> m_statusListeners = new HashMap<String, com.sun.star.frame.XStatusListener>();
    private XFrame m_xFrame;
    private XModel m_xModel;
    private final Settings m_aSettings;
    private String m_aFilterName;

    public WikiEditorImpl( XComponentContext xContext )
    {
        // Helper.trustAllSSL();
        m_xContext = xContext;
        m_aSettings = Settings.getSettings( m_xContext );
    }

    public static XSingleComponentFactory __getComponentFactory( String sImplementationName )
    {
        XSingleComponentFactory xFactory = null;

        if ( sImplementationName.equals( m_implementationName ) )
            xFactory = Factory.createComponentFactory( WikiEditorImpl.class, m_serviceNames );
        else if ( sImplementationName.equals( WikiOptionsEventHandlerImpl.m_sImplementationName ) )
            xFactory = Factory.createComponentFactory( WikiOptionsEventHandlerImpl.class,
                                                       WikiOptionsEventHandlerImpl.m_pServiceNames );

        return xFactory;
    }

    // com.sun.star.lang.XServiceInfo:
    public String getImplementationName()
    {
         return m_implementationName;
    }

    public boolean supportsService( String sService )
    {
        int len = m_serviceNames.length;

        for( int i=0; i < len; i++ )
        {
            if ( sService.equals( m_serviceNames[i] ))
                return true;
        }
        return false;
    }

    public String[] getSupportedServiceNames()
    {
        return m_serviceNames;
    }


    public synchronized void initialize( Object[] args ) throws com.sun.star.uno.Exception
    {
        if ( args.length > 0 )
        {
            m_xFrame = UnoRuntime.queryInterface( XFrame.class, args[0] );
        }
    }



    public void dispatch(
        final com.sun.star.util.URL aURL,
        com.sun.star.beans.PropertyValue[] propertyValue )
    {
        final com.sun.star.util.URL myURL = aURL;
        if ( aURL.Protocol.equals(protocolName) )
        {
            try
            {
                if ( myURL.Path.equals("send") )
                {
                    sendArticle();
                }
            } catch( java.lang.Throwable t )
            {
            }
        }
    }


    public com.sun.star.frame.XDispatch queryDispatch(
        com.sun.star.util.URL aURL,
        String str,
        int param )
    {
            if ( aURL.Protocol.equals( protocolName ))
            {

                // by default, we are responsible
                return this;
            } else
            {
                return null;
            }
    }

    public XDispatch[] queryDispatches( DispatchDescriptor[] seqDescripts )
    {
        int nCount = seqDescripts.length;
        XDispatch[] lDispatcher = new XDispatch[nCount];

        for( int i=0; i<nCount; ++i )
            lDispatcher[i] = queryDispatch(
                seqDescripts[i].FeatureURL,
                seqDescripts[i].FrameName,
                seqDescripts[i].SearchFlags );
        return lDispatcher;
   }


    public void removeStatusListener(
        com.sun.star.frame.XStatusListener xStatusListener,
        com.sun.star.util.URL aURL )
    {
    }


    public void addStatusListener(
        com.sun.star.frame.XStatusListener listener,
        com.sun.star.util.URL url )
    {
        String urlstring = url.Complete;
        m_statusListeners.put( urlstring, listener );
        // synchroneous callback required!!!
        callStatusListener( urlstring );
    }



    private void callStatusListener( String uristring )
    {
        try
        {
            new URI( uristring );

            // check whether any blogs are live...
            setListenerState( "command");
        } catch ( URISyntaxException ex )
        {
            ex.printStackTrace();
        }
    }


    private void setListenerState( String urlstring)
    {
        com.sun.star.util.URL url = new com.sun.star.util.URL();
        url.Complete = urlstring;
    }

    private void sendArticle()
    {
        if ( m_xFrame != null )
        {
            WikiPropDialog aSendDialog = null;
            try
            {
                if ( m_xModel == null )
                {
                    XController xController = m_xFrame.getController();
                    if ( xController != null )
                        m_xModel = xController.getModel();
                }

                if ( m_xModel != null )
                {
                    // The related Wiki filter must be detected from the typename
                    String aServiceName = Helper.GetDocServiceName( m_xContext, m_xModel );
                    m_aFilterName = Helper.GetFilterName( m_xContext, "MediaWiki", aServiceName );

                    if ( m_aFilterName == null || m_aFilterName.length() == 0 )
                    {
                        Helper.ShowError( m_xContext,
                                          UnoRuntime.queryInterface( XWindowPeer.class, m_xFrame.getContainerWindow() ),
                                          Helper.DLG_SENDTITLE,
                                          Helper.NOWIKIFILTER_ERROR,
                                          null,
                                          false );
                        throw new com.sun.star.uno.RuntimeException();
                    }

                    m_aSettings.loadConfiguration(); // throw away all the noncommited changes
                    // show the send dialog
                    aSendDialog = new WikiPropDialog( m_xContext, "vnd.sun.star.script:WikiEditor.SendToMediaWiki?location=application", this );
                    aSendDialog.fillWikiList();
                    aSendDialog.SetWikiTitle( Helper.GetDocTitle( m_xModel ) );
                    aSendDialog.show(); // triggers the sending
                }
            }
            catch ( Exception e )
            {
                // TODO: Error handling here
                e.printStackTrace();
            }
            finally
            {
                if ( aSendDialog != null )
                    aSendDialog.DisposeDialog();
            }
        }
    }

    public boolean SendArticleImpl( WikiPropDialog aSendDialog, Map<String,String> aWikiSetting )
    {
        boolean bResult = false;

        if ( aSendDialog != null )
        {
            String sTemp2Url = null;

            try
            {
                // TODO: stop progress spinning
                WikiArticle aArticle = new WikiArticle( m_xContext, aSendDialog.GetWikiTitle(), aWikiSetting, true, aSendDialog );

                boolean bAllowSending = true;
                if ( !aArticle.NotExist() )
                {
                    // ask whether creation of a new page is allowed
                    aSendDialog.SetThrobberActive( false );
                    bAllowSending = Helper.ShowError(
                                      m_xContext,
                                      UnoRuntime.queryInterface( XWindowPeer.class, m_xFrame.getContainerWindow() ),
                                      Helper.DLG_SENDTITLE,
                                      Helper.DLG_WIKIPAGEEXISTS_LABEL1,
                                      aSendDialog.GetWikiTitle(),
                                      true );
                    aSendDialog.SetThrobberActive( true );
                }

                if ( bAllowSending )
                {
                    PropertyValue[] lProperties = new PropertyValue[2];
                    lProperties[0]       = new PropertyValue();
                    lProperties[0].Name  = "FilterName";
                    lProperties[0].Value = m_aFilterName;
                    lProperties[1]       = new PropertyValue();
                    lProperties[1].Name  = "Overwrite";
                    lProperties[1].Value = Boolean.TRUE;

                    sTemp2Url = Helper.CreateTempFile( m_xContext );

                    XStorable xStore = UnoRuntime.queryInterface ( XStorable.class, m_xModel );
                    if ( xStore == null )
                        throw new com.sun.star.uno.RuntimeException();

                    xStore.storeToURL( sTemp2Url, lProperties );
                    String sWikiCode = Helper.EachLine( sTemp2Url );

                    if ( aArticle.setArticle( sWikiCode, aSendDialog.m_sWikiComment, aSendDialog.m_bWikiMinorEdit ) )
                    {
                        bResult = true;
                        Helper.SetDocTitle( m_xModel, aArticle.GetTitle() );
                        Map<String,Object> aDocInfo = new HashMap<String,Object>();
                        aDocInfo.put( "Doc", aArticle.GetTitle() );
                        aDocInfo.put( "Url", aArticle.GetMainURL() );
                        aDocInfo.put( "CompleteUrl", aArticle.GetMainURL() + aArticle.GetTitle() );
                        m_aSettings.addWikiDoc( aDocInfo );
                        m_aSettings.storeConfiguration();
                    }
                    else
                    {
                        Helper.ShowError( m_xContext,
                                          UnoRuntime.queryInterface( XWindowPeer.class, m_xFrame.getContainerWindow() ),
                                          Helper.DLG_SENDTITLE,
                                          Helper.GENERALSEND_ERROR,
                                          null,
                                          false );
                    }
                }
            }
            catch( WikiCancelException ec )
            {
                // nothing to do, the sending was cancelled
            }
            catch( SSLException essl )
            {
                if ( Helper.IsConnectionAllowed() )
                {
                    // report the error only if sending was not cancelled
                    Helper.ShowError( m_xContext,
                                      UnoRuntime.queryInterface( XWindowPeer.class, m_xFrame.getContainerWindow() ),
                                      Helper.DLG_SENDTITLE,
                                      Helper.UNKNOWNCERT_ERROR,
                                      null,
                                      false );
                }
            }
            catch( Exception e )
            {
                if ( Helper.IsConnectionAllowed() )
                {
                    // report the error only if sending was not cancelled
                    Helper.ShowError( m_xContext,
                                      UnoRuntime.queryInterface( XWindowPeer.class, m_xFrame.getContainerWindow() ),
                                      Helper.DLG_SENDTITLE,
                                      Helper.GENERALSEND_ERROR,
                                      null,
                                      false );
                }
                e.printStackTrace();
            }

            if ( sTemp2Url != null )
            {
                try
                {
                    // remove the temporary file
                    File aFile = new File( new URI( sTemp2Url ) );
                    if (!aFile.delete()) {
                        throw new java.lang.Exception("could not remove" + sTemp2Url);
                    }
                }
                catch ( java.lang.Exception e )
                {
                    e.printStackTrace();
                }
            }
        }

        return bResult;
    }

}
