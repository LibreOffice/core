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

import java.io.StringReader;
import java.io.OutputStreamWriter;
import java.util.Map;
import java.net.URLEncoder;
import java.net.URI;
import java.net.HttpURLConnection;

import javax.swing.text.html.HTMLEditorKit;

import com.sun.star.uno.XComponentContext;


public class WikiArticle
{
    private final XComponentContext m_xContext;

    private String m_sEditTime = "";
    private String m_sEditToken = "";

    private String m_sHTMLCode;
    private boolean m_bNoArticle = true;

    private String m_sWikiUser;
    private String m_sWikiPass;

    private final String m_sTitle;

    private final URI m_aMainURI;
    private boolean m_isLoggedIn = false;

    /** Creates a new instance of WikiArticle */
    public WikiArticle( XComponentContext xContext, String sTitle, Map<String,String> wikiSettings, boolean bLogin, WikiPropDialog aPropDialog )
        throws java.net.URISyntaxException, java.io.IOException, WikiCancelException
    {
        m_xContext = xContext;

        String sMainUrl = wikiSettings.get("Url");
        m_sWikiUser = wikiSettings.get("Username");
        m_sWikiPass = wikiSettings.get("Password");
        m_sTitle = sTitle;

        m_aMainURI = new URI(sMainUrl);

        if ( bLogin )
        {
            WikiEditSettingDialog aDialog = new WikiEditSettingDialog(m_xContext, "vnd.sun.star.script:WikiEditor.EditSetting?location=application", wikiSettings, false );
            try
            {
                while( !Login() )
                {
                    if ( aPropDialog != null )
                        aPropDialog.SetThrobberActive( false );

                    if ( MainThreadDialogExecutor.Show( xContext, aDialog ) )
                    {
                        m_sWikiUser = wikiSettings.get("Username");
                        m_sWikiPass = wikiSettings.get("Password");
                    }
                    else
                        throw new WikiCancelException();

                    if ( aPropDialog != null )
                    {
                        aPropDialog.SetThrobberActive( true );
                        Thread.yield();
                    }
                }
            }
            finally
            {
                aDialog.DisposeDialog();
            }
        }

        // in case of loading the html contents are used
        // in case of saving the contents should be checked whether they are empty
        InitArticleHTML();
    }

    public String GetMainURL()
    {
        return m_aMainURI.toString();
    }

    public String GetTitle()
    {
        return m_sTitle;
    }



    private String getArticleWiki()
        throws java.net.URISyntaxException, java.io.IOException, WikiCancelException
    {
        String sWikiCode = null;

        if (m_isLoggedIn)
        {
            URI aURI = new URI(m_aMainURI.toString() + "index.php?title=" + m_sTitle + "&action=edit");
            HttpURLConnection connGet = Helper.PrepareMethod("GET", aURI, m_xContext);
            connGet.connect();

            int nResultCode = connGet.getResponseCode();
            String sWebPage = null;
            if (nResultCode == 200) {
                sWebPage = Helper.ReadResponseBody(connGet);
            }

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
                    iPosStart = sArticle.indexOf('>') + 1;
                    sWikiCode = sArticle.substring( iPosStart, sArticle.length() );
                }
            }
        }

        return sWikiCode;
    }

    private void InitArticleHTML()
        throws java.net.URISyntaxException, java.io.IOException, WikiCancelException
    {
        if (m_isLoggedIn)
        {
            URI uri = new URI(m_aMainURI.toString() + "index.php?title=" + m_sTitle);
            HttpURLConnection connGet = Helper.PrepareMethod("GET", uri, m_xContext);
            connGet.connect();

            int nResultCode = connGet.getResponseCode();
            String sWebPage = null;
            if (nResultCode == 200) {
                sWebPage = Helper.ReadResponseBody(connGet);
            }

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
        throws java.net.URISyntaxException, java.io.IOException, WikiCancelException
    {
        boolean bResult = false;

        if (m_isLoggedIn && sWikiCode != null && sWikiComment != null)
        {
            // get the edit time and token
            getArticleWiki();

            URI uri = new URI(m_aMainURI.toString() + "index.php?title=" + m_sTitle + "&action=submit");

            HttpURLConnection connPost = Helper.PrepareMethod("POST", uri, m_xContext);
            connPost.setDoInput(true);
            connPost.setDoOutput(true);
            connPost.connect();

            OutputStreamWriter post = new OutputStreamWriter(connPost.getOutputStream());
            post.write("wpTextbox1=");
            post.write(URLEncoder.encode(sWikiCode, "UTF-8"));
            post.write("&wpSummary=");
            post.write(URLEncoder.encode(sWikiComment, "UTF-8"));
            post.write("&wpSection=");
            post.write("&wpEdittime=");
            post.write(URLEncoder.encode(m_sEditTime, "UTF-8"));
            post.write("&wpSave=Save%20page");
            post.write("&wpEditToken=");
            post.write(URLEncoder.encode(m_sEditToken, "UTF-8"));

            if (bMinorEdit) {
                post.write("&wpMinoredit=1");
            }

            post.flush();
            post.close();

            int nResultCode = connPost.getResponseCode();
            if ( nResultCode < 400 )
                bResult = true;

            String aResult = Helper.ReadResponseBody(connPost);

            // TODO: remove the debug printing, try to detect the error
            System.out.print( "nSubmitCode = " + nResultCode + "\n===\n" + aResult );
        }

        return bResult;
    }

    private boolean Login()
        throws java.net.URISyntaxException, java.io.IOException, WikiCancelException
    {
        m_isLoggedIn = Helper.Login( m_aMainURI, m_sWikiUser, m_sWikiPass, m_xContext );
        return m_isLoggedIn;
    }

    protected boolean NotExist()
    {
        boolean bResult = true;
        if ( m_sHTMLCode != null )
            bResult = m_bNoArticle;

        return bResult;
    }

}
