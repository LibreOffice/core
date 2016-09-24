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
 * Columns styles for section, or paragraph.
 ************************************************************************/
#include "xfcolumns.hxx"

void    XFColumn::SetRelWidth(sal_Int32 width)
{
    m_nRelWidth = width;
}

void    XFColumn::SetMargins(double left, double right)
{
    m_fMarginLeft = left;
    m_fMarginRight = right;
}

void    XFColumn::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( "style:rel-width", OUString::number(m_nRelWidth) + "*" );
    pAttrList->AddAttribute( "fo:margin-left", OUString::number(m_fMarginLeft) + "cm" );
    pAttrList->AddAttribute( "fo:margin-right", OUString::number(m_fMarginRight) + "cm" );

    pStrm->StartElement( "style:column" );
    pStrm->EndElement( "style:column" );
}

void    XFColumnSep::SetRelHeight(sal_Int32 height)
{
    assert(height>=0&&height<=100);
    m_nRelHeight = height;
}
void    XFColumnSep::SetWidth(double width)
{
    m_fWidth = width;
}
void    XFColumnSep::SetColor(XFColor& color)
{
    m_aColor = color;
}
void    XFColumnSep::SetVerticalAlign(enumXFAlignType align)
{
    m_eVertAlign = align;
}

void    XFColumnSep::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pAttrList->AddAttribute( "style:width", OUString::number(m_fWidth) + "cm" );
    if( m_aColor.IsValid() )
        pAttrList->AddAttribute( "style:color", m_aColor.ToString() );
    pAttrList->AddAttribute( "style:height", OUString::number(m_nRelHeight) + "%" );
    //text align:
    if( m_eVertAlign == enumXFAlignTop )
    {
        pAttrList->AddAttribute("style:vertical-align","top");
    }
    else if( m_eVertAlign == enumXFAlignMiddle )
    {
        pAttrList->AddAttribute("style:vertical-align","middle");
    }
    else if( m_eVertAlign == enumXFAlignBottom )
    {
        pAttrList->AddAttribute("style:vertical-align","bottom");
    }

    pStrm->StartElement( "style:column-sep" );
    pStrm->EndElement( "style:column-sep" );
}

void    XFColumns::SetSeparator(XFColumnSep& aSeparator)
{
    m_aSeparator = aSeparator;
    m_nFlag |= XFCOLUMNS_FLAG_SEPARATOR;
}

void    XFColumns::AddColumn(XFColumn& column)
{
    m_aColumns.push_back(column);
}

void    XFColumns::SetGap(double fGap)
{
    m_fGap = fGap;
    m_nFlag |= XFCOLUMNS_FLAG_GAP;
}

void    XFColumns::SetCount(sal_uInt16 nCount)
{
    m_nCount = nCount;
}

void    XFColumns::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( "fo:column-count", OUString::number(m_nCount));
    if( m_nFlag&XFCOLUMNS_FLAG_GAP )
    {
        pAttrList->AddAttribute( "fo:column-gap", OUString::number(m_fGap)+"cm" );
    }

    pStrm->StartElement( "style:columns" );

    if( m_nFlag&XFCOLUMNS_FLAG_SEPARATOR )  //column-sep:
    {
        m_aSeparator.ToXml(pStrm);
    }

    if(!(m_nFlag&XFCOLUMNS_FLAG_GAP) )
    {
        std::vector<XFColumn>::iterator it;
        for( it = m_aColumns.begin(); it != m_aColumns.end(); ++it )
        {
            (*it).ToXml(pStrm);
        }
    }
    pStrm->EndElement( "style:columns" );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
