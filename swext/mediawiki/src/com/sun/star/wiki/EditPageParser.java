/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EditPageParser.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mav $ $Date: 2007-11-28 11:13:17 $
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

import javax.swing.text.html.*;
import javax.swing.text.MutableAttributeSet;

public class EditPageParser extends HTMLEditorKit.ParserCallback
{

    protected String m_sEditTime = "";
    protected String m_sEditToken = "";
    private int m_nWikiArticleHash = 0;
    private boolean m_bHTMLStartFound = false;
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

    public void handleComment(char[] data,int pos)
    {
        // insert code to handle comments
    }

    public void handleEndTag(HTML.Tag t,int pos)
    {
        if (t == HTML.Tag.TEXTAREA)
        {
            m_nWikiArticleEnd = pos;
        }
        if (t == HTML.Tag.DIV)
        {
            if (m_bHTMLStartFound)
            {
                m_nHTMLArticleStart = pos+6;
                m_bHTMLStartFound = false;
            }
        }
    }

    public void handleError(String errorMsg,int pos)
    {
        //System.out.println(errorMsg);
    }

    public void handleSimpleTag(HTML.Tag t, MutableAttributeSet a,int pos)
    {
        // insert code to handle simple tags

        if (t == HTML.Tag.INPUT)
        {
            String sName = (String) a.getAttribute(HTML.Attribute.NAME);
            if (sName != null)
            {
                if (sName.equalsIgnoreCase("wpEdittime"))
                {
                    this.m_sEditTime = (String) a.getAttribute(HTML.Attribute.VALUE);
                }
                if (sName.equalsIgnoreCase("wpEditToken"))
                {
                    this.m_sEditToken = (String) a.getAttribute(HTML.Attribute.VALUE);
                }
            }

        }

    }

    public void handleStartTag(HTML.Tag t, MutableAttributeSet a,int pos)
    {
        // insert code to handle starting tags
        String sName = "";
        String sId = "";
        String sClass = "";

        if (t == HTML.Tag.TEXTAREA)
        {
            sName = (String) a.getAttribute(HTML.Attribute.NAME);
            if (sName != null)
            {
                if (sName.equalsIgnoreCase("wpTextbox1"))
                {
                    m_nWikiArticleHash = t.hashCode();
                    m_nWikiArticleStart = pos;
                }
            }
        }
        else if (t == HTML.Tag.DIV)
        {
            sId = (String) a.getAttribute(HTML.Attribute.ID);
            sClass = (String) a.getAttribute(HTML.Attribute.CLASS);
            if (sId != null)
            {
                if (sId.equalsIgnoreCase("contentSub"))
                {
                    m_bHTMLStartFound = true;
                }
            }
            if (sClass != null)
            {
                if (sClass.equalsIgnoreCase("printfooter"))
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
            sClass = (String) a.getAttribute(HTML.Attribute.CLASS);
            if ( sClass != null && sClass.equalsIgnoreCase( "error" ) )
            {
                m_nErrorInd = pos;
            }
        }
    }


}
