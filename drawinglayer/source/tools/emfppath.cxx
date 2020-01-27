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
#include <o3tl/safeint.hxx>
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

    EMFPPath::EMFPPath (sal_Int32 _nPoints, bool bLines)
    {
        if (_nPoints<0 || o3tl::make_unsigned(_nPoints)>SAL_MAX_INT32 / (2 * sizeof(float)))
        {
            _nPoints = SAL_MAX_INT32 / (2 * sizeof(float));
        }

        nPoints = _nPoints;
        pPoints.reset( new float [nPoints*2] );

        if (!bLines)
            pPointTypes.reset( new sal_uInt8 [_nPoints] );
    }

    EMFPPath::~EMFPPath ()
    {
    }

    void EMFPPath::Read (SvStream& s, sal_uInt32 pathFlags)
    {
        for (int i = 0; i < nPoints; i ++)
        {
            if (pathFlags & 0x800)
            {
                // EMFPlusPointR: points are stored in EMFPlusInteger7 or
                // EMFPlusInteger15 objects, see section 2.2.2.21/22
                // If 0x800 bit is set, the 0x4000 bit is undefined and must be ignored
                sal_Int32 x = GetEmfPlusInteger(s);
                sal_Int32 y = GetEmfPlusInteger(s);
                pPoints [i*2] = x;
                pPoints [i*2 + 1] = y;
                SAL_INFO("drawinglayer", "EMF+\t\t\tEmfPlusPointR [x,y]: " << x << ", " << y);
            }
            else if (pathFlags & 0x4000)
            {
                // EMFPlusPoint: stored in signed short 16bit integer format
                sal_Int16 x, y;

                s.ReadInt16( x ).ReadInt16( y );
                SAL_INFO ("drawinglayer", "EMF+\t\t\tEmfPlusPoint [x,y]: " << x << "," << y);
                pPoints [i*2] = x;
                pPoints [i*2 + 1] = y;
            }
            else
            {
                // EMFPlusPointF: stored in Single (float) format
                s.ReadFloat( pPoints [i*2] ).ReadFloat( pPoints [i*2 + 1] );
                SAL_INFO ("drawinglayer", "EMF+\t EMFPlusPointF [x,y]: " << pPoints [i*2] << "," << pPoints [i*2 + 1]);
            }
        }

        if (pPointTypes)
        {
            for (int i = 0; i < nPoints; i++)
            {
                s.ReadUChar(pPointTypes[i]);
                SAL_INFO("drawinglayer", "EMF+\tpoint type: " << static_cast<int>(pPointTypes[i]));
            }
        }

        aPolygon.clear();
    }

    ::basegfx::B2DPolyPolygon& EMFPPath::GetPolygon (EmfPlusHelperData const & rR, bool bMapIt, bool bAddLineToCloseShape)
    {
        ::basegfx::B2DPolygon polygon;
        aPolygon.clear ();
        int last_normal = 0, p = 0;
        ::basegfx::B2DPoint prev, mapped;
        bool hasPrev = false;

        for (int i = 0; i < nPoints; i ++)
        {
            if (p && pPointTypes && (pPointTypes [i] == 0))
            {
                aPolygon.append (polygon);
                last_normal = i;
                p = 0;
                polygon.clear ();
            }

            if (bMapIt)
                mapped = rR.Map (pPoints [i*2], pPoints [i*2 + 1]);
            else
                mapped = ::basegfx::B2DPoint (pPoints [i*2], pPoints [i*2 + 1]);

            if (pPointTypes)
            {
                if ((pPointTypes [i] & 0x07) == 3)
                {
                    if (((i - last_normal )% 3) == 1)
                    {
                        polygon.setNextControlPoint (p - 1, mapped);
                        SAL_INFO ("drawinglayer", "EMF+\t\tPolygon append next: " << p - 1 << " mapped: " << mapped.getX () << "," << mapped.getY ());
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
            SAL_INFO ("drawinglayer", "EMF+\t\tPolygon append point: " << pPoints [i*2] << "," << pPoints [i*2 + 1] << " mapped: " << mapped.getX () << ":" << mapped.getY ());

            if (hasPrev)
            {
                polygon.setPrevControlPoint (p, prev);
                SAL_INFO ("drawinglayer", "EMF+\t\tPolygon append  prev: " << p << " mapped: " << prev.getX () << "," << prev.getY ());
                hasPrev = false;
            }

            p ++;

            if (pPointTypes && (pPointTypes [i] & 0x80))
            {
                // closed polygon
                polygon.setClosed (true);
                aPolygon.append (polygon);
                SAL_INFO ("drawinglayer", "EMF+\t\tClose polygon");
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
                SAL_INFO ("drawinglayer", "EMF+\t\tPolygon: " << i);
                for (unsigned int j=0; j<polygon.count(); j++) {
                    ::basegfx::B2DPoint point = polygon.getB2DPoint(j);
                    SAL_INFO ("drawinglayer", "EMF+\t\t\tPoint: " << point.getX() << "," << point.getY());
                    if (polygon.isPrevControlPointUsed(j)) {
                        point = polygon.getPrevControlPoint(j);
                        SAL_INFO ("drawinglayer", "EMF+\t\t\tPrev: " << point.getX() << "," << point.getY());
                    }
                    if (polygon.isNextControlPointUsed(j)) {
                        point = polygon.getNextControlPoint(j);
                        SAL_INFO ("drawinglayer", "EMF+\t\t\tNext: " << point.getX() << "," << point.getY());
                    }
                }
            }
#endif
        }

        return aPolygon;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
