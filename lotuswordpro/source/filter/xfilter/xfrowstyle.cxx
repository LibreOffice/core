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
 * Table row style, ie. table row height.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-01-28 create and implements.
 ************************************************************************/
#include	"xfrowstyle.hxx"
#include	"xfbgimage.hxx"

XFRowStyle::XFRowStyle():m_fHeight(0)
{
    m_fHeight = 0;
    m_fMinHeight = 0;
    m_pBGImage = NULL;
}


void	XFRowStyle::SetbackImage(XFBGImage *pImage)
{
    if( m_pBGImage )
        delete m_pBGImage;
    m_pBGImage = pImage;
}

enumXFStyle	XFRowStyle::GetStyleFamily()
{
    return enumXFStyleTableRow;
}

void	XFRowStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( A2OUSTR("style:name"), GetStyleName() );
    if( GetParentStyleName().getLength() > 0 )
        pAttrList->AddAttribute(A2OUSTR("style:parent-style-name"),GetParentStyleName());

    pAttrList->AddAttribute( A2OUSTR("style:family"), A2OUSTR("table-row") );
    pStrm->StartElement( A2OUSTR("style:style") );

    pAttrList->Clear();
    if( m_fMinHeight )
        pAttrList->AddAttribute( A2OUSTR("style:min-row-height"), DoubleToOUString(m_fMinHeight) + A2OUSTR("cm") );
    else if( m_fHeight )
        pAttrList->AddAttribute( A2OUSTR("style:row-height"), DoubleToOUString(m_fHeight) + A2OUSTR("cm") );

    if( m_aBackColor.IsValid() && !m_pBGImage )
        pAttrList->AddAttribute( A2OUSTR("fo:background-color"), m_aBackColor.ToString() );
    pStrm->StartElement( A2OUSTR("style:properties") );
    if( m_pBGImage )
        m_pBGImage->ToXml(pStrm);
    pStrm->EndElement( A2OUSTR("style:properties") );

    pStrm->EndElement( A2OUSTR("style:style") );
}

XFRowStyle& XFRowStyle::operator=(XFRowStyle& other)
{
    m_fHeight = other.m_fHeight;
    m_fMinHeight = other.m_fMinHeight;
    m_aBackColor = other.m_aBackColor;

    if( m_pBGImage )
        delete m_pBGImage;

    if( other.m_pBGImage )
        m_pBGImage = new XFBGImage(*other.m_pBGImage);
    else
        m_pBGImage = NULL;
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
