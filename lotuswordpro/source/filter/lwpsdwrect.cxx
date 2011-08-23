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
/*****************************************************************************
 * Change History
 *  Mar 2005			Revised for lwpfilter
 ****************************************************************************/
/**
 * @file
 * For LWP filter architecture prototype
 * Implementation file of SdwRectangle.
 */
#include "lwpsdwrect.hxx"

/**************************************************************************
 * @date:	11/19/2004
 * @short:	Default constructor
**************************************************************************/
SdwRectangle::SdwRectangle() : m_bRotated(sal_False)
{
    for (UINT16 i = 0; i < 4; i++)
    {
        m_nRectCorner[0] = Point(0, 0);
    }
}
/**************************************************************************
 * @date:	11/19/2004
 * @short:	Constructor
 * @param:	aPt0~aPt3 four corner points of a rectangle.
**************************************************************************/
SdwRectangle::SdwRectangle(const Point& rPt0, const Point& rPt1,
        const Point& rPt2, const Point& rPt3) : m_bRotated(sal_True)
{
    m_nRectCorner[0] = rPt0;
    m_nRectCorner[1] = rPt1;
    m_nRectCorner[2] = rPt2;
    m_nRectCorner[3] = rPt3;

    if (rPt0.Y() == rPt1.Y() && rPt0.Y() < rPt3.Y())
    {
        m_bRotated = sal_False;
    }
}

/**************************************************************************
 * @date:	12/15/2004
 * @short:	Copy constructor
**************************************************************************/
SdwRectangle::SdwRectangle(const SdwRectangle& rOther)
{
    m_nRectCorner[0] = rOther.m_nRectCorner[0];
    m_nRectCorner[1] = rOther.m_nRectCorner[1];
    m_nRectCorner[2] = rOther.m_nRectCorner[2];
    m_nRectCorner[3] = rOther.m_nRectCorner[3];

    m_bRotated  = rOther.IsRectRotated();
}
/**************************************************************************
 * @date:	11/19/2004
 * @short:	Destructor
**************************************************************************/
SdwRectangle::~SdwRectangle()
{
}
/**************************************************************************
 * @date:	11/19/2004
 * @short:	Get the flag whether if the rectangle has been rotated.
 * @return:	m_bRotated rotation flag.
**************************************************************************/
sal_Bool SdwRectangle::IsRectRotated() const
{
    return m_bRotated;
}
/**************************************************************************
 * @date:	11/19/2004
 * @short:	Calculate and return center point of the rectangle.
 * @return:	center point
**************************************************************************/
Point SdwRectangle::GetRectCenter() const
{
    long nX = (long)((double)(m_nRectCorner[0].X() + m_nRectCorner[2].X())/2 + 0.5);
    long nY = (long)((double)(m_nRectCorner[0].Y() + m_nRectCorner[2].Y())/2 + 0.5);

    return Point(nX, nY);
}
/**************************************************************************
 * @date:	11/19/2004
 * @short:	Calculate width of the rectangle.
 * @return:	rectangle width.
**************************************************************************/
long SdwRectangle::GetWidth() const
{
    long nX0 = m_nRectCorner[0].X();
    long nY0 = m_nRectCorner[0].Y();
    long nX1 = m_nRectCorner[1].X();
    long nY1 = m_nRectCorner[1].Y();

    return (long)CalcDistBetween2Points(nX0, nY0, nX1, nY1);
}
/**************************************************************************
 * @date:	11/19/2004
 * @short:	Calculate height of the rectangle.
 * @return:	rectangle height.
**************************************************************************/
long SdwRectangle::GetHeight() const
{
    long nX1 = m_nRectCorner[1].X();
    long nY1 = m_nRectCorner[1].Y();
    long nX2 = m_nRectCorner[2].X();
    long nY2 = m_nRectCorner[2].Y();

    return (long)CalcDistBetween2Points(nX1, nY1, nX2, nY2);
}
/**************************************************************************
 * @date:	11/19/2004
 * @short:	Calculate coordinate of the original rectangle.
 * @return:	a prz rectangle
**************************************************************************/
Rectangle SdwRectangle::GetOriginalRect() const
{
    if (m_bRotated)
    {
        long nHeight = GetHeight();
        long nWidth = GetWidth();
        Point aCenter = GetRectCenter();

        Point aLT(aCenter.X()-(long)((double)nWidth/2+0.5),
            aCenter.Y()-(long)((double)nHeight/2+0.5));
        Point aRT(aLT.X()+nWidth, aLT.Y());
        Point aLB(aLT.X(), aLT.Y()-nHeight);
        Point aRB(aLT.X()+nWidth, aLT.Y()+nHeight);

        return Rectangle(aLT, aRB);
    }
    else
    {
        return Rectangle(m_nRectCorner[3], m_nRectCorner[1]);
    }
}
/**************************************************************************
 * @date:	11/19/2004
 * @short:	Calculate rotation angle of the rectangle.
 * @return:	rotation angle.
**************************************************************************/
double SdwRectangle::GetRotationAngle() const
{
    if (!m_bRotated)
    {
        return 0.00;
    }

    double fX1 = (double)(m_nRectCorner[1].X());
    double fY1 = (double)(m_nRectCorner[1].Y());
    double fX2 = (double)(m_nRectCorner[2].X());
    double fY2 = (double)(m_nRectCorner[2].Y());
    double fMidX = (fX1 + fX2) / 2;
    double fMidY = (fY1 + fY2) / 2;

    Point aCenter = GetRectCenter();
    double fCenterX = (double)aCenter.X();
    double fCenterY = (double)aCenter.Y();

    double fAngle = atan2((fMidY - fCenterY), (fMidX - fCenterX));

    return -fAngle;
}

double SdwRectangle::CalcDistBetween2Points(long nX1, long nY1, long nX2, long nY2)
{
    return sqrt((double)((nX1-nX2)*(nX1-nX2) + (nY1-nY2)*(nY1-nY2)));
}

Rectangle SdwRectangle::GetOriginalRect(const Point& rCenter, long nHalfWidth, long nHalfHeight)
{
    Point aLT(rCenter.X()-nHalfWidth, rCenter.Y()-nHalfHeight);
    Point aRB(rCenter.X()+nHalfWidth, rCenter.Y()+nHalfHeight);

    return Rectangle(aLT, aRB);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
