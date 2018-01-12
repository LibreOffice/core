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
/**
 * @file
 * For LWP filter architecture prototype
 * Implementation file of SdwRectangle.
 */

#include "lwpsdwrect.hxx"

/**************************************************************************
 * @short:  Default constructor
**************************************************************************/
SdwRectangle::SdwRectangle()
    : m_bRotated(false)
    // m_nRectCorner array fields are default initialized with Point()
{
}
/**************************************************************************
 * @short:  Constructor
 * @param:  aPt0~aPt3 four corner points of a rectangle.
**************************************************************************/
SdwRectangle::SdwRectangle(const Point& rPt0, const Point& rPt1,
        const Point& rPt2, const Point& rPt3)
    : m_bRotated(rPt0.Y() != rPt1.Y() || rPt0.Y() >= rPt3.Y())
    , m_nRectCorner({{rPt0, rPt1, rPt2, rPt3}})
{
}

/**************************************************************************
 * @short:  Calculate and return center point of the rectangle.
 * @return: center point
**************************************************************************/
Point SdwRectangle::GetRectCenter() const
{
    long nX = static_cast<long>(static_cast<double>(m_nRectCorner[0].X() + m_nRectCorner[2].X())/2 + 0.5);
    long nY = static_cast<long>(static_cast<double>(m_nRectCorner[0].Y() + m_nRectCorner[2].Y())/2 + 0.5);

    return Point(nX, nY);
}
/**************************************************************************
 * @short:  Calculate width of the rectangle.
 * @return: rectangle width.
**************************************************************************/
long SdwRectangle::GetWidth() const
{
    long nX0 = m_nRectCorner[0].X();
    long nY0 = m_nRectCorner[0].Y();
    long nX1 = m_nRectCorner[1].X();
    long nY1 = m_nRectCorner[1].Y();

    return static_cast<long>(CalcDistBetween2Points(nX0, nY0, nX1, nY1));
}
/**************************************************************************
 * @short:  Calculate height of the rectangle.
 * @return: rectangle height.
**************************************************************************/
long SdwRectangle::GetHeight() const
{
    long nX1 = m_nRectCorner[1].X();
    long nY1 = m_nRectCorner[1].Y();
    long nX2 = m_nRectCorner[2].X();
    long nY2 = m_nRectCorner[2].Y();

    return static_cast<long>(CalcDistBetween2Points(nX1, nY1, nX2, nY2));
}
/**************************************************************************
 * @short:  Calculate coordinate of the original rectangle.
 * @return: a prz rectangle
**************************************************************************/
tools::Rectangle SdwRectangle::GetOriginalRect() const
{
    if (m_bRotated)
    {
        long nHeight = GetHeight();
        long nWidth = GetWidth();
        Point aCenter = GetRectCenter();

        Point aLT(aCenter.X()-static_cast<long>(static_cast<double>(nWidth)/2+0.5),
            aCenter.Y()-static_cast<long>(static_cast<double>(nHeight)/2+0.5));
        Point aRB(aLT.X()+nWidth, aLT.Y()+nHeight);

        return tools::Rectangle(aLT, aRB);
    }
    else
    {
        return tools::Rectangle(m_nRectCorner[3], m_nRectCorner[1]);
    }
}
/**************************************************************************
 * @short:  Calculate rotation angle of the rectangle.
 * @return: rotation angle.
**************************************************************************/
double SdwRectangle::GetRotationAngle() const
{
    if (!m_bRotated)
    {
        return 0.00;
    }

    double fX1 = static_cast<double>(m_nRectCorner[1].X());
    double fY1 = static_cast<double>(m_nRectCorner[1].Y());
    double fX2 = static_cast<double>(m_nRectCorner[2].X());
    double fY2 = static_cast<double>(m_nRectCorner[2].Y());
    double fMidX = (fX1 + fX2) / 2;
    double fMidY = (fY1 + fY2) / 2;

    Point aCenter = GetRectCenter();
    double fCenterX = static_cast<double>(aCenter.X());
    double fCenterY = static_cast<double>(aCenter.Y());

    double fAngle = atan2((fMidY - fCenterY), (fMidX - fCenterX));

    return -fAngle;
}

double SdwRectangle::CalcDistBetween2Points(long nX1, long nY1, long nX2, long nY2)
{
    return sqrt(static_cast<double>((nX1-nX2)*(nX1-nX2) + (nY1-nY2)*(nY1-nY2)));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
