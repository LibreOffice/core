/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

import javax.swing.text.html.*;
import javax.swing.text.MutableAttributeSet;

public class EditPageParser extends HTMLEditorKit.ParserCallback
{

    protected String m_sEditTime = "";
    protected String m_sEditToken = "";
    protected String m_sLoginToken = "";
    protected String m_sMainURL = "";

    private int m_nWikiArticleHash = 0;
    private boolean m_bHTMLStartFound = false;
    private boolean m_bInHead = false;

    protected int m_nWikiArticleStart = -1;
    protected int m_nWikiArticleEnd = -1;
    protected int m_nHTMLArticleStart = -1;
    protected int m_nHTMLArticleEnd = -1;
    protected int m_nNoArticleInd = -1;
    protected int m_nErrorInd = -1;

    /** Creates a new instance of WikiHTMLParser */
    public EditPageParser()
    {
    }

    public void handleComment( char[] data,int pos )
    {
        // insert code to handle comments
    }

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

    public void handleError( String errorMsg,int pos )
    {
        //System.out.println( errorMsg );
    }

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
                      && ( m_sMainURL.length() == 0 || sName.indexOf( "action=edit" ) >= 0 ) )
                    {
                        m_sMainURL = sName.substring( 0, nIndexStart );
                    }
                }
            }
        }

    }

    public void handleStartTag( HTML.Tag t, MutableAttributeSet a,int pos )
    {
        // insert code to handle starting tags
        String sName = "";
        String sId = "";
        String sClass = "";

        if ( t == HTML.Tag.HEAD )
        {
            m_bInHead = true;
        }
        if ( t == HTML.Tag.TEXTAREA )
        {
            sName = ( String ) a.getAttribute( HTML.Attribute.NAME );
            if ( sName != null )
            {
                if ( sName.equalsIgnoreCase( "wpTextbox1" ) )
                {
                    m_nWikiArticleHash = t.hashCode();
                    m_nWikiArticleStart = pos;
                }
            }
        }
        else if ( t == HTML.Tag.DIV )
        {
            sId = ( String ) a.getAttribute( HTML.Attribute.ID );
            sClass = ( String ) a.getAttribute( HTML.Attribute.CLASS );
            if ( sId != null )
            {
                if ( sId.equalsIgnoreCase( "contentSub" ) )
                {
                    m_bHTMLStartFound = true;
                }
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
