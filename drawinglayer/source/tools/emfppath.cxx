/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <sal/log.hxx>
#include "emfppath.hxx"

namespace
{
    const unsigned char nTopBitInt7 = 0x80;
    const unsigned char nSignBitInt7 = 0x40;
    // include the sign bit so if it's negative we get
    // that "missing" bit pre-set to 1
    const unsigned char nValueMaskInt7 = 0x7F;
}

namespace emfplushelper
{
    typedef double matrix [4][4];

    constexpr sal_uInt32 nDetails = 8;
    constexpr double alpha[nDetails]
        = { 1. / nDetails, 2. / nDetails, 3. / nDetails, 4. / nDetails,
            5. / nDetails, 6. / nDetails, 7. / nDetails, 8. / nDetails };

    // see 2.2.2.21 EmfPlusInteger7
    //     2.2.2.22 EmfPlusInteger15
    // and 2.2.2.37 EmfPlusPointR Object
    static sal_Int16 GetEmfPlusInteger(SvStream& s)
    {
        unsigned char u8(0);
        s.ReadUChar(u8);

        bool bIsEmfPlusInteger15 = u8 & nTopBitInt7;
        bool bNegative = u8 & nSignBitInt7;
        unsigned char val1 = u8 & nValueMaskInt7;
        if (bNegative)
            val1 |= nTopBitInt7;
        if (!bIsEmfPlusInteger15)
        {
            return static_cast<signed char>(val1);
        }

        s.ReadUChar(u8);
        sal_uInt16 nRet = (val1 << 8) | u8;
        return static_cast<sal_Int16>(nRet);
    }

    EMFPPath::EMFPPath (sal_uInt32 _nPoints, bool bLines)
    {
        if (_nPoints > SAL_MAX_UINT32 / (2 * sizeof(float)))
        {
            _nPoints = SAL_MAX_UINT32 / (2 * sizeof(float));
        }

        nPoints = _nPoints;

        if (!bLines)
            pPointTypes.reset( new sal_uInt8 [_nPoints] );
    }

    EMFPPath::~EMFPPath ()
    {
    }

    void EMFPPath::Read (SvStream& s, sal_uInt32 pathFlags)
    {
        float fx, fy;
        for (sal_uInt32 i = 0; i < nPoints; i++)
        {
            if (pathFlags & 0x800)
            {
                // EMFPlusPointR: points are stored in EMFPlusInteger7 or
                // EMFPlusInteger15 objects, see section 2.2.2.21/22
                // If 0x800 bit is set, the 0x4000 bit is undefined and must be ignored
                sal_Int32 x = GetEmfPlusInteger(s);
                sal_Int32 y = GetEmfPlusInteger(s);
                xPoints.push_back(x);
                yPoints.push_back(y);
                SAL_INFO("drawinglayer.emf", "EMF+\t\t\t" << i << ". EmfPlusPointR [x,y]: " << x << ", " << y);
            }
            else if (pathFlags & 0x4000)
            {
                // EMFPlusPoint: stored in signed short 16bit integer format
                sal_Int16 x, y;

                s.ReadInt16(x).ReadInt16(y);
                SAL_INFO("drawinglayer.emf", "EMF+\t\t\t" << i << ". EmfPlusPoint [x,y]: " << x << ", " << y);
                xPoints.push_back(x);
                yPoints.push_back(y);
            }
            else
            {
                // EMFPlusPointF: stored in Single (float) format
                s.ReadFloat(fx).ReadFloat(fy);
                SAL_INFO("drawinglayer.emf", "EMF+\t" << i << ". EMFPlusPointF [x,y]: " << fx << ", " << fy);
                xPoints.push_back(fx);
                yPoints.push_back(fy);
            }
        }

        if (pPointTypes)
        {
            for (sal_uInt32 i = 0; i < nPoints; i++)
            {
                s.ReadUChar(pPointTypes[i]);
                SAL_INFO("drawinglayer.emf", "EMF+\tpoint type: 0x" << std::hex << static_cast<int>(pPointTypes[i]) << std::dec);
            }
        }

        aPolygon.clear();
    }

    ::basegfx::B2DPolyPolygon& EMFPPath::GetPolygon (EmfPlusHelperData const & rR, bool bMapIt, bool bAddLineToCloseShape)
    {
        ::basegfx::B2DPolygon polygon;
        aPolygon.clear ();
        sal_uInt32 last_normal = 0, p = 0;
        ::basegfx::B2DPoint prev, mapped;
        bool hasPrev = false;

        for (sal_uInt32 i = 0; i < nPoints; i++)
        {
            if (p && pPointTypes && (pPointTypes [i] == 0))
            {
                aPolygon.append (polygon);
                last_normal = i;
                p = 0;
                polygon.clear ();
            }

            if (bMapIt)
                mapped = rR.Map(xPoints[i], yPoints [i]);
            else
                mapped = ::basegfx::B2DPoint(xPoints[i], yPoints[i]);

            if (pPointTypes)
            {
                if ((pPointTypes [i] & 0x07) == 3)
                {
                    if (((i - last_normal )% 3) == 1)
                    {
                        assert(p != 0);
                        polygon.setNextControlPoint (p - 1, mapped);
                        SAL_INFO ("drawinglayer.emf", "EMF+\t\tPolygon append next: " << p - 1 << " mapped: " << mapped.getX () << "," << mapped.getY ());
                        continue;
                    }
                    else if (((i - last_normal) % 3) == 2)
                    {
                        prev = mapped;
                        hasPrev = true;
                        continue;
                    }
                }
                else
                {
                    last_normal = i;
                }
            }

            polygon.append (mapped);
            SAL_INFO ("drawinglayer.emf", "EMF+\t\tPoint: " << xPoints[i] << "," << yPoints[i] << " mapped: " << mapped.getX () << ":" << mapped.getY ());

            if (hasPrev)
            {
                polygon.setPrevControlPoint (p, prev);
                SAL_INFO ("drawinglayer.emf", "EMF+\t\tPolygon append prev: " << p << " mapped: " << prev.getX () << "," << prev.getY ());
                hasPrev = false;
            }

            p++;

            if (pPointTypes && (pPointTypes [i] & 0x80)) // closed polygon
            {
                polygon.setClosed (true);
                aPolygon.append (polygon);
                SAL_INFO ("drawinglayer.emf", "EMF+\t\tClose polygon");
                last_normal = i + 1;
                p = 0;
                polygon.clear ();
            }
        }

        // Draw an extra line between the last point and the first point, to close the shape.
        if (bAddLineToCloseShape)
        {
            polygon.setClosed (true);
        }

        if (polygon.count ())
        {
            aPolygon.append (polygon);

#if OSL_DEBUG_LEVEL > 1
            for (unsigned int i=0; i<aPolygon.count(); i++) {
                polygon = aPolygon.getB2DPolygon(i);
                SAL_INFO ("drawinglayer.emf", "EMF+\t\tPolygon: " << i);
                for (unsigned int j=0; j<polygon.count(); j++) {
                    ::basegfx::B2DPoint point = polygon.getB2DPoint(j);
                    SAL_INFO ("drawinglayer.emf", "EMF+\t\t\tPoint: " << point.getX() << "," << point.getY());
                    if (polygon.isPrevControlPointUsed(j)) {
                        point = polygon.getPrevControlPoint(j);
                        SAL_INFO ("drawinglayer.emf", "EMF+\t\t\tPrev: " << point.getX() << "," << point.getY());
                    }
                    if (polygon.isNextControlPointUsed(j)) {
                        point = polygon.getNextControlPoint(j);
                        SAL_INFO ("drawinglayer.emf", "EMF+\t\t\tNext: " << point.getX() << "," << point.getY());
                    }
                }
            }
#endif
        }

        return aPolygon;
    }

    static void GetCardinalMatrix(float tension, matrix& m)
    {
        m[0][1] = 2. - tension;
        m[0][2] = tension - 2.;
        m[1][0] = 2. * tension;
        m[1][1] = tension - 3.;
        m[1][2] = 3. - 2. * tension;
        m[3][1] = 1.;
        m[0][3] = m[2][2] = tension;
        m[0][0] = m[1][3] = m[2][0] = -tension;
        m[2][1] = m[2][3] = m[3][0] = m[3][2] = m[3][3] = 0.;
    }

    static double calculateSplineCoefficients(float p0, float p1, float p2, float p3, sal_uInt32 step, matrix m)
    {
        double a = m[0][0] * p0 + m[0][1] * p1 + m[0][2] * p2 + m[0][3] * p3;
        double b = m[1][0] * p0 + m[1][1] * p1 + m[1][2] * p2 + m[1][3] * p3;
        double c = m[2][0] * p0 + m[2][2] * p2;
        double d = p1;
        return (d + alpha[step] * (c + alpha[step] * (b + alpha[step] * a)));
    }

    ::basegfx::B2DPolyPolygon& EMFPPath::GetCardinalSpline(EmfPlusHelperData const& rR, float fTension,
                                                           sal_uInt32 aOffset, sal_uInt32 aNumSegments)
    {
        ::basegfx::B2DPolygon polygon;
        matrix mat;
        double x, y;
        if (aNumSegments >= nPoints)
            aNumSegments = nPoints - 1;
        GetCardinalMatrix(fTension, mat);
        // duplicate first point
        xPoints.push_front(xPoints.front());
        yPoints.push_front(yPoints.front());
        // duplicate last point
        xPoints.push_back(xPoints.back());
        yPoints.push_back(yPoints.back());

        for (sal_uInt32 i = 3 + aOffset; i < aNumSegments + 3; i++)
        {
            for (sal_uInt32 s = 0; s < nDetails; s++)
            {
                x = calculateSplineCoefficients(xPoints[i - 3], xPoints[i - 2], xPoints[i - 1],
                                                xPoints[i], s, mat);
                y = calculateSplineCoefficients(yPoints[i - 3], yPoints[i - 2], yPoints[i - 1],
                                                yPoints[i], s, mat);
                polygon.append(rR.Map(x, y));
            }
        }
        if (polygon.count())
            aPolygon.append(polygon);
        return aPolygon;
    }

    ::basegfx::B2DPolyPolygon& EMFPPath::GetClosedCardinalSpline(EmfPlusHelperData const& rR, float fTension)
    {
        ::basegfx::B2DPolygon polygon;
        matrix mat;
        double x, y;
        GetCardinalMatrix(fTension, mat);
        // add three first points at the end
        xPoints.push_back(xPoints[0]);
        yPoints.push_back(yPoints[0]);
        xPoints.push_back(xPoints[1]);
        yPoints.push_back(yPoints[1]);
        xPoints.push_back(xPoints[2]);
        yPoints.push_back(yPoints[2]);

        for (sal_uInt32 i = 3; i < nPoints + 3; i++)
        {
            for (sal_uInt32 s = 0; s < nDetails; s++)
            {
                x = calculateSplineCoefficients(xPoints[i - 3], xPoints[i - 2], xPoints[i - 1],
                                                xPoints[i], s, mat);
                y = calculateSplineCoefficients(yPoints[i - 3], yPoints[i - 2], yPoints[i - 1],
                                                yPoints[i], s, mat);
                polygon.append(rR.Map(x, y));
            }
        }
        polygon.setClosed(true);
        if (polygon.count())
            aPolygon.append(polygon);
        return aPolygon;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
