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
 * Line style of Drawing object.
 ************************************************************************/
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFDRAWLINESTYLE_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_XFILTER_XFDRAWLINESTYLE_HXX

#include "xfstyle.hxx"
#include "xfcolor.hxx"

/**
 * @brief
 * Border line style for drawing object.
 */
class XFDrawLineStyle : public XFStyle
{
public:
    XFDrawLineStyle();

public:
    /**
     * @descr   Set line width.
     */
    void    SetWidth(double width);

    /**
     * @descr   Set line color.
     */
    void    SetColor(const XFColor& color);

    /**
     * @descr   Set line transparency.
     */
    void    SetTransparency(sal_Int32 transparency);

    /**
     * @descr   Set line type, solid,dash,dot?
     */
    void    SetLineStyle(enumXFLineStyle style);

    /**
     * @descr   Set first dot number of dash line.
     */
    void    SetDot1Number(sal_Int32 number);

    /**
     * @descr   Set second dot number of dash line.
     */
    void    SetDot2Number(sal_Int32 number);

    /**
     * @descr   Set first dot length.
     */
    void    SetDot1Length(double length);

    /**
     * @descr   Set second dot length.
     */
    void    SetDot2Length(double length);

    /**
     * @descr   Set space between dash dot.
     */
    void    SetSpace(double space);

    /**
     * @descr   decide whether it's a solid line.
     */
    bool    IsSolid();

    double  GetWidth();

    XFColor GetColor();

    sal_Int32   GetTransparency();

    virtual enumXFStyle GetStyleFamily() override;

    virtual void    ToXml(IXFStream *pStrm) override;

private:
    enumXFLineStyle m_eLineStyle;
    XFColor m_aColor;
    double  m_fWidth;
    sal_Int32   m_nTransparency;
    sal_Int32   m_nNumber1;
    sal_Int32   m_nNumber2;
    double  m_fSpace;
    double  m_fLength1;
    double  m_fLength2;
};

inline void XFDrawLineStyle::SetWidth(double width)
{
    m_fWidth = width;
}

inline void XFDrawLineStyle::SetColor(const XFColor& color)
{
    m_aColor = color;
}

inline void XFDrawLineStyle::SetTransparency(sal_Int32 transparency)
{
    m_nTransparency = transparency;
}

inline void XFDrawLineStyle::SetLineStyle(enumXFLineStyle style)
{
    m_eLineStyle = style;
}

inline void XFDrawLineStyle::SetDot1Number(sal_Int32 number)
{
    m_nNumber1 = number;
}

inline void XFDrawLineStyle::SetDot2Number(sal_Int32 number)
{
    m_nNumber2 = number;
}

inline void XFDrawLineStyle::SetDot1Length(double length)
{
    m_fLength1 = length;
}

inline void XFDrawLineStyle::SetDot2Length(double length)
{
    m_fLength2 = length;
}

inline void XFDrawLineStyle::SetSpace(double space)
{
    m_fSpace = space;
}

inline bool XFDrawLineStyle::IsSolid()
{
    return (m_eLineStyle==enumXFLineSolid);
}

inline double XFDrawLineStyle::GetWidth()
{
    return m_fWidth;
}

inline XFColor XFDrawLineStyle::GetColor()
{
    return m_aColor;
}

inline sal_Int32 XFDrawLineStyle::GetTransparency()
{
    return m_nTransparency;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
