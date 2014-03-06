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
 * Implemention file for Meta data,pls reference to the header file.
 ************************************************************************/
#include "xfofficemeta.hxx"
#include "ixfstream.hxx"
#include "ixfattrlist.hxx"

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
    pStream->StartElement( "office:meta" );

    //generator:
    pStream->StartElement( "meta:generator" );
    pStream->Characters( m_strGenerator );
    pStream->EndElement( "meta:generator" );

    //title
    pStream->StartElement( "dc:title" );
    pStream->Characters( m_strTitle );
    pStream->EndElement( "dc:title" );

    //keywords
    pStream->StartElement( "meta:keywords" );
    pStream->StartElement( "meta:keyword" );
    pStream->Characters( m_strKeywords );
    pStream->EndElement( "meta:keyword" );
    pStream->EndElement( "meta:keywords" );

    //creation time
    pStream->StartElement( "meta:creation-date" );
    pStream->Characters( m_strCrtime);
    pStream->EndElement( "meta:creation-date" );

    //last revision time
    pStream->StartElement( "dc:date" );
    pStream->Characters( m_strLstime);
    pStream->EndElement( "dc:date" );
    //total edit time
    pStream->StartElement( "meta:editing-duration" );
    pStream->Characters( m_strEdtime);
    pStream->EndElement( "meta:editing-duration" );

    //description
    pStream->StartElement( "dc:description" );
    pStream->Characters( m_strDsr );
    pStream->EndElement( "dc:description" );
    //creator
    pStream->StartElement( "meta:initial-creator" );
    pStream->Characters( m_strCreator );
    pStream->EndElement( "meta:initial-creator" );

    pStream->EndElement( "office:meta" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
