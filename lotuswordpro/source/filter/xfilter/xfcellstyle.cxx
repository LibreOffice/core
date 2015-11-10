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
 * Table cell style. Number format, string value, and so on...
 ************************************************************************/
#include "xfcellstyle.hxx"
#include "xfborders.hxx"
#include "xffont.hxx"
#include "xfbgimage.hxx"

XFCellStyle::XFCellStyle()
{
    m_eHoriAlign = enumXFAlignNone;
    m_eVertAlign = enumXFAlignNone;
    m_fTextIndent = 0;
    m_pBorders = nullptr;
    m_pBackImage = nullptr;
    m_bWrapText = false;
}

XFCellStyle::~XFCellStyle()
{
    delete m_pBorders;
}

void XFCellStyle::SetPadding(double left, double right,double top, double bottom)
{
    if( left != -1 )
        m_aPadding.SetLeft(left);
    if( right != -1 )
        m_aPadding.SetRight(right);
    if( top != -1 )
        m_aPadding.SetTop(top);
    if( bottom != -1 )
        m_aPadding.SetBottom( bottom );
}

void    XFCellStyle::SetBackColor(XFColor& color)
{
    m_aBackColor = color;
}

void    XFCellStyle::SetBackImage(XFBGImage *pImage)
{
    delete m_pBackImage;
    m_pBackImage = pImage;
}

void    XFCellStyle::SetBorders(XFBorders *pBorders)
{
    delete m_pBorders;
    m_pBorders = pBorders;
}

enumXFStyle XFCellStyle::GetStyleFamily()
{
    return enumXFStyleTableCell;
}

/**
 *Affirm whether two XFCellStyle objects are equal.
 */
bool    XFCellStyle::Equal(IXFStyle *pStyle)
{
    if( this == pStyle )
        return true;
    if( !pStyle || pStyle->GetStyleFamily() != enumXFStyleTableCell )
        return false;

    XFCellStyle *pOther = dynamic_cast<XFCellStyle*>(pStyle);
    if( !pOther )
        return false;

    if( m_strDataStyle != pOther->m_strDataStyle )
        return false;

    if( m_strParentStyleName != pOther->m_strParentStyleName )
        return false;
    if( m_fTextIndent != pOther->m_fTextIndent )
        return false;

    //align:
    if( m_eHoriAlign != pOther->m_eHoriAlign )
        return false;
    if( m_eVertAlign != pOther->m_eVertAlign )
        return false;

    if( m_aBackColor != pOther->m_aBackColor )
        return false;
    //shadow:
    if( m_aShadow != pOther->m_aShadow )
        return false;
    //margin:
    if( m_aMargin != pOther->m_aMargin )
        return false;
    //padding:
    if( m_aPadding != pOther->m_aPadding )
        return false;

    //wrap:
    if( m_bWrapText != pOther->m_bWrapText )
        return false;

    //font:
    if( m_pFont.is() )
    {
        if( !pOther->m_pFont.is() )
            return false;
        if(*m_pFont != *pOther->m_pFont )
            return false;
    }
    else if( pOther->m_pFont.is() )
        return false;

    //border:
    if( m_pBorders )
    {
        if( !pOther->m_pBorders )
            return false;
        if( *m_pBorders != *pOther->m_pBorders )
            return false;
    }
    else if( pOther->m_pBorders )
        return false;

    //if there is backimage
    if( m_pBackImage )
    {
        if( !pOther->m_pBackImage )
            return false;
        if( !m_pBackImage->Equal(pOther) )
            return false;
    }
    else
    {
        if( pOther->m_pBackImage )
            return false;
    }

    return true;
}

void XFCellStyle::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    OUString style = GetStyleName();

    pAttrList->Clear();
    if( !style.isEmpty() )
        pAttrList->AddAttribute("style:name",GetStyleName());
    if( !GetParentStyleName().isEmpty() )
        pAttrList->AddAttribute("style:parent-style-name",GetParentStyleName());

    pAttrList->AddAttribute("style:family", "table-cell");
    if( !m_strParentStyleName.isEmpty() )
        pAttrList->AddAttribute("style:parent-style-name",m_strParentStyleName);
    if( !m_strDataStyle.isEmpty() )
        pAttrList->AddAttribute( "style:data-style-name", m_strDataStyle );

    pStrm->StartElement("style:style");

    //Paragraph properties:
    pAttrList->Clear();

    //text indent:
    if( m_fTextIndent>FLOAT_MIN )
    {
        pAttrList->AddAttribute("fo:text-indent", OUString::number(m_fTextIndent) + "cm" );
    }
    //padding:
    m_aPadding.ToXml(pStrm);
    //margin:
    m_aMargin.ToXml(pStrm);

    //text horizontal align:
    if( m_eHoriAlign != enumXFAlignNone )
    {
        pAttrList->AddAttribute("fo:text-align", GetAlignName(m_eHoriAlign) );
    }
    //text vertical align
    if( m_eVertAlign != enumXFAlignNone )
        pAttrList->AddAttribute( "fo:vertical-align", GetAlignName(m_eVertAlign) );

    //wrap text:
    if( m_bWrapText )
        pAttrList->AddAttribute( "fo:wrap-option", "wrap" );

    //shadow:
    m_aShadow.ToXml(pStrm);
    //borders:
    if( m_pBorders )
        m_pBorders->ToXml(pStrm);

    //background color:
    if( m_aBackColor.IsValid() && !m_pBackImage )
    {
        pAttrList->AddAttribute("fo:background-color", m_aBackColor.ToString() );
    }
    //Font properties:
    if( m_pFont.is() )
        m_pFont->ToXml(pStrm);

    pStrm->StartElement("style:properties");

    if( m_pBackImage )
        m_pBackImage->ToXml(pStrm);

    pStrm->EndElement("style:properties");

    pStrm->EndElement("style:style");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
