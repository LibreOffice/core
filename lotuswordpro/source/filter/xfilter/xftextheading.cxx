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
 * Heading text for OOo.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-01-07 create this file.
 * 2005-01-10 it works now.
 ************************************************************************/
#include	"xftextheading.hxx"
#include	"xftextcontent.hxx"

XFTextHeading::XFTextHeading()
{
    m_pContent = NULL;
    m_nLevel = 0;
}

XFTextHeading::~XFTextHeading()
{
    if( m_pContent )
        delete m_pContent;
}

enumXFContent XFTextHeading::GetContentType()
{
    return enumXFContentHeading;
}

void XFTextHeading::SetLevel(sal_Int16 level)
{
    m_nLevel = level;
}

void XFTextHeading::SetText(rtl::OUString text)
{
    m_pContent = new XFTextContent(text);
}

void XFTextHeading::ToXml(IXFStream *pStrm)
{
    rtl::OUString style = GetStyleName();
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    assert(m_nLevel>0);
    pAttrList->Clear();
    if( style.getLength() )
        pAttrList->AddAttribute( A2OUSTR("text:style-name"), style );
    pAttrList->AddAttribute( A2OUSTR("text:level"), Int16ToOUString(m_nLevel) );

    pStrm->StartElement( A2OUSTR("text:h") );
    m_pContent->ToXml(pStrm);
    pStrm->EndElement( A2OUSTR("text:h") );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
