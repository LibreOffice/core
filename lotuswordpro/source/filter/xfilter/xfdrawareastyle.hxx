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
 * Area style of Drawing object.
 * Only support fill color and fill pattern now.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2004-02-21 create this file.
 ************************************************************************/
#ifndef		_XFDRAWAREASTYLE_HXX
#define		_XFDRAWAREASTYLE_HXX

#include	"xfstyle.hxx"
#include	"xfcolor.hxx"

class XFDrawAreaStyle : public XFStyle
{
public:
    XFDrawAreaStyle();

public:
    void	SetAreaStyle(enumXFAreaStyle style);

    void	SetLineStyle(enumXFAreaLineStyle style);

    void	SetBackColor(XFColor& color);

    void	SetLineColor(XFColor& color);

    void	SetLineAngle(sal_Int32 angle);

    void	SetLineSpace(double space);

    enumXFAreaStyle	GetAreaStyle();

    XFColor	GetBackColor();

    virtual enumXFStyle	GetStyleFamily();

    virtual void	ToXml(IXFStream *pStrm);

private:
    enumXFAreaStyle		m_eAreaStyle;
    enumXFAreaLineStyle	m_eLineStyle;
    XFColor		m_aBackColor;
    XFColor		m_aLineColor;
    sal_Int32	m_nAngle;
    double		m_fSpace;
};

inline void XFDrawAreaStyle::SetAreaStyle(enumXFAreaStyle style)
{
    m_eAreaStyle = style;
}

inline void XFDrawAreaStyle::SetLineStyle(enumXFAreaLineStyle style)
{
    m_eLineStyle = style;
}

inline void	XFDrawAreaStyle::SetBackColor(XFColor& color)
{
    m_aBackColor = color;
}

inline void XFDrawAreaStyle::SetLineColor(XFColor& color)
{
    m_aLineColor = color;
}

inline void XFDrawAreaStyle::SetLineAngle(sal_Int32 angle)
{
    m_nAngle = angle;
}

inline void XFDrawAreaStyle::SetLineSpace(double space)
{
    m_fSpace = space;
}

inline enumXFAreaStyle	XFDrawAreaStyle::GetAreaStyle()
{
    return m_eAreaStyle;
}

inline XFColor XFDrawAreaStyle::GetBackColor()
{
    return m_aBackColor;
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
