/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WikiPropDialog.java,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: mav $ $Date: 2008-01-30 19:02:16 $
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
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

public class WikiPropDialog extends WikiDialog{

    WikiEditorImpl m_aWikiEditor;

    private final String sSendMethod = "Send";
    private final String sCancelMethod = "Cancel";
    private final String sHelpMethod = "Help";
    private final String sLoadMethod = "Load";
    private final String sWikiListMethod = "WikiListChange";
    private final String sArticleTextMethod = "ArticleTextChange";
    private final String sAddWikiMethod = "AddWiki";

    String[] m_pMethods = {sSendMethod, sCancelMethod, sHelpMethod, sLoadMethod, sWikiListMethod, sArticleTextMethod, sAddWikiMethod};

    private String m_sWikiTitle = "";
    protected String m_sWikiEngineURL = "";
    protected String m_sWikiComment = "";
    protected boolean m_bWikiMinorEdit = false;

    private Thread m_aSendingThread;

    /** Creates a new instance of WikiPropDialog */
    public WikiPropDialog(XComponentContext xContext, String DialogURL, WikiEditorImpl aWikiEditorForThrobber )
    {
        super(xContext, DialogURL);
        super.setMethods(m_pMethods);

        if ( aWikiEditorForThrobber != null )
        {
            InsertThrobber();
            m_aWikiEditor = aWikiEditorForThrobber;
        }

        InitStrings( xContext );
        InitShowBrowser( xContext );
    }

    private void InitStrings( XComponentContext xContext )
    {
        try
        {
            SetTitle( Helper.GetLocalizedString( xContext, Helper.DLG_SENDTITLE ) );
            GetPropSet( "Label1" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_SENDTOMEDIAWIKI_LABEL1 ) );
            GetPropSet( "Label2" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_SENDTOMEDIAWIKI_LABEL2 ) );
            GetPropSet( "Label3" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_SENDTOMEDIAWIKI_LABEL3 ) );
            GetPropSet( "MinorCheck" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_SENDTOMEDIAWIKI_MINORCHECK ) );
            GetPropSet( "BrowserCheck" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_SENDTOMEDIAWIKI_BROWSERCHECK ) );
            GetPropSet( "HelpButton" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_HELP ) );
            GetPropSet( "CancelButton" ).setPropertyValue( "Label", Helper.GetLocalizedString( xContext, Helper.DLG_CANCEL ) );
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
                XPropertySet aHelpButtonProps = GetPropSet( "HelpButton" );
                XPropertySet aSendButtonProps = GetPropSet( "SendButton" );
                XPropertySet aAddButtonProps = GetPropSet( "AddButton" );

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

                XPropertySet[] aToDisable = { aWikiListProps, aArticleTextProps, aCommentTextProps, aMinorCheckProps, aBrowserCheckProps, aHelpButtonProps, aSendButtonProps, aAddButtonProps };
                for ( int nInd = 0; nInd < aToDisable.length; nInd++ )
                    aToDisable[nInd].setPropertyValue( "Enabled", Boolean.FALSE );
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }

            // TODO: In future the result of storing will be interesting
            // TODO: do not do it in OOo2.2
            final WikiPropDialog aThisDialog = this;
            final XDialog xDialogToClose = xDialog;

            // start spinning
            SetThrobberActive( true );

            if ( Helper.AllowThreadUsage( m_xContext ) )
            {
                m_aSendingThread = new Thread( "com.sun.star.thread.WikiEditorSendingThread" )
                {
                    public void run()
                    {
                        try
                        {
                            if ( m_aWikiEditor != null )
                            {
                                Thread.yield();
                                m_aWikiEditor.SendArticleImpl( aThisDialog );
                                m_bAction = true;
                            }
                        } catch( java.lang.Exception e )
                        {}
                        finally
                        {
                            xDialogToClose.endExecute();
                            Helper.AllowConnection( true );
                        }
                    }
                };

                m_aSendingThread.start();
            }
            else
            {
                try
                {
                    if ( m_aWikiEditor != null )
                    {
                        m_aWikiEditor.SendArticleImpl( aThisDialog );
                        m_bAction = true;
                    }
                } catch( java.lang.Exception e )
                {}
                finally
                {
                    xDialogToClose.endExecute();
                    Helper.AllowConnection( true );
                }
            }

            return true;
        }
        else if ( MethodName.equals( sLoadMethod ) )
        {
            try
            {
                short [] sel = (short[]) GetPropSet("WikiList").getPropertyValue("SelectedItems");
                String [] items = (String []) GetPropSet("WikiList").getPropertyValue("StringItemList");
                m_sWikiEngineURL = items[sel[0]];
                m_aSettings.setLastUsedWikiServer(sel[0]);
                m_sWikiTitle = (String) GetPropSet("ArticleText").getPropertyValue("Text");
            }
            catch (UnknownPropertyException ex)
            {
                ex.printStackTrace();
            }
            catch (WrappedTargetException ex)
            {
                ex.printStackTrace();
            }
            m_bAction = true;
            xDialog.endExecute();
            return true;
        }
        else if ( MethodName.equals( sCancelMethod ) )
        {
            // disallow any connection till the dialog is closed
            Helper.AllowConnection( false );

            if ( m_aSendingThread == null )
            {
                m_bAction = false;
                xDialog.endExecute();
            }
            else
            {
                Helper.ShowError( m_xContext,
                                  m_xDialog,
                                  Helper.DLG_SENDTITLE,
                                  Helper.CANCELSENDING_ERROR,
                                  null,
                                  false );
            }

            return true;
        }
        else if ( MethodName.equals( sHelpMethod ) )
        {
            m_bAction = false;
            //xDialog.endExecute();
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

            return true;
        }

        return false;
    }

    private void InsertThrobber()
    {
        try
        {
            XControl xDialogControl = ( XControl ) UnoRuntime.queryInterface( XControl.class, m_xDialog );
            XControlModel xDialogModel = null;
            if ( xDialogControl != null )
                xDialogModel = xDialogControl.getModel();

            XMultiServiceFactory xDialogFactory = ( XMultiServiceFactory ) UnoRuntime.queryInterface( XMultiServiceFactory.class, xDialogModel );
            if ( xDialogFactory != null )
            {
                XControlModel xThrobberModel = (XControlModel)UnoRuntime.queryInterface( XControlModel.class, xDialogFactory.createInstance( "com.sun.star.awt.UnoThrobberControlModel" ) );
                XPropertySet xThrobberProps = (XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, xThrobberModel );
                if ( xThrobberProps != null )
                {
                    xThrobberProps.setPropertyValue( "Name", "WikiThrobber" );
                    xThrobberProps.setPropertyValue( "PositionX", new Integer( 242 ) );
                    xThrobberProps.setPropertyValue( "PositionY", new Integer( 42 ) );
                    xThrobberProps.setPropertyValue( "Height", new Integer( 16 ) );
                    xThrobberProps.setPropertyValue( "Width", new Integer( 16 ) );

                    XNameContainer xDialogContainer = (XNameContainer)UnoRuntime.queryInterface( XNameContainer.class, xDialogModel );
                    xDialogContainer.insertByName( "WikiThrobber", xThrobberModel );
                }
            }
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }
    }

    public void SetThrobberActive( boolean bActive )
    {
        if ( m_xControlContainer != null )
        {
            try
            {
                XThrobber xThrobber = (XThrobber)UnoRuntime.queryInterface( XThrobber.class, m_xControlContainer.getControl( "WikiThrobber" ) );
                if ( xThrobber != null )
                {
                    if ( bActive )
                        xThrobber.start();
                    else
                        xThrobber.stop();
                }
            }
            catch( Exception e )
            {
                e.printStackTrace();
            }
        }
    }

}

