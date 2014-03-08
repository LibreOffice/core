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
#include "xfdocfield.hxx"

void XFFileName::SetType(OUString sType)
{
    m_strType = sType;
}

void XFFileName::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    if (m_strType.equals("FileName"))
        pAttrList->AddAttribute( "text:display", "name-and-extension" );
    else if (m_strType.equals("Path"))
        pAttrList->AddAttribute( "text:display", "path" );
    pStrm->StartElement( "text:file-name" );
    pStrm->EndElement( "text:file-name" );
}

void XFWordCount::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( "text:word-count" );
    pStrm->EndElement( "text:word-count" );
}

void XFCharCount::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( "text:character-count" );
    pStrm->EndElement( "text:character-count" );
}

void XFDescription::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( "text:description" );
    pStrm->EndElement( "text:description" );
}

void XFPageCountStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( "text:page-count" );
}
void XFPageCountEnd::ToXml(IXFStream *pStrm)
{
    pStrm->EndElement( "text:page-count" );
}

void XFWordCountStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( "text:word-count" );
}

void XFCharCountStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( "text:character-count" );
}

void XFDescriptionStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( "text:description" );
}

void XFWordCountEnd::ToXml(IXFStream *pStrm)
{
    pStrm->EndElement( "text:word-count" );
}

void XFCharCountEnd::ToXml(IXFStream *pStrm)
{
    pStrm->EndElement( "text:character-count" );
}

void XFDescriptionEnd::ToXml(IXFStream *pStrm)
{
    pStrm->EndElement( "text:description" );
}

void XFKeywords::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( "text:keywords" );
    pStrm->EndElement( "text:keywords" );
}

void XFTotalEditTime::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    if (!m_strStyleName.isEmpty())
        pAttrList->AddAttribute( "style:data-style-name", m_strStyleName );
    pStrm->StartElement( "text:editing-duration" );
    pStrm->EndElement( "text:editing-duration" );
}
void XFTotalEditTimeStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    if (!m_strStyleName.isEmpty())
        pAttrList->AddAttribute( "style:data-style-name", m_strStyleName );
    pStrm->StartElement( "text:editing-duration" );
}
void XFTotalEditTimeEnd::ToXml(IXFStream *pStrm)
{
    pStrm->EndElement( "text:editing-duration" );
}
void XFCreateTime::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    if (!m_strStyleName.isEmpty())
        pAttrList->AddAttribute( "style:data-style-name", m_strStyleName );
    pStrm->StartElement( "text:creation-time" );
    pStrm->EndElement( "text:creation-time" );
}
void XFLastEditTime::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    if (!m_strStyleName.isEmpty())
        pAttrList->AddAttribute( "style:data-style-name", m_strStyleName );
    pStrm->StartElement( "text:modification-time" );
    pStrm->EndElement( "text:modification-time" );
}

void XFCreateTimeStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    if (!m_strStyleName.isEmpty())
        pAttrList->AddAttribute( "style:data-style-name", m_strStyleName );
    pStrm->StartElement( "text:creation-time" );
}

void XFLastEditTimeStart::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    if (!m_strStyleName.isEmpty())
        pAttrList->AddAttribute( "style:data-style-name", m_strStyleName );
    pStrm->StartElement( "text:modification-time" );
}
void XFCreateTimeEnd::ToXml(IXFStream *pStrm)
{
    pStrm->EndElement( "text:creation-time" );
}

void XFLastEditTimeEnd::ToXml(IXFStream *pStrm)
{
    pStrm->EndElement( "text:modification-time" );
}

void XFInitialCreator::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();
    pStrm->StartElement( "text:initial-creator" );
    pStrm->EndElement( "text:initial-creator" );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
