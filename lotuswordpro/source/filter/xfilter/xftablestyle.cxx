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
 * Table style.
 ************************************************************************/
#include "xftablestyle.hxx"
#include "xfbgimage.hxx"

XFTableStyle::XFTableStyle()
{
    m_fWidth = 2;
    m_eAlign = enumXFAlignStart;
    m_pBGImage = nullptr;
}

XFTableStyle::~XFTableStyle()
{
    delete m_pBGImage;
}

void    XFTableStyle::SetBackImage(XFBGImage *pImage)
{
    delete m_pBGImage;

    m_pBGImage = pImage;
}

enumXFStyle XFTableStyle::GetStyleFamily()
{
    return enumXFStyleTable;
}

void    XFTableStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( "style:name", GetStyleName() );
    if( !GetParentStyleName().isEmpty() )
        pAttrList->AddAttribute("style:parent-style-name",GetParentStyleName());
    pAttrList->AddAttribute( "style:family", "table" );
    pStrm->StartElement( "style:style" );

    pAttrList->Clear();
    pAttrList->AddAttribute( "style:width", OUString::number(m_fWidth) + "cm" );
    //text align:
    if( m_eAlign == enumXFAlignStart )
    {
        pAttrList->AddAttribute("table:align","left");
    }
    else if( m_eAlign == enumXFAlignCenter )
    {
        pAttrList->AddAttribute("table:align","center");
    }
    else if( m_eAlign == enumXFAlignEnd )
    {
        pAttrList->AddAttribute("table:align","right");
    }
    else if( m_eAlign == enumXFALignMargins )
    {
        pAttrList->AddAttribute("table:align","margins");
    }
    //background color:
    if( m_aBackColor.IsValid() && !m_pBGImage )
        pAttrList->AddAttribute("fo:background-color", m_aBackColor.ToString() );
    //shadow:
    m_aShadow.ToXml(pStrm);
    //margin:
    m_aMargins.ToXml(pStrm);
    //breaks:
    m_aBreaks.ToXml(pStrm);

    pStrm->StartElement( "style:properties" );
    if( m_pBGImage )
        m_pBGImage->ToXml(pStrm);

    pStrm->EndElement( "style:properties" );

    pStrm->EndElement( "style:style" );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
