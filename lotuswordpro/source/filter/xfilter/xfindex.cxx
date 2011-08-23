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
/*************************************************************************
 * Change History
   2005-02-02	create and implemente.
 ************************************************************************/
#include	"xfindex.hxx"
#include	"xfparagraph.hxx"

XFIndex::XFIndex()
{
    m_pTitle = NULL;
    m_eType = enumXFIndexTOC;
    m_bProtect = sal_True;
    m_bSeparator = sal_False;
    m_nMaxLevel = 0;

    for(sal_uInt16 i=1;i<MAX_TOC_LEVEL+1;i++)
    {
        m_aTOCSource[i].clear();
    }
}

XFIndex::~XFIndex()
{
    if( m_pTitle )
        delete m_pTitle;

    for(sal_uInt16 i=1;i<MAX_TOC_LEVEL+1;i++)
    {
        m_aTOCSource[i].clear();
    }

    while(m_aTemplates.size()>0)
    {
        XFIndexTemplate * pTemplate = m_aTemplates.back();
        m_aTemplates.pop_back();
        delete pTemplate;pTemplate=NULL;
    }
}

void XFIndex::SetSectionStyle(rtl::OUString strSectStyleName)
{
    SetStyleName(strSectStyleName);
}

void	XFIndex::SetTitle(rtl::OUString title, rtl::OUString strParaStyle)
{
    m_strTitle = title;

    m_pTitle = new XFParagraph();
    m_pTitle->SetStyleName(strParaStyle);
    m_pTitle->Add(m_strTitle);
}

void	XFIndex::AddTemplate(sal_uInt32 level,
                    rtl::OUString /*style*/,
                    enumXFIndexTemplate type1,
                    enumXFIndexTemplate type2,
                    enumXFIndexTemplate type3,
                    enumXFIndexTemplate type4 )
{
    if( level<1 )
        return;

    if (level > m_nMaxLevel)
        m_nMaxLevel = level;

    XFIndexTemplate * templ = new XFIndexTemplate();

    templ->AddEntry(type1);
    templ->AddEntry(type2);
    templ->AddEntry(type3);
    templ->AddEntry(type4);
    templ->SetLevel( rtl::OUString::valueOf( ( sal_Int64 ) level ) );

    m_aTemplates.push_back( templ );
}

void	XFIndex::AddTemplate(rtl::OUString level, rtl::OUString style, XFIndexTemplate* templ)
{
//	if( level<1 || !templ )
//		return;

//	if (level > m_nMaxLevel)
//		m_nMaxLevel = level;

    templ->SetLevel( level );
    if(m_eType != enumXFIndexTOC) // TOC's styles are applied to template entries separately
    {
        templ->SetStyleName( style );
    }
    m_aTemplates.push_back( templ );
}

void	XFIndex::AddEntry(rtl::OUString entry, rtl::OUString strParaStyle)
{
    XFParagraph *pPara = new XFParagraph();
    pPara->SetStyleName(strParaStyle);
    pPara->Add(entry);
    pPara->Add(new XFTabStop());
    XFContentContainer::Add(pPara);
}

sal_Int32	XFIndex::GetEntryCount()
{
    return GetCount();
}

void	XFIndex::SetProtected(sal_Bool protect)
{
    m_bProtect = protect;
}

void	XFIndex::SetSeparator(sal_Bool sep)
{
    m_bSeparator = sep;
}

void XFIndex::AddTocSource(sal_uInt16 nLevel, const rtl::OUString sStyleName)
{
    if (nLevel > MAX_TOC_LEVEL)
    {
        return;
    }

    m_aTOCSource[nLevel].push_back(sStyleName);
}

void XFIndex::SetDefaultAlphaIndex(rtl::OUString /*strDivision*/,sal_Bool bRunin, sal_Bool bSeparator)
{
    XFIndexTemplate * pTemplateSep = new XFIndexTemplate();
    if (bSeparator)
    {
        SetSeparator(sal_True);
        pTemplateSep->AddEntry(enumXFIndexTemplateText,A2OUSTR(""));
    }
    AddTemplate(A2OUSTR("separator"),A2OUSTR("Separator"),pTemplateSep);

    XFIndexTemplate * pTemplate1 = new XFIndexTemplate();
    pTemplate1->AddEntry(enumXFIndexTemplateText,A2OUSTR(""));
    pTemplate1->AddEntry(enumXFIndexTemplateTab,A2OUSTR(""));
    pTemplate1->AddEntry(enumXFIndexTemplatePage,A2OUSTR(""));
    AddTemplate(Int32ToOUString(1),A2OUSTR("Primary"),pTemplate1);

    XFIndexTemplate * pTemplate2 = new XFIndexTemplate();
    pTemplate2->AddEntry(enumXFIndexTemplateText,A2OUSTR(""));
    pTemplate2->AddEntry(enumXFIndexTemplateTab,A2OUSTR(""));
    pTemplate2->AddEntry(enumXFIndexTemplatePage,A2OUSTR(""));
    XFIndexTemplate * pTemplate3 = new XFIndexTemplate();
    pTemplate3->AddEntry(enumXFIndexTemplateText,A2OUSTR(""));
    pTemplate3->AddEntry(enumXFIndexTemplateTab,A2OUSTR(""));
    pTemplate3->AddEntry(enumXFIndexTemplatePage,A2OUSTR(""));

    if (bRunin)
    {
        AddTemplate(Int32ToOUString(2),A2OUSTR("Primary"),pTemplate2);
        AddTemplate(Int32ToOUString(3),A2OUSTR("Primary"),pTemplate3);
    }
    else
    {
        AddTemplate(Int32ToOUString(2),A2OUSTR("Secondary"),pTemplate2);
        AddTemplate(Int32ToOUString(3),A2OUSTR("Secondary"),pTemplate3);
    }
}

void	XFIndex::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    rtl::OUString strIndexName;
    rtl::OUString strTplName;
    rtl::OUString strSourceName;

    if(m_eType == enumXFIndexTOC )
    {
        strIndexName = A2OUSTR("text:table-of-content");
        strSourceName = A2OUSTR("text:table-of-content-source");
        strTplName = A2OUSTR("text:table-of-content-entry-template");
    }
    else if( m_eType == enumXFIndexAlphabetical )
    {
        strIndexName = A2OUSTR("text:alphabetical-index");
        strSourceName = A2OUSTR("text:alphabetical-index-source");
        strTplName = A2OUSTR("text:alphabetical-index-entry-template");
    }
    else if( m_eType == enumXFIndexUserIndex )
    {
        strIndexName = A2OUSTR("text:user-index");
        strSourceName = A2OUSTR("text:text:user-index-source");
        strTplName = A2OUSTR("text:user-index-entry-template");
    }
    else if( m_eType == enumXFIndexObject )
    {
        strIndexName = A2OUSTR("text:object-index");
        strSourceName = A2OUSTR("text:object-index-source");
        strTplName = A2OUSTR("text:object-index-entry-template");
    }
    else if( m_eType == enumXFIndexIllustration )
    {
        strIndexName = A2OUSTR("text:illustration-index");
        strSourceName = A2OUSTR("text:illustration-index-source");
        strTplName = A2OUSTR("text:illustration-index-entry-template");
    }
    else if( m_eType == enumXFIndexTableIndex )
    {
        strIndexName = A2OUSTR("text:table-index");
        strSourceName = A2OUSTR("text:table-index-source");
        strTplName = A2OUSTR("text:table-index-entry-template");
    }

    if( GetStyleName().getLength() > 0 )
        pAttrList->AddAttribute( A2OUSTR("text:style-name"), GetStyleName() );
    if( m_bProtect )
        pAttrList->AddAttribute( A2OUSTR("text:protected"), A2OUSTR("true") );
    else
        pAttrList->AddAttribute( A2OUSTR("text:protected"), A2OUSTR("false") );

    pAttrList->AddAttribute( A2OUSTR("text:name"), m_strTitle );
    pStrm->StartElement( strIndexName );

    //text:table-of-content-source:
    pAttrList->Clear();

    if(m_eType == enumXFIndexTOC )
    {
        pAttrList->AddAttribute( A2OUSTR("text:outline-level"), Int32ToOUString(10));
        pAttrList->AddAttribute( A2OUSTR("text:use-index-source-styles"), A2OUSTR("true"));
        pAttrList->AddAttribute( A2OUSTR("text:use-index-marks"), A2OUSTR("true"));
        pAttrList->AddAttribute( A2OUSTR("text:use-outline-level"), A2OUSTR("false"));
    }
    if (m_bSeparator)
        pAttrList->AddAttribute( A2OUSTR("text:alphabetical-separators"), A2OUSTR("true") );

    pStrm->StartElement( strSourceName );
    //title template:
    pAttrList->Clear();

    pStrm->StartElement( A2OUSTR("text:index-title-template") );
    pStrm->Characters( m_strTitle );
    pStrm->EndElement( A2OUSTR("text:index-title-template") );

    //entry templates:
    std::vector<XFIndexTemplate *>::iterator it;
    for (it = m_aTemplates.begin(); it != m_aTemplates.end(); it++)
    {
        (*it)->SetTagName( strTplName);
        (*it)->ToXml(pStrm);
    }

    // by
    if(m_eType == enumXFIndexTOC )
    {
        for(sal_uInt16 i=1; i <= MAX_TOC_LEVEL; i++)
        {
            if (m_aTOCSource[i].size() == 0)
            {
                // unnecessary to output this level
                continue;
            }

            pAttrList->Clear();
            pAttrList->AddAttribute( A2OUSTR("text:outline-level"), Int32ToOUString(i));
            pStrm->StartElement( A2OUSTR("text:index-source-styles") );

            std::vector<rtl::OUString>::iterator it_str;
            for (it_str = m_aTOCSource[i].begin(); it_str != m_aTOCSource[i].end(); it++)
            {
                pAttrList->Clear();
                pAttrList->AddAttribute( A2OUSTR("text:style-name"), *it_str);
                pStrm->StartElement( A2OUSTR("text:index-source-style") );
                pStrm->EndElement( A2OUSTR("text:index-source-style") );
            }
            pStrm->EndElement( A2OUSTR("text:index-source-styles") );
        }
    }

    pStrm->EndElement( strSourceName );

    //index-body:
    pAttrList->Clear();
    pStrm->StartElement( A2OUSTR("text:index-body") );
    //index-title:
    if(m_strTitle.getLength())
    {
        pAttrList->AddAttribute( A2OUSTR("text:name"), m_strTitle + A2OUSTR("_Head") );
        pStrm->StartElement( A2OUSTR("text:index-title") );
        if( m_pTitle )
            m_pTitle->ToXml(pStrm);
        pStrm->EndElement( A2OUSTR("text:index-title") );
    }

    XFContentContainer::ToXml(pStrm);
    pStrm->EndElement( A2OUSTR("text:index-body") );

    pStrm->EndElement( strIndexName );
}


void XFIndexTemplate::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( A2OUSTR("text:outline-level"), m_nLevel );
    pAttrList->AddAttribute( A2OUSTR("text:style-name"), m_strStyle );
    if( m_strTagName.getLength() == 0 )
        m_strTagName = A2OUSTR("text:table-of-content-entry-template");
    pStrm->StartElement( m_strTagName );

    for( size_t i=0; i<m_aEntries.size(); i++ )
    {
        pAttrList->Clear();

        switch( m_aEntries[i].first )
        {
        case enumXFIndexTemplateChapter:
            if(m_aEntries[i].second.getLength() > 0)
                pAttrList->AddAttribute( A2OUSTR("text:style-name"), m_aEntries[i].second );

            pStrm->StartElement( A2OUSTR("text:index-entry-chapter-number") );
            pStrm->EndElement( A2OUSTR("text:index-entry-chapter-number") );
            break;
        case enumXFIndexTemplateText:
            if(m_aEntries[i].second.getLength() > 0)
                pAttrList->AddAttribute( A2OUSTR("text:style-name"), m_aEntries[i].second );

            pStrm->StartElement( A2OUSTR("text:index-entry-text") );
            pStrm->EndElement( A2OUSTR("text:index-entry-text") );
            break;
        case enumXFIndexTemplateTab:
            if( m_eTabType != enumXFTabRight )
                pAttrList->AddAttribute( A2OUSTR("style:position"), DoubleToOUString(m_fTabLength) + A2OUSTR("cm") );

            if(m_aEntries[i].second.getLength() > 0)
                pAttrList->AddAttribute( A2OUSTR("text:style-name"), m_aEntries[i].second );

            switch(m_eTabType)
            {
            case enumXFTabLeft:
                pAttrList->AddAttribute( A2OUSTR("style:type"), A2OUSTR("left") );
                break;
            case enumXFTabCenter:
                pAttrList->AddAttribute( A2OUSTR("style:type"), A2OUSTR("center") );
                break;
            case enumXFTabRight:
                pAttrList->AddAttribute( A2OUSTR("style:type"), A2OUSTR("right") );
                break;
            case enumXFTabChar:
                pAttrList->AddAttribute( A2OUSTR("style:type"), A2OUSTR("char") );
                break;
            default:
                break;
            }
            //delimiter:
            if( m_eTabType == enumXFTabChar )
                pAttrList->AddAttribute( A2OUSTR("style:char"), m_strTabDelimiter );
            //leader char:
            if( m_strTabLeader.getLength() )
                pAttrList->AddAttribute( A2OUSTR("style:leader-char"), m_strTabLeader );

            pStrm->StartElement( A2OUSTR("text:index-entry-tab-stop") );
            pStrm->EndElement( A2OUSTR("text:index-entry-tab-stop") );
            break;
        case enumXFIndexTemplatePage:
            if(m_aEntries[i].second.getLength() > 0)
                pAttrList->AddAttribute( A2OUSTR("text:style-name"), m_aEntries[i].second );

            pStrm->StartElement( A2OUSTR("text:index-entry-page-number") );
            pStrm->EndElement( A2OUSTR("text:index-entry-page-number") );
            break;
        case enumXFIndexTemplateLinkStart:
            pStrm->StartElement( A2OUSTR("text:index-entry-link-start") );
            pStrm->EndElement( A2OUSTR("text:index-entry-link-start") );
            break;
        case enumXFIndexTemplateLinkEnd:
            pStrm->StartElement( A2OUSTR("text:index-entry-link-end") );
            pStrm->EndElement( A2OUSTR("text:index-entry-link-end") );
            break;
        case enumXFIndexTemplateSpan:
            pAttrList->Clear();
            pAttrList->AddAttribute(A2OUSTR("text:style-name"), GetStyleName());
            pStrm->StartElement( A2OUSTR("text:index-entry-span") );
            pStrm->Characters(m_aTextEntries[i]);
            pStrm->EndElement( A2OUSTR("text:index-entry-span") );
            break;
        case enumXFIndexTemplateBibliography:
            pStrm->StartElement( A2OUSTR("text:index-entry-bibliography") );
            pStrm->EndElement( A2OUSTR("text:index-entry-bibliography") );
            break;
        default:
            break;
        }
    }

    pStrm->EndElement( m_strTagName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
