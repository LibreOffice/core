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
 * Implementation file for Meta data,pls reference to the header file.
 ************************************************************************/
#include <xfilter/xfofficemeta.hxx>
#include <xfilter/ixfstream.hxx>
#include <xfilter/ixfattrlist.hxx>

void    XFOfficeMeta::SetCreator(const OUString& creator)
{
    m_strCreator = creator;
}
void    XFOfficeMeta::SetDescription(const OUString& dsr)
{
    m_strDsr = dsr;
}
void XFOfficeMeta::SetKeywords(const OUString& keywords)
{
    m_strKeywords = keywords;
}
void XFOfficeMeta::SetCreationTime(const OUString& crtime)
{
    m_strCrtime = crtime;
}
void XFOfficeMeta::SetLastTime(const OUString& lstime)
{
    m_strLstime = lstime;
}
void XFOfficeMeta::SetEditTime(const OUString& edtime)
{
    m_strEdtime = edtime;
}
void    XFOfficeMeta::ToXml(IXFStream *pStream)
{
    IXFAttrList *pAttrList = pStream->GetAttrList();

    pAttrList->Clear();
    pStream->StartElement( u"office:meta"_ustr );

    //generator:
    pStream->StartElement( u"meta:generator"_ustr );
    pStream->Characters( m_strGenerator );
    pStream->EndElement( u"meta:generator"_ustr );

    //title
    pStream->StartElement( u"dc:title"_ustr );
    pStream->Characters( m_strTitle );
    pStream->EndElement( u"dc:title"_ustr );

    //keywords
    pStream->StartElement( u"meta:keywords"_ustr );
    pStream->StartElement( u"meta:keyword"_ustr );
    pStream->Characters( m_strKeywords );
    pStream->EndElement( u"meta:keyword"_ustr );
    pStream->EndElement( u"meta:keywords"_ustr );

    //creation time
    pStream->StartElement( u"meta:creation-date"_ustr );
    pStream->Characters( m_strCrtime);
    pStream->EndElement( u"meta:creation-date"_ustr );

    //last revision time
    pStream->StartElement( u"dc:date"_ustr );
    pStream->Characters( m_strLstime);
    pStream->EndElement( u"dc:date"_ustr );
    //total edit time
    pStream->StartElement( u"meta:editing-duration"_ustr );
    pStream->Characters( m_strEdtime);
    pStream->EndElement( u"meta:editing-duration"_ustr );

    //description
    pStream->StartElement( u"dc:description"_ustr );
    pStream->Characters( m_strDsr );
    pStream->EndElement( u"dc:description"_ustr );
    //creator
    pStream->StartElement( u"meta:initial-creator"_ustr );
    pStream->Characters( m_strCreator );
    pStream->EndElement( u"meta:initial-creator"_ustr );

    pStream->EndElement( u"office:meta"_ustr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
