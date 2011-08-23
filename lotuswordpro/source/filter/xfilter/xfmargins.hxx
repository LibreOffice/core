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
 * Margins object, include left margin,right margin,top margin and bottom margin.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-01-19 create this file.
 ************************************************************************/
#ifndef		_XFMARGINS_HXX
#define		_XFMARGINS_HXX

#include	"xfglobal.hxx"
#include	"ixfproperty.hxx"

#define		XFMARGINS_FLAG_LEFT		0x00000001
#define		XFMARGINS_FLAG_RIGHT	0x00000002
#define		XFMARGINS_FLAG_TOP		0x00000004
#define		XFMARGINS_FLAG_BOTTOM	0x00000008

class XFMargins : public IXFProperty
{
public:
    XFMargins();
public:
    void	Reset();
    void	SetLeft(double left);
    void	SetRight(double right);
    void	SetTop(double top);
    void	SetBottom(double bottom);

    double	GetLeft();
    double	GetRight();
    double	GetTop();
    double	GetBottom();

    virtual void	ToXml(IXFStream *pStrm);

    friend bool operator==(XFMargins& indent1, XFMargins& indent2);
    friend bool operator!=(XFMargins& indent1, XFMargins& indent2);
    XFMargins& operator=(XFMargins& other);
private:
    double	m_fLeft;
    double	m_fRight;
    double	m_fTop;
    double	m_fBottom;
    int		m_nFlag;
};

inline void	XFMargins::SetLeft(double left)
{
    m_fLeft = left;
    m_nFlag |= XFMARGINS_FLAG_LEFT;
}

inline void	XFMargins::SetRight(double right)
{
    m_fRight = right;
    m_nFlag |= XFMARGINS_FLAG_RIGHT;
}

inline void	XFMargins::SetTop(double top)
{
    m_fTop = top;
    m_nFlag |= XFMARGINS_FLAG_TOP;
}

inline void	XFMargins::SetBottom(double bottom)
{
    m_fBottom = bottom;
    m_nFlag |= XFMARGINS_FLAG_BOTTOM;
}

inline double	XFMargins::GetLeft()
{
    return m_fLeft;
}

inline double	XFMargins::GetRight()
{
    return m_fRight;
}

inline double	XFMargins::GetTop()
{
    return m_fTop;
}

inline double	XFMargins::GetBottom()
{
    return m_fBottom;
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
