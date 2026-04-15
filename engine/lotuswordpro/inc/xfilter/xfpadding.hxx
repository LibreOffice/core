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
 * Padding for paragraph.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFPADDING_HXX
#define INCLUDED_LOTUSWORDPRO_INC_XFILTER_XFPADDING_HXX

#include <xfilter/ixfstream.hxx>
#include <xfilter/ixfattrlist.hxx>

#define XFPADDING_FLAG_LEFT     0X00000001
#define XFPADDING_FLAG_RIGHT    0X00000002
#define XFPADDING_FLAG_TOP      0X00000004
#define XFPADDING_FLAG_BOTTOM   0X00000008

/**
 * @brief
 * Paddings for paragraph,frame, ...
 */
class XFPadding
{
public:
    XFPadding();

public:
    /**
     * @descr   Reset all values.
     */
    void    Reset();

    /**
     * @descr   Set padding left.
     */
    void    SetLeft(double left);

    /**
     * @descr   Set padding right.
     */
    void    SetRight(double right);

    /**
     * @descr   Set padding top.
     */
    void    SetTop(double top);

    /**
     * @descr   Set padding bottom.
     */
    void    SetBottom(double bottom);

    /**
     * @descr   Output padding properties.
     */
    void    ToXml(IXFStream *pStrm);

    friend bool operator==(XFPadding const & p1, XFPadding const & p2);
    friend bool operator!=(XFPadding const & p1, XFPadding const & p2);
private:
    int     m_nFlag;
    double  m_fLeft;
    double  m_fRight;
    double  m_fTop;
    double  m_fBottom;
};

inline XFPadding::XFPadding()
{
    Reset();
}

inline void XFPadding::Reset()
{
    m_nFlag = 0;
    m_fLeft = 0;
    m_fRight = 0;
    m_fTop = 0;
    m_fBottom = 0;
}

inline void XFPadding::SetLeft(double left)
{
    m_fLeft = left;
    m_nFlag |= XFPADDING_FLAG_LEFT;
}

inline void XFPadding::SetRight(double right)
{
    m_fRight = right;
    m_nFlag |= XFPADDING_FLAG_RIGHT;
}

inline void XFPadding::SetTop(double top)
{
    m_fTop = top;
    m_nFlag |= XFPADDING_FLAG_TOP;
}

inline void XFPadding::SetBottom(double bottom)
{
    m_fBottom = bottom;
    m_nFlag |= XFPADDING_FLAG_BOTTOM;
}

inline void XFPadding::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    if( (m_fLeft == m_fRight) && (m_fLeft == m_fTop) && (m_fLeft== m_fBottom ) && m_nFlag == 0x00000015 )
    {
        pAttrList->AddAttribute( u"fo:padding"_ustr, OUString::number(m_fLeft) + "cm" );
        return;
    }
    //margin left:
    if( m_nFlag&XFPADDING_FLAG_LEFT )
    {
        pAttrList->AddAttribute( u"fo:padding-left"_ustr, OUString::number(m_fLeft) + "cm" );
    }
    //margin right:
    if( m_nFlag&XFPADDING_FLAG_RIGHT )
    {
        pAttrList->AddAttribute(u"fo:padding-right"_ustr, OUString::number(m_fRight) + "cm" );
    }
    //margin top:
    if( m_nFlag&XFPADDING_FLAG_TOP )
    {
        pAttrList->AddAttribute(u"fo:padding-top"_ustr, OUString::number(m_fTop) + "cm" );
    }
    //margin bottom:
    if( m_nFlag&XFPADDING_FLAG_BOTTOM )
    {
        pAttrList->AddAttribute(u"fo:padding-bottom"_ustr, OUString::number(m_fBottom) + "cm" );
    }
}

inline bool operator==(XFPadding const & p1, XFPadding const & p2)
{
    return (
        (p1.m_nFlag == p2.m_nFlag) &&
        (p1.m_fLeft == p2.m_fLeft) &&
        (p1.m_fRight == p2.m_fRight) &&
        (p1.m_fTop == p2.m_fTop) &&
        (p1.m_fBottom == p2.m_fBottom)
        );
}

inline bool operator!=(XFPadding const & p1, XFPadding const & p2)
{
    return !(p1==p2);
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
