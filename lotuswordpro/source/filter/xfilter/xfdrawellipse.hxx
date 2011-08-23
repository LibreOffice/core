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
 * Eclipse object.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2004-2-18 create this file.
 ************************************************************************/
#ifndef		_XFDRAWELLIPSE_HXX
#define		_XFDRAWELLIPSE_HXX

#include	"xfdrawobj.hxx"

/**
 * @brief
 * Ellipse obejct for drawing layer.
 * You can set Ellipse position by setting it's center point and radius or calling XFDrawEllipse::SetPosition function.
 * If you using the first method,i'll calc the real position.
 *
 * You can draw part of an ellipse by using SetAngle(), for example, use SetaAngle(0,180) to draw half a ellipse.
 * When draw part of an ellipse, you can use SetDrawKind() to decide which kind of object to draw, an arch, a section,
 * or a cut.
 */
class XFDrawEllipse : public XFDrawObject
{
public:
    XFDrawEllipse();

public:
    /**
     * @descr	Set ellipse center point.
     */
    void	SetCenterPoint(double x, double y);

    /**
     * @descr	Set ellipse center point.
     */
    void	SetCenterPoint(XFPoint pt);

    /**
     * @descr	Set ellipse radius.
     */
    void	SetRadius(double radiusX, double radiusY);

    /**
     * @descr	If only draw part of the ellipse, set the start angle and end angle.
     */
    void	SetAngle(double start, double end);

    /**
     * @descr	Set ellipse draw type,please refer to enumXFDrawKind.
     */
    void	SetDrawKind(enumXFDrawKind kind);

    /**
     * @descr	Output ellipse obejct.
     */
    virtual void	ToXml(IXFStream *pStrm);

private:
    XFPoint	m_aCenterPoint;
    double	m_fRadiusX;
    double	m_fRadiusY;
    double	m_fStartAngle;
    double	m_fEndAngle;
    enumXFDrawKind	m_eDrawKind;
    sal_Bool m_bUseAngle;
};

inline void XFDrawEllipse::SetCenterPoint(double x, double y)
{
    m_aCenterPoint = XFPoint(x,y);
}

inline void XFDrawEllipse::SetCenterPoint(XFPoint pt)
{
    m_aCenterPoint = pt;
}

inline void XFDrawEllipse::SetRadius(double radiusX, double radiusY)
{
    m_fRadiusX = radiusX;
    m_fRadiusY = radiusY;
}

inline void XFDrawEllipse::SetAngle(double start, double end)
{
    assert(start>=0&&start<=360);
    assert(end>=0&&end<=360);

    m_fStartAngle = start;
    m_fEndAngle = end;
    m_bUseAngle = sal_True;
    m_eDrawKind = enumXFDrawKindCut;
}

inline void XFDrawEllipse::SetDrawKind(enumXFDrawKind kind)
{
    m_eDrawKind = kind;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
