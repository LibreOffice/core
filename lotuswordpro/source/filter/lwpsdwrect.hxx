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
#ifndef _SDWRECTANGLE_HXX
#define _SDWRECTANGLE_HXX

#include "lwpheader.hxx"
#include <tools/gen.hxx>

/**
 * @brief
 *   SdwRectangle: Not like rectangle in SODC, rectagles in Word Pro are
 * defined by 4 points. So they can be any posture in the two-dimensional
 * coordinate system. The class is used to describ such rectangles.
 *	 The foure corner points must be save in menber variable:m_nRectCorner
 * one after another. We assume the input order(the same order in .lwp files)
 * is as below:
 *
 * y ^     m_nRectCorner[3]             m_nRectCorner[2]
 *   |                 |--------------------|
 *   |                 |                    |
 *   |                 |                    |
 *   |                 |--------------------|
 *   |     m_nRectCorner[0]             m_nRectCorner[1]
 *   |
 *   |------------------------------------------------------->
 * (0,0)                                                     x
 *
 */
class SdwRectangle
{
private:
    sal_Bool m_bRotated;

public:
    Point m_nRectCorner[4];

public:

    SdwRectangle();

    SdwRectangle(const Point& rPt0, const Point& rPt1,
        const Point& rPt2, const Point& rPt3);

    SdwRectangle(const SdwRectangle& rOther);

    ~SdwRectangle();

public:
    sal_Bool IsRectRotated() const;

    Point GetRectCenter() const ;

    long GetWidth() const;

    long GetHeight() const;

    Rectangle GetOriginalRect() const;

    double GetRotationAngle() const;

    static double CalcDistBetween2Points(long nX1, long nY1, long nX2, long nY2);

    static Rectangle GetOriginalRect(const Point& rCenter, long nHalfWidth, long nHalfHeight);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
