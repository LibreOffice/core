/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 * Represente index source,index body and index index entry.
 ************************************************************************/
#include "xfindex.hxx"
#include "xfparagraph.hxx"

XFIndex::XFIndex()
{
    m_pTitle = nullptr;
    m_eType = enumXFIndexTOC;
    m_bProtect = true;
    m_bSeparator = false;
    m_nMaxLevel = 0;

    for(sal_uInt16 i=1;i<MAX_TOC_LEVEL+1;i++)
    {
        m_aTOCSource[i].clear();
    }
}

XFIndex::~XFIndex()
{
    delete m_pTitle;

    for(sal_uInt16 i=1;i<MAX_TOC_LEVEL+1;i++)
    {
        m_aTOCSource[i].clear();
    }

    while(m_aTemplates.size()>0)
    {
        XFIndexTemplate * pTemplate = m_aTemplates.back();
        m_aTemplates.pop_back();
        delete pTemplate;
    }
}

void    XFIndex::AddTemplate(const OUString& level, const OUString& style, XFIndexTemplate* templ)
{
    templ->SetLevel( level );
    if(m_eType != enumXFIndexTOC) // TOC's styles are applied to template entries separately
    {
        templ->SetStyleName( style );
    }
    m_aTemplates.push_back( templ );
}

void    XFIndex::SetProtected(bool protect)
{
    m_bProtect = protect;
}

void    XFIndex::SetSeparator(bool sep)
{
    m_bSeparator = sep;
}

void XFIndex::AddTocSource(sal_uInt16 nLevel, const OUString& sStyleName)
{
    if (nLevel > MAX_TOC_LEVEL)
    {
        return;
    }

    m_aTOCSource[nLevel].push_back(sStyleName);
}

void    XFIndex::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    OUString strIndexName;
    OUString strTplName;
    OUString strSourceName;

    if(m_eType == enumXFIndexTOC )
    {
        strIndexName = "text:table-of-content";
        strSourceName = "text:table-of-content-source";
        strTplName = "text:table-of-content-entry-template";
    }
    else if( m_eType == enumXFIndexAlphabetical )
    {
        strIndexName = "text:alphabetical-index";
        strSourceName = "text:alphabetical-index-source";
        strTplName = "text:alphabetical-index-entry-template";
    }
    else if( m_eType == enumXFIndexUserIndex )
    {
        strIndexName = "text:user-index";
        strSourceName = "text:text:user-index-source";
        strTplName = "text:user-index-entry-template";
    }
    else if( m_eType == enumXFIndexObject )
    {
        strIndexName = "text:object-index";
        strSourceName = "text:object-index-source";
        strTplName = "text:object-index-entry-template";
    }
    else if( m_eType == enumXFIndexIllustration )
    {
        strIndexName = "text:illustration-index";
        strSourceName = "text:illustration-index-source";
        strTplName = "text:illustration-index-entry-template";
    }
    else if( m_eType == enumXFIndexTableIndex )
    {
        strIndexName = "text:table-index";
        strSourceName = "text:table-index-source";
        strTplName = "text:table-index-entry-template";
    }

    if( !GetStyleName().isEmpty() )
        pAttrList->AddAttribute( "text:style-name", GetStyleName() );
    if( m_bProtect )
        pAttrList->AddAttribute( "text:protected", "true" );
    else
        pAttrList->AddAttribute( "text:protected", "false" );

    pAttrList->AddAttribute( "text:name", m_strTitle );
    pStrm->StartElement( strIndexName );

    //text:table-of-content-source:
    pAttrList->Clear();

    if(m_eType == enumXFIndexTOC )
    {
        pAttrList->AddAttribute( "text:outline-level", OUString::number(10));
        pAttrList->AddAttribute( "text:use-index-source-styles", "true");
        pAttrList->AddAttribute( "text:use-index-marks", "true");
        pAttrList->AddAttribute( "text:use-outline-level", "false");
    }
    if (m_bSeparator)
        pAttrList->AddAttribute( "text:alphabetical-separators", "true" );

    pStrm->StartElement( strSourceName );
    //title template:
    pAttrList->Clear();

    pStrm->StartElement( "text:index-title-template" );
    pStrm->Characters( m_strTitle );
    pStrm->EndElement( "text:index-title-template" );

    //entry templates:
    std::vector<XFIndexTemplate *>::iterator it;
    for (it = m_aTemplates.begin(); it != m_aTemplates.end(); ++it)
    {
        (*it)->SetTagName( strTplName);
        (*it)->ToXml(pStrm);
    }

    // by
    if(m_eType == enumXFIndexTOC )
    {
        for(sal_uInt16 i=1; i <= MAX_TOC_LEVEL; i++)
        {
            if (m_aTOCSource[i].empty())
            {
                // unnecessary to output this level
                continue;
            }

            pAttrList->Clear();
            pAttrList->AddAttribute( "text:outline-level", OUString::number(i));
            pStrm->StartElement( "text:index-source-styles" );

            std::vector<OUString>::iterator it_str;
            for (it_str = m_aTOCSource[i].begin(); it_str != m_aTOCSource[i].end(); ++it_str)
            {
                pAttrList->Clear();
                pAttrList->AddAttribute( "text:style-name", *it_str);
                pStrm->StartElement( "text:index-source-style" );
                pStrm->EndElement( "text:index-source-style" );
            }
            pStrm->EndElement( "text:index-source-styles" );
        }
    }

    pStrm->EndElement( strSourceName );

    //index-body:
    pAttrList->Clear();
    pStrm->StartElement( "text:index-body" );
    //index-title:
    if(!m_strTitle.isEmpty())
    {
        pAttrList->AddAttribute( "text:name", m_strTitle + "_Head" );
        pStrm->StartElement( "text:index-title" );
        if( m_pTitle )
            m_pTitle->ToXml(pStrm);
        pStrm->EndElement( "text:index-title" );
    }

    XFContentContainer::ToXml(pStrm);
    pStrm->EndElement( "text:index-body" );

    pStrm->EndElement( strIndexName );
}

void XFIndexTemplate::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( "text:outline-level", m_nLevel );
    pAttrList->AddAttribute( "text:style-name", m_strStyle );
    if( m_strTagName.isEmpty() )
        m_strTagName = "text:table-of-content-entry-template";
    pStrm->StartElement( m_strTagName );

    for( size_t i=0; i<m_aEntries.size(); i++ )
    {
        pAttrList->Clear();

        switch( m_aEntries[i].first )
        {
        case enumXFIndexTemplateChapter:
            if(!m_aEntries[i].second.isEmpty())
                pAttrList->AddAttribute( "text:style-name", m_aEntries[i].second );

            pStrm->StartElement( "text:index-entry-chapter-number" );
            pStrm->EndElement( "text:index-entry-chapter-number" );
            break;
        case enumXFIndexTemplateText:
            if(!m_aEntries[i].second.isEmpty())
                pAttrList->AddAttribute( "text:style-name", m_aEntries[i].second );

            pStrm->StartElement( "text:index-entry-text" );
            pStrm->EndElement( "text:index-entry-text" );
            break;
        case enumXFIndexTemplateTab:
            if( m_eTabType != enumXFTabRight )
                pAttrList->AddAttribute( "style:position", OUString::number(m_fTabLength) + "cm" );

            if(!m_aEntries[i].second.isEmpty())
                pAttrList->AddAttribute( "text:style-name", m_aEntries[i].second );

            switch(m_eTabType)
            {
            case enumXFTabLeft:
                pAttrList->AddAttribute( "style:type", "left" );
                break;
            case enumXFTabCenter:
                pAttrList->AddAttribute( "style:type", "center" );
                break;
            case enumXFTabRight:
                pAttrList->AddAttribute( "style:type", "right" );
                break;
            case enumXFTabChar:
                pAttrList->AddAttribute( "style:type", "char" );
                break;
            default:
                break;
            }
            //delimiter:
            if( m_eTabType == enumXFTabChar )
                pAttrList->AddAttribute( "style:char", m_strTabDelimiter );
            //leader char:
            if( !m_strTabLeader.isEmpty() )
                pAttrList->AddAttribute( "style:leader-char", m_strTabLeader );

            pStrm->StartElement( "text:index-entry-tab-stop" );
            pStrm->EndElement( "text:index-entry-tab-stop" );
            break;
        case enumXFIndexTemplatePage:
            if(!m_aEntries[i].second.isEmpty())
                pAttrList->AddAttribute( "text:style-name", m_aEntries[i].second );

            pStrm->StartElement( "text:index-entry-page-number" );
            pStrm->EndElement( "text:index-entry-page-number" );
            break;
        case enumXFIndexTemplateLinkStart:
            pStrm->StartElement( "text:index-entry-link-start" );
            pStrm->EndElement( "text:index-entry-link-start" );
            break;
        case enumXFIndexTemplateLinkEnd:
            pStrm->StartElement( "text:index-entry-link-end" );
            pStrm->EndElement( "text:index-entry-link-end" );
            break;
        case enumXFIndexTemplateSpan:
            pAttrList->Clear();
            pAttrList->AddAttribute("text:style-name", GetStyleName());
            pStrm->StartElement( "text:index-entry-span" );
            pStrm->Characters(m_aTextEntries[i]);
            pStrm->EndElement( "text:index-entry-span" );
            break;
        case enumXFIndexTemplateBibliography:
            pStrm->StartElement( "text:index-entry-bibliography" );
            pStrm->EndElement( "text:index-entry-bibliography" );
            break;
        default:
            break;
        }
    }

    pStrm->EndElement( m_strTagName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
