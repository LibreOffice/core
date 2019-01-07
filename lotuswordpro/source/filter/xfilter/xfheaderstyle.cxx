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
 * Header style,exist in page-master object.
 ************************************************************************/
#include <xfilter/xfheaderstyle.hxx>
#include <xfilter/xfbgimage.hxx>

XFHeaderStyle::XFHeaderStyle(bool isFooter)
{
    m_bIsFooter = isFooter;
    m_bDynamicSpace = true;
    m_fHeight = -1;
    m_fMinHeight = -1;
}

XFHeaderStyle::~XFHeaderStyle()
{
}

void    XFHeaderStyle::SetMargins(double left, double right, double bottom)
{
    if( left != -1 )
        m_aMargin.SetLeft(left);
    if( right != -1 )
        m_aMargin.SetRight(right);
    if( bottom != -1 )
        m_aMargin.SetBottom(bottom);
}
void    XFHeaderStyle::SetDynamicSpace(bool dynamic)
{
    m_bDynamicSpace = dynamic;
}

void    XFHeaderStyle::SetHeight(double height)
{
    m_fHeight = height;
}

void    XFHeaderStyle::SetMinHeight(double minHeight)
{
    m_fMinHeight = minHeight;
}

void    XFHeaderStyle::SetShadow(XFShadow *pShadow)
{
    if( pShadow == m_pShadow.get() )
        return;
    m_pShadow.reset( pShadow );
}

void    XFHeaderStyle::SetBorders(std::unique_ptr<XFBorders> pBorders)
{
    m_pBorders = std::move(pBorders);
}

void    XFHeaderStyle::SetBackImage(std::unique_ptr<XFBGImage>& rImage)
{
    m_pBGImage = std::move(rImage);
}

void    XFHeaderStyle::SetBackColor(XFColor color)
{
    m_aBackColor = color;
}

void    XFHeaderStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    if( m_bIsFooter )
        pStrm->StartElement( "style:footer-style" );
    else
        pStrm->StartElement( "style:header-style" );

    //height
    if( m_fMinHeight>0 )
        pAttrList->AddAttribute( "fo:min-height", OUString::number(m_fMinHeight) + "cm" );
    else if( m_fHeight>0 )
        pAttrList->AddAttribute( "svg:height", OUString::number(m_fHeight) + "cm" );

    m_aMargin.ToXml(pStrm);
    if(m_pShadow)
        m_pShadow->ToXml(pStrm);
    m_aPadding.ToXml(pStrm);
    if( m_pBorders )
        m_pBorders->ToXml(pStrm);

    if( m_aBackColor.IsValid() )
        pAttrList->AddAttribute( "fo:background-color", m_aBackColor.ToString() );

    if( m_bDynamicSpace )
        pAttrList->AddAttribute( "style:dynamic-spacing", "true" );
    else
        pAttrList->AddAttribute( "style:dynamic-spacing", "false" );

    pStrm->StartElement( "style:properties" );

    // background image
    if( m_pBGImage )
        m_pBGImage->ToXml(pStrm);

    pStrm->EndElement( "style:properties" );

    if( m_bIsFooter )
        pStrm->EndElement( "style:footer-style" );
    else
        pStrm->EndElement( "style:header-style" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
