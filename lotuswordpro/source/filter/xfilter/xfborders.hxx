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
 * 2005-01-17 changed for the XFColor object.
 ************************************************************************/
#ifndef		_XFBORDERS_HXX
#define		_XFBORDERS_HXX

#include	"xfglobal.hxx"
#include	"xfcolor.hxx"

/**
 * @brief
 * Single border obejct for all objects with borders.
 */
class XFBorder
{
public:
    XFBorder();

public:
    /**
     * @descr	Set border color.
     */
    void		SetColor(XFColor& color);

    /**
     * @descr	Set border width in cm.
     */
    void		SetWidth(double width);

    /**
     * @descr	Set whether the border has double line.
     */
    void		SetDoubleLine(sal_Bool dual,sal_Bool bSameWidth);

    /**
     * @descr	Set inner line width for border with double line.
     */
    void		SetWidthInner(double inner);

    /**
     * @descr	Set space between double lines fo border.
     */
    void		SetWidthSpace(double space);

    /**
     * @descr	Set outter line width for border with double line.
     */
    void		SetWidthOutter(double outer);

private:
    /**
     * @descr	Forst line width to OOo border width format.
     */
    rtl::OUString	GetLineWidth();

    rtl::OUString	ToString();

    friend bool operator==(XFBorder& b1, XFBorder& b2);
    friend bool operator!=(XFBorder& b1, XFBorder& b2);
    friend class XFBorders;

private:
    double		m_fOffset;
    XFColor		m_aColor;
    sal_Bool	m_bDouble;
    sal_Bool	m_bSameWidth;
    double		m_fWidthInner;
    double		m_fWidthSpace;
    double		m_fWidthOutter;
};

/**
 * @brief
 * Borders for object with borders.
 * Borders may include (right,left,top,bottom) borders.
 */
class XFBorders
{
public:
    XFBorders(){}

public:
    /**
     * @descr	Set color of border side.
     */
    void	SetColor(enumXFBorder side, XFColor& color);

    /**
     * @descr	Set width or border side.
     */
    void	SetWidth(enumXFBorder side, double width);

    /**
     * @descr	Set border side as doubel line.
     * @param	bSameWidth whether two borders are same width.
     */
    void	SetDoubleLine(enumXFBorder side, sal_Bool dual,sal_Bool bSameWidth);

    /**
     * @descr	Set inner border with of border side.
     */
    void	SetWidthInner(enumXFBorder side, double inner);

    /**
     * @descr	Set space between two borders fo border side.
     */
    void	SetWidthSpace(enumXFBorder side, double space);

    /**
     * @descr	Set outter border width of border side.
     */
    void	SetWidthOutter(enumXFBorder side, double outer);

    XFBorder * GetLeft() {return &m_aBorderLeft;};

    XFBorder * GetRight() {return &m_aBorderRight;};

    XFBorder * GetTop() {return &m_aBorderTop;};

    XFBorder * GetBottom() {return &m_aBorderBottom;};

    void	ToXml(IXFStream *pStrm);

    friend bool operator==(XFBorders& b1, XFBorders& b2);
    friend bool operator!=(XFBorders& b1, XFBorders& b2);

private:
    XFBorder	m_aBorderLeft;
    XFBorder	m_aBorderRight;
    XFBorder	m_aBorderTop;
    XFBorder	m_aBorderBottom;
};


#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
