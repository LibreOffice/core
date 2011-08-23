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
/*************************************************************************
 * Change History
 * 2005-01-17 create this file.
 ************************************************************************/

#include	"xfcolumns.hxx"

void	XFColumn::SetRelWidth(sal_Int32 width)
{
    m_nRelWidth = width;
}

sal_Int32 XFColumn::GetRelWidth()
{
    return m_nRelWidth;
}

void	XFColumn::SetMargins(double left, double right)
{
    m_fMarginLeft = left;
    m_fMarginRight = right;
}

double	XFColumn::GetMarginLeft()
{
    return m_fMarginLeft;
}
double	XFColumn::GetMarginRight()
{
    return m_fMarginRight;
}

void	XFColumn::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    pAttrList->AddAttribute( A2OUSTR("style:rel-width"), Int32ToOUString(m_nRelWidth) + A2OUSTR("*") );
    pAttrList->AddAttribute( A2OUSTR("fo:margin-left"), DoubleToOUString(m_fMarginLeft) + A2OUSTR("cm") );
    pAttrList->AddAttribute( A2OUSTR("fo:margin-right"), DoubleToOUString(m_fMarginRight) + A2OUSTR("cm") );

    pStrm->StartElement( A2OUSTR("style:column") );
    pStrm->EndElement( A2OUSTR("style:column") );
}

void	XFColumnSep::SetRelHeight(sal_Int32 height)
{
    assert(height>=0&&height<=100);
    m_nRelHeight = height;
}
void	XFColumnSep::SetWidth(double width)
{
    m_fWidth = width;
}
void	XFColumnSep::SetColor(XFColor& color)
{
    m_aColor = color;
}
void	XFColumnSep::SetVerticalAlign(enumXFAlignType align)
{
    m_eVertAlign = align;
}

void	XFColumnSep::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pAttrList->AddAttribute( A2OUSTR("style:width"), DoubleToOUString(m_fWidth) + A2OUSTR("cm") );
    if( m_aColor.IsValid() )
        pAttrList->AddAttribute( A2OUSTR("style:color"), m_aColor.ToString() );
    pAttrList->AddAttribute( A2OUSTR("style:height"), Int32ToOUString(m_nRelHeight) + A2OUSTR("%") );
    //text align:
    if( m_eVertAlign == enumXFAlignTop )
    {
        pAttrList->AddAttribute(A2OUSTR("style:vertical-align"),A2OUSTR("top"));
    }
    else if( m_eVertAlign == enumXFAlignMiddle )
    {
        pAttrList->AddAttribute(A2OUSTR("style:vertical-align"),A2OUSTR("middle"));
    }
    else if( m_eVertAlign == enumXFAlignBottom )
    {
        pAttrList->AddAttribute(A2OUSTR("style:vertical-align"),A2OUSTR("bottom"));
    }

    pStrm->StartElement( A2OUSTR("style:column-sep") );
    pStrm->EndElement( A2OUSTR("style:column-sep") );
}

void	XFColumns::SetSeperator(sal_Int32 height, double /*width*/, XFColor color,enumXFAlignType align)
{
    m_aSeperator.SetRelHeight(height);
    m_aSeperator.SetWidth(0.1);
    m_aSeperator.SetColor(color);
    m_aSeperator.SetVerticalAlign(align);
    m_nFlag |= XFCOLUMNS_FLAG_SEPERATOR;
}

void	XFColumns::SetSeperator(XFColumnSep& aSeperator)
{
    m_aSeperator = aSeperator;
    m_nFlag |= XFCOLUMNS_FLAG_SEPERATOR;
}

void	XFColumns::AddColumn(XFColumn& column)
{
    m_aColumns.push_back(column);
}

sal_Int32	XFColumns::GetCount()
{
    return m_aColumns.size();;
}

void	XFColumns::SetGap(double fGap)
{
    m_fGap = fGap;
    m_nFlag |= XFCOLUMNS_FLAG_GAP;
}

void	XFColumns::SetCount(sal_uInt16 nCount)
{
    m_nCount = nCount;
}

void	XFColumns::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();
    pAttrList->Clear();

    //pAttrList->AddAttribute( A2OUSTR("fo:column-count"), Int32ToOUString(GetCount()) );
    pAttrList->AddAttribute( A2OUSTR("fo:column-count"), Int32ToOUString(m_nCount));
    if( m_nFlag&XFCOLUMNS_FLAG_GAP )
    {
        pAttrList->AddAttribute( A2OUSTR("fo:column-gap"), DoubleToOUString(m_fGap)+A2OUSTR("cm") );
    }

    pStrm->StartElement( A2OUSTR("style:columns") );

    if( m_nFlag&XFCOLUMNS_FLAG_SEPERATOR )	//column-sep:
    {
        m_aSeperator.ToXml(pStrm);
    }

    if(!(m_nFlag&XFCOLUMNS_FLAG_GAP) )
    {
        std::vector<XFColumn>::iterator it;
        for( it = m_aColumns.begin(); it != m_aColumns.end(); it++ )
        {
            (*it).ToXml(pStrm);
        }
    }
    pStrm->EndElement( A2OUSTR("style:columns") );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
