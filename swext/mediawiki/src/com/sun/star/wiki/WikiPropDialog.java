/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: WikiPropDialog.java,v $
 *
 * $Revision: 1.19 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package com.sun.star.wiki;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDialog;
import com.sun.star.awt.XThrobber;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import java.util.Hashtable;

public class WikiPropDialog extends WikiDialog{

    WikiEditorImpl m_aWikiEditor;

    private final String sSendMethod = "Send";
    private final String sWikiListMethod = "WikiListChange";
    private final String sArticleTextMethod = "ArticleTextChange";
    private final String sAddWikiMethod = "AddWiki";

    String[] m_pMethods = {sSendMethod, sWikiListMethod, sArticleTextMethod, sAddWikiMethod};

    private String m_sWikiTitle = "";
    protected String m_sWikiEngineURL = "";
    protected String m_sWikiComment = "";
    protected boolean m_bWikiMinorEdit = false;

    /** Creates a new instance of WikiPropDialog */
    public WikiPropDialog(XComponentContext xContext, String DialogURL, WikiEditorImpl aWikiEditorForThrobber )
    {
        super(xContext, DialogURL);
        super.setMethods(m_pMethods);

        if ( aWikiEditorForThrobber != null )
        {
            InsertThrobber( 224, 122, 10, 10 );
            m_aWikiEditor = aWikiEditorForThrobber;
        }

        InitStrings( xContext );
        InitShowBrowser( xContext );
        InitControls( xContext );
    }

    private void InitControls( XComponentContext xContext )
    {
        try
        {
            GetPropSet( "CommentText" ).setPropertyValue( "AutoVScroll", Boolean.TRUE );
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }
    }

    private void InitStrings( XComponentContext xContext )
    {
        try
        {
            SetTitle( Helper.GetLocalizedString( xContext, Helper.DLG_SENDTITLE ) );
            GetPropSet( "Label1" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_SENDTOMEDIAWIKI_LABEL1 ) );
            GetPropSet( "FixedLine2" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_WIKIARTICLE ) );
            GetPropSet( "Label2" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_SENDTOMEDIAWIKI_LABEL2 ) );
            GetPropSet( "Label3" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_SENDTOMEDIAWIKI_LABEL3 ) );
            GetPropSet( "MinorCheck" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_SENDTOMEDIAWIKI_MINORCHECK ) );
            GetPropSet( "BrowserCheck" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_SENDTOMEDIAWIKI_BROWSERCHECK ) );
            GetPropSet( "AddButton" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_ADDBUTTON ) );
            GetPropSet( "SendButton" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_SENDBUTTON ) );
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }
    }

    private void InitShowBrowser( XComponentContext xContext )
    {
        try
        {
            GetPropSet( "BrowserCheck" ).setPropertyValue( "State", new Short( Helper.GetShowInBrowserByDefault( m_xContext ) ? (short)1 : (short)0 ) );
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }
    }

    public boolean show()
    {
        EnableControls( true );
        boolean bResult = super.show();

        if ( bResult && Helper.GetShowInBrowserByDefault( m_xContext ) )
           Helper.ShowURLInBrowser( m_xContext, m_sWikiEngineURL + "index.php?title=" + m_sWikiTitle );

        return bResult;
    }

    public synchronized void ThreadStop( boolean bSelf )
    {
        boolean bShowError = ( !bSelf && m_aThread != null && !m_bThreadFinished );

        super.ThreadStop( bSelf );

        if ( bShowError )
            Helper.ShowError( m_xContext,
                              m_xDialog,
                              Helper.DLG_SENDTITLE,
                              Helper.CANCELSENDING_ERROR,
                              null,
                              false );
    }

    public void fillWikiList()
    {
        String [] WikiList = m_aSettings.getWikiURLs();

        try
        {
            XPropertySet xPS = GetPropSet("WikiList");
            xPS.setPropertyValue("StringItemList", WikiList);
            // short [] nSel = new short[1];
            // nSel[0] = (short) m_aSettings.getLastUsedWikiServer();
            // xPS.setPropertyValue("SelectedItems", sel);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    public void fillDocList()
    {
        XPropertySet xPS = GetPropSet("ArticleText");
        try
        {
            short [] sel = (short[]) GetPropSet("WikiList").getPropertyValue("SelectedItems");
            xPS.setPropertyValue("StringItemList", m_aSettings.getWikiDocList(sel[0], 5));
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }


    public String GetWikiTitle()
    {
        return m_sWikiTitle;
    }

    public void SetWikiTitle(String sArticle)
    {
        m_sWikiTitle = sArticle;
        try
        {
            XPropertySet xPS = GetPropSet("ArticleText");
            xPS.setPropertyValue("Text", sArticle);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }


    public void switchSendButtonIfNecessary()
    {
        XPropertySet xSendButton = GetPropSet( "SendButton" );
        if ( xSendButton != null )
        {
            XPropertySet xWikiListProps = GetPropSet( "WikiList" );
            XPropertySet xArticleProps = GetPropSet( "ArticleText" );
            if ( xWikiListProps != null && xArticleProps != null )
            {
                try
                {
                    short [] pSel = (short[]) GetPropSet("WikiList").getPropertyValue("SelectedItems");
                    String sArticle = (String)xArticleProps.getPropertyValue( "Text" );
                    if ( pSel != null && pSel.length > 0 && sArticle != null && sArticle.length() != 0 )
                        xSendButton.setPropertyValue( "Enabled", Boolean.TRUE );
                    else
                        xSendButton.setPropertyValue( "Enabled", Boolean.FALSE );
                }
                catch (Exception ex)
                {
                    ex.printStackTrace();
                }
            }
        }
    }

    public void EnableControls( boolean bEnable )
    {
        try
        {
            String[] pControls = { "WikiList",
                                   "ArticleText",
                                   "CommentText",
                                   "MinorCheck",
                                   "BrowserCheck",
                                   "HelpButton",
                                   "AddButton" };

            for ( int nInd = 0; nInd < pControls.length; nInd++ )
                GetPropSet( pControls[nInd] ).setPropertyValue( "Enabled", new Boolean( bEnable ) );

            if ( bEnable )
            {
                SetFocusTo( "WikiList" );
                switchSendButtonIfNecessary();
            }
            else
            {
                GetPropSet( "SendButton" ).setPropertyValue( "Enabled", new Boolean( bEnable ) );
                SetFocusTo( "CancelButton" );
            }
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }
    }

    public boolean callHandlerMethod( XDialog xDialog, Object EventObject, String MethodName )
    {
        if ( MethodName.equals( sSendMethod ) )
        {
            try
            {
                XPropertySet aWikiListProps = GetPropSet( "WikiList" );
                XPropertySet aArticleTextProps = GetPropSet( "ArticleText" );
                XPropertySet aCommentTextProps = GetPropSet( "CommentText" );
                XPropertySet aMinorCheckProps = GetPropSet( "MinorCheck" );
                XPropertySet aBrowserCheckProps = GetPropSet( "BrowserCheck" );

                short [] sel = (short[]) aWikiListProps.getPropertyValue("SelectedItems");
                String [] items = (String []) aWikiListProps.getPropertyValue("StringItemList");
                m_sWikiEngineURL = items[sel[0]];
                m_aSettings.setLastUsedWikiServer(sel[0]);
                m_sWikiTitle = (String) aArticleTextProps.getPropertyValue("Text");
                m_sWikiComment = (String) aCommentTextProps.getPropertyValue("Text");

                short minorState = ((Short) aMinorCheckProps.getPropertyValue("State")).shortValue();
                if (minorState != 0)
                    m_bWikiMinorEdit = true;
                else
                    m_bWikiMinorEdit = false;

                short nBrowserState = ((Short) aBrowserCheckProps.getPropertyValue("State")).shortValue();
                Helper.SetShowInBrowserByDefault( m_xContext, nBrowserState != 0 );

                // allow to disable other buttons
                EnableControls( false );
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }

            final WikiPropDialog aThisDialog = this;
            final XDialog xDialogToClose = xDialog;
            final XComponentContext xContext = m_xContext;

            // start spinning
            SetThrobberVisible( true );
            SetThrobberActive( true );

            // the following method might show a dialog, should be used in main thread
            final Hashtable aWikiSettings = m_aSettings.getSettingByUrl( m_sWikiEngineURL );
            if ( Helper.AllowThreadUsage( m_xContext ) )
            {
                m_aThread = new Thread( "com.sun.star.thread.WikiEditorSendingThread" )
                {
                    public void run()
                    {
                        try
                        {
                            if ( m_aWikiEditor != null )
                            {
                                Thread.yield();
                                m_bAction = m_aWikiEditor.SendArticleImpl( aThisDialog, aWikiSettings );
                            }
                        }
                        finally
                        {
                            EnableControls( true );
                            SetThrobberActive( false );
                            SetThrobberVisible( false );

                            ThreadStop( true );
                            if ( m_bAction )
                                MainThreadDialogExecutor.Close( xContext, xDialogToClose );
                        }
                    }
                };

                m_aThread.start();
            }
            else
            {
                try
                {
                    if ( m_aWikiEditor != null )
                    {
                        m_bAction = m_aWikiEditor.SendArticleImpl( aThisDialog, aWikiSettings );
                    }
                } catch( java.lang.Exception e )
                {}
                finally
                {
                    EnableControls( true );
                    SetThrobberActive( false );
                    SetThrobberVisible( false );

                    if ( m_bAction )
                        xDialogToClose.endExecute();
                }
            }

            return true;
        }
        else if ( MethodName.equals( sWikiListMethod ) )
        {
            fillDocList();
            switchSendButtonIfNecessary();
            return true;
        }
        else if ( MethodName.equals( sArticleTextMethod ) )
        {
            switchSendButtonIfNecessary();
            return true;
        }
        else if ( MethodName.equals( sAddWikiMethod ) )
        {
            WikiEditSettingDialog xAddDialog = new WikiEditSettingDialog(m_xContext, "vnd.sun.star.script:WikiEditor.EditSetting?location=application");
            if ( xAddDialog.show() )
                fillWikiList();

            xAddDialog.DisposeDialog();

            return true;
        }

        return false;
    }

    public void windowClosed( EventObject e )
    {
        ThreadStop( false );
    }
}

