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
 * Interfer for all Drawing object.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2004-2-17 create this file.
 ************************************************************************/
#ifndef		_XFDRAWOBJ_HXX
#define		_XFDRAWOBJ_HXX

#include	"xfframe.hxx"
#include	"xfrect.hxx"

#define		XFDRAWOBJECT_FLAG_ROTATE	0X00000001
#define		XFDRAWOBJECT_FLAG_TRANLATE	0X00000002
#define		XFDRAWOBJECT_FLAG_SKEWX		0X00000004
#define		XFDRAWOBJECT_FLAG_SKEWY		0X00000008
#define		XFDRAWOBJECT_FLAG_SCALE		0X00000010

/**
 * @brief
 * Base class for all drawing object(ellipse,rect,circle,...).
 * I can set Postions,anchor,rotate,text style name here.
 *
 * Drawing obejcts can be rotated,scaled and skewed, drawing objects must have positions setted,
 * you can use SetPosition(...).
 */
class XFDrawObject : public XFFrame
{
public:
    XFDrawObject();

public:
    /**
     * @descr	Set style name for drawing text.
     */
    void	SetTextStyleName(rtl::OUString style);

    /**
     * @descr	Set drawing obejct rotate.
     */
    void	SetRotate(double degree, XFPoint aRotatePoint=XFPoint(0,0));

    /**
     * @descr	Set drawing object scale.
     */
    void	SetScale(double cx, double cy);

    /**
     * @descr	Set drawing object skew.
     */
    void	SetSkewX(double cx);

    /**
     * @descr	Set drawing obejct y skew.
     */
    void	SetSkewY(double cy);

    void	ContentToXml(IXFStream *pStrm);

    virtual void	ToXml(IXFStream *pStrm);

protected:
    XFContentContainer	m_aContents;
    rtl::OUString	m_strTextStyle;
    double	m_fRotate;
    XFPoint	m_aRotatePoint;
    double	m_fScaleX;
    double	m_fScaleY;
    double	m_fSkewX;
    double	m_fSkewY;

    unsigned int	m_nFlag;
};

inline void XFDrawObject::SetTextStyleName(rtl::OUString style)
{
    m_strTextStyle = style;
}
/*
inline void XFDrawObject::SetPosition(double x, double y, double width, double height)
{
    SetX(x);
    SetY(y);
    SetWidth(width);
    SetHeight(height);
}

inline void XFDrawObject::SetPosition(XFRect rect)
{
    SetX(rect.GetX());
    SetY(rect.GetY());
    SetWidth(rect.GetWidth());
    SetHeight(rect.GetHeight());
}
*/
inline void XFDrawObject::SetRotate(double degree, XFPoint aRotatePoint)
{
    m_nFlag |= XFDRAWOBJECT_FLAG_ROTATE;
    m_fRotate = degree*2*PI/360;
    m_aRotatePoint = aRotatePoint;
}

inline void XFDrawObject::SetScale(double cx, double cy)
{
    m_nFlag |= XFDRAWOBJECT_FLAG_SCALE;
    m_fScaleX = cx;
    m_fScaleY = cy;
}

inline void XFDrawObject::SetSkewX(double cx)
{
    m_nFlag |= XFDRAWOBJECT_FLAG_SKEWX;
    m_fSkewX = cx*2*PI/360;
}

inline void XFDrawObject::SetSkewY(double cy)
{
    m_nFlag |= XFDRAWOBJECT_FLAG_SKEWY;
    m_fSkewY = cy*2*PI/360;
}


#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
