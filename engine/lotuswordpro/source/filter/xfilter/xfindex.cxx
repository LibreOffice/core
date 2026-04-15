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
 * Represents index source, index body and index entry.
 ************************************************************************/
#include <xfilter/ixfattrlist.hxx>
#include <xfilter/xfindex.hxx>

XFIndex::XFIndex()
    : m_eType(enumXFIndexTOC)
    , m_bProtect(true)
    , m_bSeparator(false)
{}

XFIndex::~XFIndex()
{
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
        pAttrList->AddAttribute( u"text:style-name"_ustr, GetStyleName() );
    if( m_bProtect )
        pAttrList->AddAttribute( u"text:protected"_ustr, u"true"_ustr );
    else
        pAttrList->AddAttribute( u"text:protected"_ustr, u"false"_ustr );

    pAttrList->AddAttribute( u"text:name"_ustr, m_strTitle );
    pStrm->StartElement( strIndexName );

    //text:table-of-content-source:
    pAttrList->Clear();

    if(m_eType == enumXFIndexTOC )
    {
        pAttrList->AddAttribute( u"text:outline-level"_ustr, OUString::number(10));
        pAttrList->AddAttribute( u"text:use-index-source-styles"_ustr, u"true"_ustr);
        pAttrList->AddAttribute( u"text:use-index-marks"_ustr, u"true"_ustr);
        pAttrList->AddAttribute( u"text:use-outline-level"_ustr, u"false"_ustr);
    }
    if (m_bSeparator)
        pAttrList->AddAttribute( u"text:alphabetical-separators"_ustr, u"true"_ustr );

    pStrm->StartElement( strSourceName );
    //title template:
    pAttrList->Clear();

    pStrm->StartElement( u"text:index-title-template"_ustr );
    pStrm->Characters( m_strTitle );
    pStrm->EndElement( u"text:index-title-template"_ustr );

    //entry templates:
    for (auto const& elem : m_aTemplates)
    {
        elem->SetTagName( strTplName);
        elem->ToXml(pStrm);
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
            pAttrList->AddAttribute( u"text:outline-level"_ustr, OUString::number(i));
            pStrm->StartElement( u"text:index-source-styles"_ustr );

            for (auto const& elemTOCSource : m_aTOCSource[i])
            {
                pAttrList->Clear();
                pAttrList->AddAttribute( u"text:style-name"_ustr, elemTOCSource);
                pStrm->StartElement( u"text:index-source-style"_ustr );
                pStrm->EndElement( u"text:index-source-style"_ustr );
            }
            pStrm->EndElement( u"text:index-source-styles"_ustr );
        }
    }

    pStrm->EndElement( strSourceName );

    //index-body:
    pAttrList->Clear();
    pStrm->StartElement( u"text:index-body"_ustr );
    //index-title:
    if(!m_strTitle.isEmpty())
    {
        pAttrList->AddAttribute( u"text:name"_ustr, m_strTitle + "_Head" );
        pStrm->StartElement( u"text:index-title"_ustr );
        pStrm->EndElement( u"text:index-title"_ustr );
    }

    XFContentContainer::ToXml(pStrm);
    pStrm->EndElement( u"text:index-body"_ustr );

    pStrm->EndElement( strIndexName );
}

void XFIndexTemplate::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( u"text:outline-level"_ustr, m_nLevel );
    pAttrList->AddAttribute( u"text:style-name"_ustr, m_strStyle );
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
                pAttrList->AddAttribute( u"text:style-name"_ustr, m_aEntries[i].second );

            pStrm->StartElement( u"text:index-entry-chapter-number"_ustr );
            pStrm->EndElement( u"text:index-entry-chapter-number"_ustr );
            break;
        case enumXFIndexTemplateText:
            if(!m_aEntries[i].second.isEmpty())
                pAttrList->AddAttribute( u"text:style-name"_ustr, m_aEntries[i].second );

            pStrm->StartElement( u"text:index-entry-text"_ustr );
            pStrm->EndElement( u"text:index-entry-text"_ustr );
            break;
        case enumXFIndexTemplateTab:
            if( m_eTabType != enumXFTabRight )
                pAttrList->AddAttribute( u"style:position"_ustr, OUString::number(m_fTabLength) + "cm" );

            if(!m_aEntries[i].second.isEmpty())
                pAttrList->AddAttribute( u"text:style-name"_ustr, m_aEntries[i].second );

            switch(m_eTabType)
            {
            case enumXFTabLeft:
                pAttrList->AddAttribute( u"style:type"_ustr, u"left"_ustr );
                break;
            case enumXFTabCenter:
                pAttrList->AddAttribute( u"style:type"_ustr, u"center"_ustr );
                break;
            case enumXFTabRight:
                pAttrList->AddAttribute( u"style:type"_ustr, u"right"_ustr );
                break;
            case enumXFTabChar:
                pAttrList->AddAttribute( u"style:type"_ustr, u"char"_ustr );
                break;
            default:
                break;
            }
            //delimiter:
            if( m_eTabType == enumXFTabChar )
                pAttrList->AddAttribute( u"style:char"_ustr, m_strTabDelimiter );
            //leader char:
            if( !m_strTabLeader.isEmpty() )
                pAttrList->AddAttribute( u"style:leader-char"_ustr, m_strTabLeader );

            pStrm->StartElement( u"text:index-entry-tab-stop"_ustr );
            pStrm->EndElement( u"text:index-entry-tab-stop"_ustr );
            break;
        case enumXFIndexTemplatePage:
            if(!m_aEntries[i].second.isEmpty())
                pAttrList->AddAttribute( u"text:style-name"_ustr, m_aEntries[i].second );

            pStrm->StartElement( u"text:index-entry-page-number"_ustr );
            pStrm->EndElement( u"text:index-entry-page-number"_ustr );
            break;
        case enumXFIndexTemplateLinkStart:
            pStrm->StartElement( u"text:index-entry-link-start"_ustr );
            pStrm->EndElement( u"text:index-entry-link-start"_ustr );
            break;
        case enumXFIndexTemplateLinkEnd:
            pStrm->StartElement( u"text:index-entry-link-end"_ustr );
            pStrm->EndElement( u"text:index-entry-link-end"_ustr );
            break;
        case enumXFIndexTemplateSpan:
            pAttrList->Clear();
            pAttrList->AddAttribute(u"text:style-name"_ustr, GetStyleName());
            pStrm->StartElement( u"text:index-entry-span"_ustr );
            pStrm->Characters(m_aTextEntries[i]);
            pStrm->EndElement( u"text:index-entry-span"_ustr );
            break;
        case enumXFIndexTemplateBibliography:
            pStrm->StartElement( u"text:index-entry-bibliography"_ustr );
            pStrm->EndElement( u"text:index-entry-bibliography"_ustr );
            break;
        default:
            break;
        }
    }

    pStrm->EndElement( m_strTagName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
