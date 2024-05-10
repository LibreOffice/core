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
 * doc field.
 ************************************************************************/
#include <xfilter/ixfattrlist.hxx>
#include <xfilter/ixfstream.hxx>
#include <xfilter/xfdocfield.hxx>

void XFFileName::SetType(const OUString& sType)
{
    m_strType = sType;
}

void XFFileName::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    if (m_strType == "FileName")
        pAttrList->AddAttribute( u"text:display"_ustr, u"name-and-extension"_ustr );
    else if (m_strType == "Path")
        pAttrList->AddAttribute( u"text:display"_ustr, u"path"_ustr );
    pStrm->StartElement( u"text:file-name"_ustr );
    pStrm->EndElement( u"text:file-name"_ustr );
}

void XFWordCount::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( u"text:word-count"_ustr );
    pStrm->EndElement( u"text:word-count"_ustr );
}

void XFCharCount::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( u"text:character-count"_ustr );
    pStrm->EndElement( u"text:character-count"_ustr );
}

void XFDescription::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( u"text:description"_ustr );
    pStrm->EndElement( u"text:description"_ustr );
}

void XFPageCountStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( u"text:page-count"_ustr );
}
void XFPageCountEnd::ToXml(IXFStream *pStrm)
{
    pStrm->EndElement( u"text:page-count"_ustr );
}

void XFWordCountStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( u"text:word-count"_ustr );
}

void XFCharCountStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( u"text:character-count"_ustr );
}

void XFDescriptionStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( u"text:description"_ustr );
}

void XFWordCountEnd::ToXml(IXFStream *pStrm)
{
    pStrm->EndElement( u"text:word-count"_ustr );
}

void XFCharCountEnd::ToXml(IXFStream *pStrm)
{
    pStrm->EndElement( u"text:character-count"_ustr );
}

void XFDescriptionEnd::ToXml(IXFStream *pStrm)
{
    pStrm->EndElement( u"text:description"_ustr );
}

void XFKeywords::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( u"text:keywords"_ustr );
    pStrm->EndElement( u"text:keywords"_ustr );
}

void XFTotalEditTime::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    if (!m_strStyleName.isEmpty())
        pAttrList->AddAttribute( u"style:data-style-name"_ustr, m_strStyleName );
    pStrm->StartElement( u"text:editing-duration"_ustr );
    pStrm->EndElement( u"text:editing-duration"_ustr );
}
void XFTotalEditTimeStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    if (!m_strStyleName.isEmpty())
        pAttrList->AddAttribute( u"style:data-style-name"_ustr, m_strStyleName );
    pStrm->StartElement( u"text:editing-duration"_ustr );
}
void XFTotalEditTimeEnd::ToXml(IXFStream *pStrm)
{
    pStrm->EndElement( u"text:editing-duration"_ustr );
}
void XFCreateTime::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    if (!m_strStyleName.isEmpty())
        pAttrList->AddAttribute( u"style:data-style-name"_ustr, m_strStyleName );
    pStrm->StartElement( u"text:creation-time"_ustr );
    pStrm->EndElement( u"text:creation-time"_ustr );
}
void XFLastEditTime::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    if (!m_strStyleName.isEmpty())
        pAttrList->AddAttribute( u"style:data-style-name"_ustr, m_strStyleName );
    pStrm->StartElement( u"text:modification-time"_ustr );
    pStrm->EndElement( u"text:modification-time"_ustr );
}

void XFCreateTimeStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    if (!m_strStyleName.isEmpty())
        pAttrList->AddAttribute( u"style:data-style-name"_ustr, m_strStyleName );
    pStrm->StartElement( u"text:creation-time"_ustr );
}

void XFLastEditTimeStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    if (!m_strStyleName.isEmpty())
        pAttrList->AddAttribute( u"style:data-style-name"_ustr, m_strStyleName );
    pStrm->StartElement( u"text:modification-time"_ustr );
}
void XFCreateTimeEnd::ToXml(IXFStream *pStrm)
{
    pStrm->EndElement( u"text:creation-time"_ustr );
}

void XFLastEditTimeEnd::ToXml(IXFStream *pStrm)
{
    pStrm->EndElement( u"text:modification-time"_ustr );
}

void XFInitialCreator::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( u"text:initial-creator"_ustr );
    pStrm->EndElement( u"text:initial-creator"_ustr );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
