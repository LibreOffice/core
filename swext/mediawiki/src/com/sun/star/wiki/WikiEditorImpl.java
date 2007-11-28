/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WikiEditorImpl.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mav $ $Date: 2007-11-28 11:14:58 $
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
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.DispatchDescriptor;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStatusListener;
import com.sun.star.frame.XStorable;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XInitialization;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lib.uno.helper.Factory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.lib.uno.helper.WeakBase;
import com.sun.star.util.XCloseBroadcaster;
import com.sun.star.view.XSelectionSupplier;
import java.io.File;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;


public final class WikiEditorImpl extends WeakBase
   implements com.sun.star.lang.XServiceInfo, XDispatchProvider, XDispatch, XInitialization
{

    private final XComponentContext m_xContext;
    private static final String m_implementationName = WikiEditorImpl.class.getName();
    private static final String[] m_serviceNames = {"com.sun.star.wiki.WikiEditor" };

    // information needed for component registration
    public static final String[] supportedServiceNames = {"com.sun.star.frame.ProtocolHandler"};
    public static final Class implementationClass = WikiEditorImpl.class;
    // protocol name that this protocol handler handles
    public static final String protocolName = "vnd.com.sun.star.wiki:";

    private Map m_statusListeners = new HashMap();


    private XComponent xComp;
    private String sTempUrl;

    private XFrame m_xFrame;
    private XModel m_xModel;
    private Settings m_settings;

    private String m_aFilterName;

    public WikiEditorImpl( XComponentContext xContext )
    {
        // Helper.trustAllSSL();
        m_xContext = xContext;
        m_settings = Settings.getSettings( m_xContext );
    };

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

    public static boolean __writeRegistryServiceInfo( XRegistryKey xRegistryKey )
    {
        boolean bResult = Factory.writeRegistryServiceInfo( m_implementationName,
                                                            m_serviceNames,
                                                            xRegistryKey );
        return ( bResult && Factory.writeRegistryServiceInfo( WikiOptionsEventHandlerImpl.m_sImplementationName,
                                                              WikiOptionsEventHandlerImpl.m_pServiceNames,
                                                              xRegistryKey ) );
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


    private XSelectionSupplier m_sel;
    private XController m_ctrl;
    private boolean m_bInitialized;
    public synchronized void initialize( Object[] args ) throws com.sun.star.uno.Exception
    {
        if ( m_bInitialized )
        {
            //logger.log( Level.SEVERE, "Extension instance was initialized again" );
        }
        if ( args.length > 0 )
        {
            m_bInitialized = true;
            m_xFrame = ( XFrame )UnoRuntime.queryInterface( XFrame.class, args[0] );
            // become close listener
            XCloseBroadcaster cb = ( XCloseBroadcaster )UnoRuntime.queryInterface(
                XCloseBroadcaster.class, m_xFrame );
        }
    }



    public void dispatch(
        final com.sun.star.util.URL aURL,
        com.sun.star.beans.PropertyValue[] propertyValue )
    {
        final com.sun.star.util.URL myURL = aURL;
        //logger.log( Level.INFO, "received dispatch request for: "+aURL.Complete );
        if ( aURL.Protocol.compareTo( protocolName ) == 0 )
        {
            /*
            synchronized( this )
            {
                if( m_bClosing ) return;
                else m_bActive = true;
            }
             **/

            try
            {
                if ( myURL.Path.compareTo( "load" ) == 0 )
                {
                    loadArticle();
                }
                if ( myURL.Path.compareTo( "send" ) == 0 )
                {
                    sendArticle();
                }
            } catch( java.lang.Throwable t )
            {
                //logger.log( Level.WARNING, "exception while handeling dispatch", t );
            }

            /*
            synchronized( this )
            {
                m_bActive = false;
                // if we became owner while we were active
                // we are responsible for closing the m_xFrame now
                if ( m_bOwner && m_xFrame != null )
                {
                    try
                    {
                        XCloseable xclose = ( XCloseable )UnoRuntime.queryInterface(
                            XCloseable.class, m_xFrame );
                        xclose.close( true );
                    } catch ( CloseVetoException cve )
                    {
                        logger.log( Level.SEVERE, "cannot close owned frame" );
                    }
                    // relase reference to the m_xFrame;
                    m_xFrame = null;
                }
            }
             */
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
        callStatusListener( urlstring, listener );
    }

    public void callStatusListeners()
    {
        Set entries = m_statusListeners.entrySet();
        Iterator iter = entries.iterator();
        while ( iter.hasNext() )
        {
            Map.Entry entry = ( Map.Entry ) iter.next();
            String uristring = ( String ) entry.getKey();
            XStatusListener listener = ( XStatusListener ) entry.getValue();
            callStatusListener( uristring, listener );
        }
    }

    public void callStatusListener( String uristring, XStatusListener listener )
    {
        try
        {

            URI uri = new URI( uristring );

            // check whether any blogs are live...
            setListenerState( listener, "command", false );
        } catch ( URISyntaxException ex )
        {
            ex.printStackTrace();
        }
    }


    private void setListenerState( XStatusListener listener, String urlstring, boolean state )
    {
        com.sun.star.util.URL url = new com.sun.star.util.URL();
        url.Complete = urlstring;
        //listener.statusChanged( new FeatureStateEvent( this, url, "", state, false, null ));

    }


    public void loadArticle()
    {
        try
        {
            WikiPropDialog aPropDialog = new WikiPropDialog( m_xContext, "vnd.sun.star.script:WikiEditor.Load?location=application", null );
            aPropDialog.fillWikiList();
            aPropDialog.fillDocList();
            if ( aPropDialog.show() )
            {
                Hashtable wikiSettings = m_settings.getSettingByUrl( aPropDialog.m_sWikiEngineURL );

                WikiArticle aArticle = new WikiArticle( m_xContext, aPropDialog.m_sWikiTitle, wikiSettings, false, null );

                if ( aArticle.m_sHTMLCode == null )
                {
                    // the loading has failed
                    // TODO: error handling
                    return;
                }

                aArticle.cleanHTML();

                XInputStream xInputStream = Helper.SaveHTMLTemp( m_xContext, aArticle.m_sHTMLCode );
                if ( xInputStream == null )
                    throw new com.sun.star.io.IOException();

                Object oDesktop = m_xContext.getServiceManager().createInstanceWithContext( "com.sun.star.frame.Desktop",m_xContext );
                XComponentLoader xCL = ( XComponentLoader ) UnoRuntime.queryInterface( XComponentLoader.class, oDesktop );
                if ( xCL == null )
                    throw new com.sun.star.uno.RuntimeException();

                PropertyValue [] props = new PropertyValue[4];
                props[0] = new PropertyValue();
                props[0].Name = "FilterName";
                props[0].Value = "HTML";
                props[1] = new PropertyValue();
                props[1].Name = "DocumentTitle";
                props[1].Value = aArticle.GetTitle();
                props[2] = new PropertyValue();
                props[2].Name = "InputStream";
                props[2].Value = xInputStream;
                props[3] = new PropertyValue();
                props[3].Name = "DocumentBaseURL";
                props[3].Value = aArticle.GetMainURL() + aArticle.GetTitle();

                // the following argument seems to let office crash
                // props[4] = new PropertyValue();
                // props[4].Name = "AsTemplate";
                // props[4].Value = true;

                xComp = xCL.loadComponentFromURL( "private:stream", "_blank", 0, props );

                Hashtable htd = new Hashtable();

                htd.put( "Doc", aArticle.GetTitle() );
                htd.put( "Url", aArticle.GetMainURL() );
                htd.put( "CompleteUrl", aArticle.GetMainURL() + aArticle.GetTitle() );
                m_settings.addWikiDoc( htd );
                m_settings.storeConfiguration();
            }

        } catch ( Exception e )
        {
            e.printStackTrace();
        }
    }

    public void sendArticle()
    {
        if ( m_xFrame != null )
        {
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
                        ErrorDialog ed = new ErrorDialog(m_xContext, "vnd.sun.star.script:WikiEditor.Error?location=application", "MediaWiki filter is requered to be able to export!");
                        ed.show();
                        throw new com.sun.star.uno.RuntimeException();
                    }

                    // show the send dialog
                    WikiPropDialog aSendDialog = new WikiPropDialog( m_xContext, "vnd.sun.star.script:WikiEditor.SendToMediaWiki?location=application", this );
                    aSendDialog.fillWikiList();
                    aSendDialog.m_sWikiTitle = Helper.GetDocTitle( m_xModel );
                    aSendDialog.show(); // triggers the sending
                }
            }
            catch ( Exception e )
            {
                // TODO: Error handling here
                e.printStackTrace();
            }
        }
    }

    public boolean SendArticleImpl( WikiPropDialog aSendDialog )
    {
        boolean bResult = false;

        if ( aSendDialog != null )
        {
            String sTemp2Url = null;

            try
            {
                Hashtable wikiSettings = m_settings.getSettingByUrl( aSendDialog.m_sWikiEngineURL );

                // TODO: stop progress spinning
                WikiArticle aArticle = new WikiArticle( m_xContext, aSendDialog.m_sWikiTitle, wikiSettings, true, aSendDialog );

                boolean bAllowSending = true;
                if ( aArticle.NotExist() )
                {
                    // ask whether creation of a new page is allowed
                    XDialog xDialog = WikiDialog.CreateSimpleDialog( m_xContext, "vnd.sun.star.script:WikiEditor.NewWikiPage?location=application" );
                    if ( xDialog != null )
                    {
                        aSendDialog.SetThrobberActive( false );
                        bAllowSending = MainThreadDialogExecutor.Execute( m_xContext, xDialog );
                        aSendDialog.SetThrobberActive( true );
                    }
                    else
                        throw new WikiCancelException();
                }

                if ( bAllowSending )
                {
                    PropertyValue[] lProperties = new PropertyValue[2];
                    lProperties[0]       = new PropertyValue();
                    lProperties[0].Name  = "FilterName";
                    lProperties[0].Value = m_aFilterName;
                    lProperties[1]       = new PropertyValue();
                    lProperties[1].Name  = "Overwrite";
                    lProperties[1].Value = new Boolean( true );

                    sTemp2Url = Helper.CreateTempFile( m_xContext );

                    XStorable xStore = ( com.sun.star.frame.XStorable )UnoRuntime.queryInterface ( XStorable.class, m_xModel );
                    if ( xStore == null )
                        throw new com.sun.star.uno.RuntimeException();

                    xStore.storeToURL( sTemp2Url, lProperties );
                    String sWikiCode = Helper.EachLine( sTemp2Url );

                    if ( aArticle.setArticle( sWikiCode, aSendDialog.m_sWikiComment, aSendDialog.m_bWikiMinorEdit ) )
                    {
                        bResult = true;
                        Object desktop = m_xContext.getServiceManager().createInstanceWithContext( "com.sun.star.frame.Desktop", m_xContext );
                        XDesktop xDesktop = ( XDesktop ) UnoRuntime.queryInterface( com.sun.star.frame.XDesktop.class, desktop );
                        Helper.SetDocTitle( m_xModel, aArticle.GetTitle() );
                        Hashtable aDocInfo = new Hashtable();
                        aDocInfo.put( "Doc", aArticle.GetTitle() );
                        aDocInfo.put( "Url", aArticle.GetMainURL() );
                        aDocInfo.put( "CompleteUrl", aArticle.GetMainURL() + aArticle.GetTitle() );
                        m_settings.addWikiDoc( aDocInfo );
                        m_settings.storeConfiguration();

                        if ( aSendDialog.m_bWikiShowBrowser )
                           Helper.ShowURLInBrowser( m_xContext, aArticle.GetViewURL() );
                    }
                    else
                    {
                        ErrorDialog ed = new ErrorDialog(m_xContext, "vnd.sun.star.script:WikiEditor.Error?location=application", "Sending of the document has failed!");
                        MainThreadDialogExecutor.Show( m_xContext, ed );
                    }
                }
            }
            catch( WikiCancelException ec )
            {
                // nothing to do, the sending was cancelled
            }
            catch( Exception e )
            {
                if ( Helper.IsConnectionAllowed() )
                {
                    // report the error only if sending was not cancelled
                    ErrorDialog ed = new ErrorDialog(m_xContext, "vnd.sun.star.script:WikiEditor.Error?location=application", "Sending of the document has failed!");
                    MainThreadDialogExecutor.Show( m_xContext, ed );
                }
                e.printStackTrace();
            }

            if ( sTemp2Url != null )
            {
                try
                {
                    // remove the temporary file
                    File aFile = new File( new URI( sTemp2Url ) );
                    aFile.delete();
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
