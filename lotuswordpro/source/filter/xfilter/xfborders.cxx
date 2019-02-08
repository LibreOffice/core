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
 * Border object,now only used by paragraph object.
 ************************************************************************/
#include <xfilter/xfborders.hxx>

XFBorder::XFBorder()
{
    m_bDouble = false;
    m_bSameWidth = false;
    m_fWidthInner = 0;
    m_fWidthSpace = 0;
    m_fWidthOuter = 0;
}

void    XFBorder::SetColor(XFColor const & color)
{
    m_aColor = color;
}

void    XFBorder::SetWidth(double width)
{
    if( !m_bDouble )
    {
        m_fWidthInner = width;
        m_bSameWidth = true;
    }
    else if (m_bSameWidth)
    {
        m_fWidthInner = width;
        m_fWidthOuter = width;
        m_fWidthSpace = width;
    }
}

void    XFBorder::SetDoubleLine(bool dual,bool bSameWidth)
{
    m_bDouble = dual;
    m_bSameWidth = bSameWidth;
}

void    XFBorder::SetWidthInner(double inner)
{
    assert(m_bDouble);
    m_fWidthInner = inner;
}

void    XFBorder::SetWidthSpace(double space)
{
    assert(m_bDouble);
    m_fWidthSpace = space;
}

void    XFBorder::SetWidthOuter(double outer)
{
    assert(m_bDouble);
    m_fWidthOuter = outer;
}

OUString   XFBorder::GetLineWidth()
{
    OUString   str;

    if( m_bDouble )
    {
        str = OUString::number(m_fWidthInner) + "cm " +
            OUString::number(m_fWidthSpace) +  "cm " +
            OUString::number(m_fWidthOuter) + "cm";
    }
    return str;
}

OUString   XFBorder::ToString()
{
    OUString str;

    if( m_bDouble )
    {
        double width = m_fWidthInner + m_fWidthSpace + m_fWidthOuter;
        if( width<FLOAT_MIN )
        {
            return str;
        }

        str = OUString::number(width) + "cm double " + m_aColor.ToString();
    }
    else
    {
        double width = m_fWidthInner;
        if( width<FLOAT_MIN )
        {
            return str;
        }
        str = OUString::number(width) + "cm solid " + m_aColor.ToString();
    }
    return str;
}

bool operator==(XFBorder const & b1, XFBorder const & b2)
{
    if( b1.m_bDouble != b2.m_bDouble )
        return false;
    if( !b1.m_bDouble )
    {
        if( b1.m_fWidthInner != b2.m_fWidthInner )
            return false;
    }
    else
    {
        if( b1.m_bSameWidth != b2.m_bSameWidth )
            return true;
        if( b1.m_fWidthInner != b2.m_fWidthInner ||
            b1.m_fWidthSpace != b2.m_fWidthSpace ||
            b1.m_fWidthOuter != b2.m_fWidthOuter
            )
            return false;
    }
    if( b1.m_aColor != b2.m_aColor )
        return false;
    return true;
}

bool operator!=(XFBorder const & b1, XFBorder const & b2)
{
    return !(b1 == b2);
}

//XFBorders:

void    XFBorders::SetColor(enumXFBorder side, XFColor const & color)
{
    switch(side)
    {
    case enumXFBorderLeft:
        m_aBorderLeft.SetColor(color);
        break;
    case enumXFBorderRight:
        m_aBorderRight.SetColor(color);
        break;
    case enumXFBorderTop:
        m_aBorderTop.SetColor(color);
        break;
    case enumXFBorderBottom:
        m_aBorderBottom.SetColor(color);
        break;
    default:
        break;
    }
}

void    XFBorders::SetWidth(enumXFBorder side, double width)
{
    switch(side)
    {
    case enumXFBorderLeft:
        m_aBorderLeft.SetWidth(width);
        break;
    case enumXFBorderRight:
        m_aBorderRight.SetWidth(width);
        break;
    case enumXFBorderTop:
        m_aBorderTop.SetWidth(width);
        break;
    case enumXFBorderBottom:
        m_aBorderBottom.SetWidth(width);
        break;
    default:
        break;
    }
}

void    XFBorders::SetDoubleLine(enumXFBorder side)
{
    switch(side)
    {
    case enumXFBorderLeft:
        m_aBorderLeft.SetDoubleLine(true/*dual*/,false/*bSameWidth*/);
        break;
    case enumXFBorderRight:
        m_aBorderRight.SetDoubleLine(true/*dual*/,false/*bSameWidth*/);
        break;
    case enumXFBorderTop:
        m_aBorderTop.SetDoubleLine(true/*dual*/,false/*bSameWidth*/);
        break;
    case enumXFBorderBottom:
        m_aBorderBottom.SetDoubleLine(true/*dual*/,false/*bSameWidth*/);
        break;
    default:
        break;
    }
}

void    XFBorders::SetWidthInner(enumXFBorder side, double inner)
{
    switch(side)
    {
    case enumXFBorderLeft:
        m_aBorderLeft.SetWidthInner(inner);
        break;
    case enumXFBorderRight:
        m_aBorderRight.SetWidthInner(inner);
        break;
    case enumXFBorderTop:
        m_aBorderTop.SetWidthInner(inner);
        break;
    case enumXFBorderBottom:
        m_aBorderBottom.SetWidthInner(inner);
        break;
    default:
        break;
    }
}

void    XFBorders::SetWidthSpace(enumXFBorder side, double space)
{
    switch(side)
    {
    case enumXFBorderLeft:
        m_aBorderLeft.SetWidthSpace(space);
        break;
    case enumXFBorderRight:
        m_aBorderRight.SetWidthSpace(space);
        break;
    case enumXFBorderTop:
        m_aBorderTop.SetWidthSpace(space);
        break;
    case enumXFBorderBottom:
        m_aBorderBottom.SetWidthSpace(space);
        break;
    default:
        break;
    }
}

void    XFBorders::SetWidthOuter(enumXFBorder side, double outer)
{
    switch(side)
    {
    case enumXFBorderLeft:
        m_aBorderLeft.SetWidthOuter(outer);
        break;
    case enumXFBorderRight:
        m_aBorderRight.SetWidthOuter(outer);
        break;
    case enumXFBorderTop:
        m_aBorderTop.SetWidthOuter(outer);
        break;
    case enumXFBorderBottom:
        m_aBorderBottom.SetWidthOuter(outer);
        break;
    default:
        break;
    }
}

bool    operator ==(XFBorders const & b1, XFBorders const & b2)
{
    if( b1.m_aBorderLeft != b2.m_aBorderLeft )
        return false;
    if( b1.m_aBorderRight != b2.m_aBorderRight )
        return false;
    if( b1.m_aBorderTop != b2.m_aBorderTop )
        return false;
    if( b1.m_aBorderBottom != b2.m_aBorderBottom )
        return false;
    return true;
}

bool operator!=(XFBorders const & b1, XFBorders const & b2)
{
    return !(b1 == b2);
}

void    XFBorders::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    if( !m_aBorderLeft.GetLineWidth().isEmpty() )
        pAttrList->AddAttribute( "style:border-line-width-left", m_aBorderLeft.GetLineWidth() );
    if( !m_aBorderRight.GetLineWidth().isEmpty() )
        pAttrList->AddAttribute( "style:border-line-width-right", m_aBorderRight.GetLineWidth() );
    if( !m_aBorderTop.GetLineWidth().isEmpty() )
        pAttrList->AddAttribute( "style:border-line-width-top", m_aBorderTop.GetLineWidth() );
    if( !m_aBorderBottom.GetLineWidth().isEmpty() )
        pAttrList->AddAttribute( "style:border-line-width-bottom", m_aBorderBottom.GetLineWidth() );

    if( !m_aBorderLeft.ToString().isEmpty() )
        pAttrList->AddAttribute( "fo:border-left", m_aBorderLeft.ToString() );
    else
        pAttrList->AddAttribute( "fo:border-left", "none" );

    if( !m_aBorderRight.ToString().isEmpty() )
        pAttrList->AddAttribute( "fo:border-right", m_aBorderRight.ToString() );
    else
        pAttrList->AddAttribute( "fo:border-right", "none" );

    if( !m_aBorderTop.ToString().isEmpty() )
        pAttrList->AddAttribute( "fo:border-top", m_aBorderTop.ToString() );
    else
        pAttrList->AddAttribute( "fo:border-top", "none" );

    if( !m_aBorderBottom.ToString().isEmpty() )
        pAttrList->AddAttribute( "fo:border-bottom", m_aBorderBottom.ToString() );
    else
        pAttrList->AddAttribute( "fo:border-bottom", "none" );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
