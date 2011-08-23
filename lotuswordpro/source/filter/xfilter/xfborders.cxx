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
/*************************************************************************
 * Change History
 * 2005-01-14 create this file.
 * 2005-01-17 change for the XFColor object.
 ************************************************************************/
#include	"xfborders.hxx"

XFBorder::XFBorder()
{
    m_fOffset = 0;
    m_bDouble = sal_False;
    m_bSameWidth = sal_False;
    m_fWidthInner = 0;
    m_fWidthSpace = 0;
    m_fWidthOutter = 0;
}

void	XFBorder::SetColor(XFColor& color)
{
    m_aColor = color;
}

void	XFBorder::SetWidth(double width)
{
    if( !m_bDouble )
    {
        m_fWidthInner = width;
        m_bSameWidth = sal_True;
    }
    else if( m_bDouble && m_bSameWidth )
    {
        m_fWidthInner = width;
        m_fWidthOutter = width;
        m_fWidthSpace = width;
    }
}

void	XFBorder::SetDoubleLine(sal_Bool dual,sal_Bool bSameWidth)
{
    m_bDouble = dual;
    m_bSameWidth = bSameWidth;
}

void	XFBorder::SetWidthInner(double inner)
{
    assert(m_bDouble);
    m_fWidthInner = inner;
}

void	XFBorder::SetWidthSpace(double space)
{
    assert(m_bDouble);
    m_fWidthSpace = space;
}

void	XFBorder::SetWidthOutter(double outer)
{
    assert(m_bDouble);
    m_fWidthOutter = outer;
}

rtl::OUString	XFBorder::GetLineWidth()
{
    rtl::OUString	str;

    if( m_bDouble )
    {
        str = FloatToOUString(m_fWidthInner);
        str += A2OUSTR("cm ") + DoubleToOUString(m_fWidthSpace);
        str += A2OUSTR("cm ") + DoubleToOUString(m_fWidthOutter) + A2OUSTR("cm");
    }
    return str;
}

rtl::OUString	XFBorder::ToString()
{
    rtl::OUString str;

    if( m_bDouble )
    {
        double width = m_fWidthInner + m_fWidthSpace + m_fWidthOutter;
        if( width<FLOAT_MIN )
        {
            return str;
        }

        str = DoubleToOUString(width) + A2OUSTR("cm");
        str += A2OUSTR(" double ") + m_aColor.ToString();
    }
    else
    {
        double width = m_fWidthInner;
        if( width<FLOAT_MIN )
        {
            return str;
        }
        str = DoubleToOUString(width) + A2OUSTR("cm");
        str += A2OUSTR(" solid ") + m_aColor.ToString();
    }
    return str;
}

bool operator==(XFBorder& b1, XFBorder& b2)
{
    if( b1.m_fOffset != b2.m_fOffset )
        return false;
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
            b1.m_fWidthOutter != b2.m_fWidthOutter
            )
            return false;
    }
    if( b1.m_aColor != b2.m_aColor )
        return sal_False;
    return true;
}

bool operator!=(XFBorder& b1, XFBorder& b2)
{
    return !(b1 == b2);
}

//
//XFBorders:
//
void	XFBorders::SetColor(enumXFBorder side, XFColor& color)
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

void	XFBorders::SetWidth(enumXFBorder side, double width)
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

void	XFBorders::SetDoubleLine(enumXFBorder side, sal_Bool dual,sal_Bool bSameWidth)
{
    switch(side)
    {
    case enumXFBorderLeft:
        m_aBorderLeft.SetDoubleLine(dual,bSameWidth);
        break;
    case enumXFBorderRight:
        m_aBorderRight.SetDoubleLine(dual,bSameWidth);
        break;
    case enumXFBorderTop:
        m_aBorderTop.SetDoubleLine(dual,bSameWidth);
        break;
    case enumXFBorderBottom:
        m_aBorderBottom.SetDoubleLine(dual,bSameWidth);
        break;
    default:
        break;
    }
}

void	XFBorders::SetWidthInner(enumXFBorder side, double inner)
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

void	XFBorders::SetWidthSpace(enumXFBorder side, double space)
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

void	XFBorders::SetWidthOutter(enumXFBorder side, double outer)
{
    switch(side)
    {
    case enumXFBorderLeft:
        m_aBorderLeft.SetWidthOutter(outer);
        break;
    case enumXFBorderRight:
        m_aBorderRight.SetWidthOutter(outer);
        break;
    case enumXFBorderTop:
        m_aBorderTop.SetWidthOutter(outer);
        break;
    case enumXFBorderBottom:
        m_aBorderBottom.SetWidthOutter(outer);
        break;
    default:
        break;
    }
}

bool	operator ==(XFBorders& b1, XFBorders& b2)
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

bool operator!=(XFBorders& b1, XFBorders& b2)
{
    return !(b1 == b2);
}

void	XFBorders::ToXml(IXFStream *pStrm)
{
    IXFAttrList	*pAttrList = pStrm->GetAttrList();

    if( m_aBorderLeft.GetLineWidth().getLength() )
        pAttrList->AddAttribute( A2OUSTR("style:border-line-width-left"), m_aBorderLeft.GetLineWidth() );
    if( m_aBorderRight.GetLineWidth().getLength() )
        pAttrList->AddAttribute( A2OUSTR("style:border-line-width-right"), m_aBorderRight.GetLineWidth() );
    if( m_aBorderTop.GetLineWidth().getLength() )
        pAttrList->AddAttribute( A2OUSTR("style:border-line-width-top"), m_aBorderTop.GetLineWidth() );
    if( m_aBorderBottom.GetLineWidth().getLength() )
        pAttrList->AddAttribute( A2OUSTR("style:border-line-width-bottom"), m_aBorderBottom.GetLineWidth() );

    if( m_aBorderLeft.ToString().getLength() )
        pAttrList->AddAttribute( A2OUSTR("fo:border-left"), m_aBorderLeft.ToString() );
    else
        pAttrList->AddAttribute( A2OUSTR("fo:border-left"), A2OUSTR("none") );

    if( m_aBorderRight.ToString().getLength() )
        pAttrList->AddAttribute( A2OUSTR("fo:border-right"), m_aBorderRight.ToString() );
    else
        pAttrList->AddAttribute( A2OUSTR("fo:border-right"), A2OUSTR("none") );

    if( m_aBorderTop.ToString().getLength() )
        pAttrList->AddAttribute( A2OUSTR("fo:border-top"), m_aBorderTop.ToString() );
    else
        pAttrList->AddAttribute( A2OUSTR("fo:border-top"), A2OUSTR("none") );

    if( m_aBorderBottom.ToString().getLength() )
        pAttrList->AddAttribute( A2OUSTR("fo:border-bottom"), m_aBorderBottom.ToString() );
    else
        pAttrList->AddAttribute( A2OUSTR("fo:border-bottom"), A2OUSTR("none") );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
