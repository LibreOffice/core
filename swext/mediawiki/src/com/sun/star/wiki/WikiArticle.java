/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WikiArticle.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mav $ $Date: 2008-01-29 11:01:28 $
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

import com.sun.star.task.UrlRecord;
import java.io.*;
import java.util.Hashtable;
import javax.swing.text.html.*;
import com.sun.star.uno.XComponentContext;

import org.apache.commons.httpclient.*;
import org.apache.commons.httpclient.methods.*;


public class WikiArticle
{
    private XComponentContext m_xContext;

    private String m_sEditTime = "";
    private String m_sEditToken = "";

    protected String m_sHTMLCode;
    private boolean m_bNoArticle = true;

    protected String m_sWikiUser;
    protected String m_sWikiPass;

    protected String m_sTitle = "";

    private URI m_aMainURI;
    private HostConfiguration m_aHostConfig;


    /** Creates a new instance of WikiArticle */
    public WikiArticle( XComponentContext xContext, String sTitle, Hashtable wikiSettings, boolean bLogin, WikiPropDialog aPropDialog )
        throws java.net.MalformedURLException, com.sun.star.uno.Exception, java.io.IOException, WikiCancelException
    {
        m_xContext = xContext;

        String sMainUrl = (String) wikiSettings.get("Url");
        m_sWikiUser = (String) wikiSettings.get("Username");
        m_sWikiPass = (String) wikiSettings.get("Password");
        m_sTitle = sTitle;

        m_aMainURI = new URI( sMainUrl );

//         viewURL = sMainUrl + "index.php?title=" + m_sTitle;
//         editURL = sMainUrl + "index.php?title=" + m_sTitle + "&action=edit";
//         submitURL = sMainUrl + "index.php?title=" + m_sTitle + "&action=submit";
//         loginURL = sMainUrl + "index.php?title=Special:Userlogin";
//         loginSubmitURL = sMainUrl + "index.php?title=Special:Userlogin&action=submitlogin";

        boolean bGotLogin = false;
        if ( bLogin && m_sWikiUser.equals( "" ) && m_sWikiPass.equals( "" ) )
        {
            UrlRecord aRecord = Helper.GetUsersForURL( m_xContext, m_aMainURI.toString() );
            // TODO: there could be more users available, it should probably be possible to select from them
            // from other side, asking each time for the user name could disturb the user
            // For now the first acceptable user will be used.
            if ( aRecord != null && aRecord.UserList != null )
                for ( int nUserInd = 0; !bGotLogin && nUserInd < aRecord.UserList.length; nUserInd++ )
                    for ( int nPassInd = 0; !bGotLogin && nPassInd < aRecord.UserList[nUserInd].Passwords.length; nPassInd++ )
                    {
                        m_sWikiUser = aRecord.UserList[nUserInd].UserName;
                        m_sWikiPass = aRecord.UserList[nUserInd].Passwords[nPassInd];
                        bGotLogin = Login();
                    }
        }

        while( bLogin && !bGotLogin && !Login() )
        {
            // TODO: be sure that this is no main thread
            WikiEditSettingDialog wd = new WikiEditSettingDialog(m_xContext, "vnd.sun.star.script:WikiEditor.EditSetting?location=application", wikiSettings );

            if ( aPropDialog != null )
                aPropDialog.SetThrobberActive( false );

            if ( MainThreadDialogExecutor.Show( xContext, wd ) )
            {
                m_sWikiUser = (String) wikiSettings.get("Username");
                m_sWikiPass = (String) wikiSettings.get("Password");
            }
            else
                throw new WikiCancelException();

            if ( aPropDialog != null )
            {
                aPropDialog.SetThrobberActive( true );
                Thread.yield();
            }
        }

        // in case of loading the html contents are used
        // in case of saving the contents should be checked whether they are empty
        InitArticleHTML();

        // getArticleWiki();
    }

    public String GetMainURL()
    {
        return m_aMainURI.toString();
    }

    public String GetTitle()
    {
        return m_sTitle;
    }

    public String GetViewURL()
    {
        return m_aMainURI.toString() + "index.php?title=" + m_sTitle;
    }

    private String getArticleWiki()
        throws com.sun.star.uno.Exception, java.io.IOException, WikiCancelException
    {
        String sWikiCode = null;

        if ( m_aHostConfig != null )
        {
            URI aURI = new URI( m_aMainURI.toString() + "index.php?title=" + m_sTitle + "&action=edit" );
            GetMethod aRequest = new GetMethod( aURI.getEscapedPathQuery() );

            Helper.ExecuteMethod( aRequest, m_aHostConfig, aURI, m_xContext, false );

            int nResultCode = aRequest.getStatusCode();
            String sWebPage = null;
            if ( nResultCode == 200 )
                sWebPage = aRequest.getResponseBodyAsString();

            if ( sWebPage != null )
            {
                StringReader r = new StringReader(sWebPage);
                HTMLEditorKit.Parser parse = Helper.GetHTMLParser();
                EditPageParser callback = new EditPageParser();

                parse.parse(r,callback,true);
                m_sEditTime = callback.m_sEditTime;
                m_sEditToken = callback.m_sEditToken;

                int iPosStart = callback.m_nWikiArticleStart;
                int iPosEnd = callback.m_nWikiArticleEnd;

                if ( iPosStart >= 0 && iPosEnd > 0 )
                {
                    String sArticle = sWebPage.substring(iPosStart, iPosEnd);
                    iPosStart = sArticle.indexOf(">") + 1;
                    sWikiCode = sArticle.substring( iPosStart, sArticle.length() );
                }
            }
        }

        return sWikiCode;
    }

    private void InitArticleHTML()
        throws com.sun.star.uno.Exception, java.io.IOException, WikiCancelException
    {
        if ( m_aHostConfig != null )
        {
            URI aURI = new URI( m_aMainURI.toString() + "index.php?title=" + m_sTitle );
            GetMethod aRequest = new GetMethod( aURI.getEscapedPathQuery() );

            Helper.ExecuteMethod( aRequest, m_aHostConfig, aURI, m_xContext, false );

            int nResultCode = aRequest.getStatusCode();
            String sWebPage = null;
            if ( nResultCode == 200 )
                sWebPage = aRequest.getResponseBodyAsString();

            if ( sWebPage != null )
            {
                StringReader r = new StringReader(sWebPage);
                HTMLEditorKit.Parser parse = Helper.GetHTMLParser();
                EditPageParser callback = new EditPageParser();

                parse.parse(r,callback,true);

                int iPosStart = callback.m_nHTMLArticleStart;
                int iPosEnd = callback.m_nHTMLArticleEnd;
                int nPosNoArt = callback.m_nNoArticleInd;

                if ( iPosStart >= 0 && iPosEnd > 0 )
                {
                    m_sHTMLCode = sWebPage.substring(iPosStart, iPosEnd);
                    m_bNoArticle = ( nPosNoArt >= 0 && nPosNoArt >= iPosStart && nPosNoArt <= iPosEnd );
                }
            }
        }
    }

    protected boolean setArticle( String sWikiCode, String sWikiComment, boolean bMinorEdit )
        throws com.sun.star.uno.Exception, java.io.IOException, WikiCancelException
    {
        boolean bResult = false;

        if ( m_aHostConfig != null && sWikiCode != null && sWikiComment != null )
        {
            // get the edit time and token
            getArticleWiki();

            URI aURI = new URI( m_aMainURI.getPath() + "index.php?title=" + m_sTitle + "&action=submit" );
            PostMethod aPost = new PostMethod();
            aPost.setPath( aURI.getEscapedPathQuery() );

            // aPost.addParameter( "title", m_sTitle );
            // aPost.addParameter( "action", "submit" );
            aPost.addParameter( "wpTextbox1", sWikiCode );
            aPost.addParameter( "wpSummary", sWikiComment );
            aPost.addParameter( "wpSection", "" );
            aPost.addParameter( "wpEdittime", m_sEditTime );
            aPost.addParameter( "wpSave", "Save page" );
            aPost.addParameter( "wpEditToken", m_sEditToken );

            if ( bMinorEdit )
                aPost.addParameter( "wpMinoredit", "1" );

            Helper.ExecuteMethod( aPost, m_aHostConfig, aURI, m_xContext, false );

            int nResultCode = aPost.getStatusCode();
            if ( nResultCode < 400 )
                bResult = true;

            String aResult = aPost.getResponseBodyAsString();

            // TODO: remove the debug printing, try to detect the error
            System.out.print( "nSubmitCode = " + nResultCode + "\n===\n" + aResult );
        }

        return bResult;
    }

    protected boolean Login()
        throws com.sun.star.uno.Exception, java.io.IOException, WikiCancelException
    {
        m_aHostConfig = Helper.Login( m_aMainURI, m_sWikiUser, m_sWikiPass, m_xContext );
        return ( m_aHostConfig != null );
    }

    protected void cleanHTML()
    {
        if ( m_sHTMLCode != null )
        {
            //Welcome to regex hell ;)

            //strip comments
            m_sHTMLCode = m_sHTMLCode.replaceAll("\\<![ \\r\\n\\t]*(--([^\\-]|[\\r\\n]|-[^\\-])*--[ \\r\\n\\t]*)\\>","");

            //strip edit section links
            m_sHTMLCode = m_sHTMLCode.replaceAll("\\<div class=\"editsection\".*?\\</div\\>","");

            //strip huge spaces
            m_sHTMLCode = m_sHTMLCode.replaceAll("\\<p\\>\\<br /\\>[ \r\n\t]*?\\</p\\>","");

            //strip toc
            m_sHTMLCode = m_sHTMLCode.replaceAll("\\<table.*id=\"toc\"(.|[\r\n])*?\\</table\\>","");

            //strip jump-to-nav
            m_sHTMLCode = m_sHTMLCode.replaceAll("\\<div id=\"jump-to-nav\".*?\\</div\\>","");

            //strip Javascript
            m_sHTMLCode = m_sHTMLCode.replaceAll("\\<script(.|[\r\n])*?\\</script\\>","");
        }
    }


    protected boolean NotExist()
    {
        boolean bResult = true;
        if ( m_sHTMLCode != null )
            bResult = m_bNoArticle;

        return bResult;
    }

}
