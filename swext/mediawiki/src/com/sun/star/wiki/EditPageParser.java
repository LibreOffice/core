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

import javax.swing.text.html.*;
import javax.swing.text.MutableAttributeSet;

public class EditPageParser extends HTMLEditorKit.ParserCallback
{

    protected String m_sEditTime = "";
    protected String m_sEditToken = "";
    protected String m_sLoginToken = "";
    protected String m_sMainURL = "";

    private boolean m_bHTMLStartFound = false;
    private boolean m_bInHead = false;

    protected int m_nWikiArticleStart = -1;
    protected int m_nWikiArticleEnd = -1;
    protected int m_nHTMLArticleStart = -1;
    protected int m_nHTMLArticleEnd = -1;
    protected int m_nNoArticleInd = -1;
    protected int m_nErrorInd = -1;

    @Override
    public void handleComment( char[] data,int pos )
    {
        // insert code to handle comments
    }

    @Override
    public void handleEndTag( HTML.Tag t,int pos )
    {
        if ( t == HTML.Tag.TEXTAREA )
        {
            m_nWikiArticleEnd = pos;
        }
        else if ( t == HTML.Tag.DIV )
        {
            if ( m_bHTMLStartFound )
            {
                m_nHTMLArticleStart = pos+6;
                m_bHTMLStartFound = false;
            }
        }
        else if ( t == HTML.Tag.HEAD )
        {
            m_bInHead = false;
        }
    }

    @Override
    public void handleError( String errorMsg,int pos )
    {
    }

    @Override
    public void handleSimpleTag( HTML.Tag t, MutableAttributeSet a,int pos )
    {
        // insert code to handle simple tags

        if ( t == HTML.Tag.INPUT )
        {
            String sName = ( String ) a.getAttribute( HTML.Attribute.NAME );
            if ( sName != null )
            {
                if ( sName.equalsIgnoreCase( "wpEdittime" ) )
                {
                    this.m_sEditTime = ( String ) a.getAttribute( HTML.Attribute.VALUE );
                }
                else if ( sName.equalsIgnoreCase( "wpEditToken" ) )
                {
                    this.m_sEditToken = ( String ) a.getAttribute( HTML.Attribute.VALUE );
                }
                else if ( sName.equalsIgnoreCase( "wpLoginToken" ) )
                {
                    this.m_sLoginToken = ( String ) a.getAttribute( HTML.Attribute.VALUE );
                }
            }

        }
        else if ( t == HTML.Tag.LINK )
        {
            if ( m_bInHead )
            {
                String sName = ( String ) a.getAttribute( HTML.Attribute.HREF );
                if ( sName != null )
                {
                    int nIndexStart = sName.indexOf( "index.php" );
                    // get the main URL from the first header-link with index.php
                    // the link with "action=edit" inside is preferable
                    if ( nIndexStart>= 0
                      && ( m_sMainURL.length() == 0 || sName.contains("action=edit") ) )
                    {
                        m_sMainURL = sName.substring( 0, nIndexStart );
                    }
                }
            }
        }

    }

    @Override
    public void handleStartTag( HTML.Tag t, MutableAttributeSet a,int pos )
    {
        // insert code to handle starting tags
        String sClass;

        if ( t == HTML.Tag.HEAD )
        {
            m_bInHead = true;
        }
        else if ( t == HTML.Tag.TEXTAREA )
        {
            String sName = ( String ) a.getAttribute( HTML.Attribute.NAME );
            if ( sName != null && sName.equalsIgnoreCase( "wpTextbox1" ) )
            {
                m_nWikiArticleStart = pos;
            }
        }
        else if ( t == HTML.Tag.DIV )
        {
            String sId = ( String ) a.getAttribute( HTML.Attribute.ID );
            sClass = ( String ) a.getAttribute( HTML.Attribute.CLASS );
            if ( sId != null  && sId.equalsIgnoreCase( "contentSub" ) )
            {
                m_bHTMLStartFound = true;
            }
            if ( sClass != null )
            {
                if ( sClass.equalsIgnoreCase( "printfooter" ) )
                {
                    m_nHTMLArticleEnd = pos;
                }
                else if ( sClass.equalsIgnoreCase( "noarticletext" ) )
                {
                    m_nNoArticleInd = pos;
                }
                else if ( sClass.equalsIgnoreCase( "errorbox" ) )
                {
                    m_nErrorInd = pos;
                }
            }
        }
        else if ( t == HTML.Tag.P )
        {
            sClass = ( String ) a.getAttribute( HTML.Attribute.CLASS );
            if ( sClass != null && sClass.equalsIgnoreCase( "error" ) )
            {
                m_nErrorInd = pos;
            }
        }
    }


}
