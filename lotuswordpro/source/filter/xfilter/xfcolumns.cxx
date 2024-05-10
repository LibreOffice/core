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
#include <xfilter/xfcolumns.hxx>
#include <xfilter/ixfattrlist.hxx>

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

    pAttrList->AddAttribute( u"style:rel-width"_ustr, OUString::number(m_nRelWidth) + "*" );
    pAttrList->AddAttribute( u"fo:margin-left"_ustr, OUString::number(m_fMarginLeft) + "cm" );
    pAttrList->AddAttribute( u"fo:margin-right"_ustr, OUString::number(m_fMarginRight) + "cm" );

    pStrm->StartElement( u"style:column"_ustr );
    pStrm->EndElement( u"style:column"_ustr );
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
void    XFColumnSep::SetColor(XFColor const & color)
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
    pAttrList->AddAttribute( u"style:width"_ustr, OUString::number(m_fWidth) + "cm" );
    if( m_aColor.IsValid() )
        pAttrList->AddAttribute( u"style:color"_ustr, m_aColor.ToString() );
    pAttrList->AddAttribute( u"style:height"_ustr, OUString::number(m_nRelHeight) + "%" );
    //text align:
    if( m_eVertAlign == enumXFAlignTop )
    {
        pAttrList->AddAttribute(u"style:vertical-align"_ustr,u"top"_ustr);
    }
    else if( m_eVertAlign == enumXFAlignMiddle )
    {
        pAttrList->AddAttribute(u"style:vertical-align"_ustr,u"middle"_ustr);
    }
    else if( m_eVertAlign == enumXFAlignBottom )
    {
        pAttrList->AddAttribute(u"style:vertical-align"_ustr,u"bottom"_ustr);
    }

    pStrm->StartElement( u"style:column-sep"_ustr );
    pStrm->EndElement( u"style:column-sep"_ustr );
}

void    XFColumns::SetSeparator(XFColumnSep const & aSeparator)
{
    m_aSeparator = aSeparator;
    m_nFlag |= XFCOLUMNS_FLAG_SEPARATOR;
}

void    XFColumns::AddColumn(XFColumn const & column)
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

    pAttrList->AddAttribute( u"fo:column-count"_ustr, OUString::number(m_nCount));
    if( m_nFlag&XFCOLUMNS_FLAG_GAP )
    {
        pAttrList->AddAttribute( u"fo:column-gap"_ustr, OUString::number(m_fGap)+"cm" );
    }

    pStrm->StartElement( u"style:columns"_ustr );

    if( m_nFlag&XFCOLUMNS_FLAG_SEPARATOR )  //column-sep:
    {
        m_aSeparator.ToXml(pStrm);
    }

    if(!(m_nFlag&XFCOLUMNS_FLAG_GAP) )
    {
        for (auto & column : m_aColumns)
        {
            column.ToXml(pStrm);
        }
    }
    pStrm->EndElement( u"style:columns"_ustr );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
