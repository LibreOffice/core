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
 * Section object.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-12-28 create this file.
 ************************************************************************/
#include	"xfsection.hxx"

XFSection::XFSection()
{
    m_bProtected = sal_False;
    m_bHiden = sal_False;
    m_strSectionName = XFGlobal::GenSectionName();
}

XFSection::~XFSection()
{
}

void	XFSection::SetSectionName(rtl::OUString name)
{
    m_strSectionName = name;
}

void XFSection::SetSourceLink( rtl::OUString link)
{
    m_strSourceLink = link;
}

void	XFSection::SetProtected(sal_Bool bProtected)
{
    m_bProtected = bProtected;
}

void	XFSection::SetHiden(sal_Bool hiden)
{
    m_bHiden = hiden;
}

void	XFSection::ToXml(IXFStream *pStrm)
{
    assert(m_strSectionName.getLength() > 0 );

    IXFAttrList		*pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    rtl::OUString style = GetStyleName();
    if( style.getLength() )
        pAttrList->AddAttribute( A2OUSTR("text:style-name"), style);
    //section name
    if( m_strSectionName.getLength() )
        pAttrList->AddAttribute( A2OUSTR("text:name"), m_strSectionName);
    if( m_bProtected )
        pAttrList->AddAttribute( A2OUSTR("text:protected"), A2OUSTR("true") );
    if( m_bHiden )
        pAttrList->AddAttribute( A2OUSTR("text:display"), A2OUSTR("none") );

    pStrm->StartElement( A2OUSTR("text:section") );
    if( m_strSourceLink.getLength() )
    {
        pAttrList->Clear();
        pAttrList->AddAttribute( A2OUSTR("xlink:href"), m_strSourceLink);
        pAttrList->AddAttribute( A2OUSTR("text:filter-name"), A2OUSTR("wordpro"));
        pStrm->StartElement( A2OUSTR("text:section-source") );
        pStrm->EndElement( A2OUSTR("text:section-source") );
    }

    XFContentContainer::ToXml(pStrm);

    pStrm->EndElement( A2OUSTR("text:section") );

}
/*
void XFSection::ToXmlHeader(IXFStream *pStrm)
{
    assert(m_strSectionName.getLength() > 0 );

    IXFAttrList		*pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    rtl::OUString style = GetStyleName();
    if( style.getLength() )
        pAttrList->AddAttribute( A2OUSTR("text:style-name"), style);
    //section name
    if( m_strSectionName.getLength() )
        pAttrList->AddAttribute( A2OUSTR("text:name"), m_strSectionName);
    if( m_bProtected )
        pAttrList->AddAttribute( A2OUSTR("text:protected"), A2OUSTR("true") );
    if( m_bHiden )
        pAttrList->AddAttribute( A2OUSTR("text:display"), A2OUSTR("none") );

    pStrm->StartElement( A2OUSTR("text:section") );
}

void XFSection::ToXmlTail(IXFStream *pStrm)
{
    pStrm->EndElement( A2OUSTR("text:section") );
}
*/
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
