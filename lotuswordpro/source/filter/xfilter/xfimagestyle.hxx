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
 * Image style object.
 ************************************************************************/
/*************************************************************************
 * Change History
   2005-02-22	create this file.
   2005-02-25	make it inherit from XFFrameStyle.
 ************************************************************************/
#ifndef		_XFIMAGESTYLE_HXX
#define		_XFIMAGESTYLE_HXX

#include	"xfframestyle.hxx"
#include	"xfrect.hxx"

class XFImageStyle : public XFFrameStyle
{
public:
    XFImageStyle();

public:
    void	SetBrightness(sal_Int32 brightness);

    void	SetContrast(sal_Int32 contrast);

    void	SetGamma(sal_Int32 gamma);

    void	SetTransparent(sal_Int32 transparent);

    void	SetAdjustRed(sal_Int32 red);

    void	SetAdjustGreen(sal_Int32 green);

    void	SetAdjustBlue(sal_Int32 blue);

    void	SetClip(double left, double right, double top, double bottom);

    void	SetFlip(sal_Bool hori, sal_Bool vert);

    void	SetColorMode(enumXFColorMode mode);

    virtual void	ToXml(IXFStream *pStrm);

private:
    sal_Int32	m_nBrightness;
    sal_Int32	m_nContrast;
    sal_Int32	m_nGamma;
    sal_Int32	m_nTransparent;
    sal_Int32	m_nAdjustRed;
    sal_Int32	m_nAdjustGreen;
    sal_Int32	m_nAdjustBlue;
    double		m_fClipLeft;
    double		m_fClipRight;
    double		m_fClipTop;
    double		m_fClipBottom;
    sal_Bool	m_bHoriFlip;
    sal_Bool	m_bVertFlip;
    enumXFColorMode	m_eColorMode;
};

inline void	XFImageStyle::SetBrightness(sal_Int32 brightness)
{
    m_nBrightness = brightness;
}

inline void	XFImageStyle::SetContrast(sal_Int32 contrast)
{
    m_nContrast = contrast;
}

inline void	XFImageStyle::SetGamma(sal_Int32 gamma)
{
    m_nGamma = gamma;
}

inline void	XFImageStyle::SetTransparent(sal_Int32 transparent)
{
    m_nTransparent = transparent;
}

inline void	XFImageStyle::SetAdjustRed(sal_Int32 red)
{
    m_nAdjustRed = red;
}

inline void	XFImageStyle::SetAdjustGreen(sal_Int32 green)
{
    m_nAdjustGreen = green;
}

inline void	XFImageStyle::SetAdjustBlue(sal_Int32 blue)
{
    m_nAdjustBlue = blue;
}

inline void XFImageStyle::SetClip(double left, double right, double top, double bottom)
{
    m_fClipLeft = left;
    m_fClipRight = right;
    m_fClipTop = top;
    m_fClipBottom = bottom;
}

inline void	XFImageStyle::SetFlip(sal_Bool hori, sal_Bool vert)
{
    m_bHoriFlip = hori;
    m_bVertFlip = vert;
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
