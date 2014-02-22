/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <basegfx/curve/b2dbeziertools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <algorithm>



namespace basegfx
{
    B2DCubicBezierHelper::B2DCubicBezierHelper(const B2DCubicBezier& rBase, sal_uInt32 nDivisions)
    :   maLengthArray(),
        mnEdgeCount(0)
    {
        const bool bIsBezier(rBase.isBezier());

        if(bIsBezier)
        {
            
            if(nDivisions < 1)
            {
                nDivisions = 1;
            }
            else if(nDivisions > 1000)
            {
                nDivisions = 1000;
            }

            
            mnEdgeCount = nDivisions + 1;

            
            maLengthArray.clear();
            maLengthArray.reserve(mnEdgeCount);
            B2DPoint aCurrent(rBase.getStartPoint());
            double fLength(0.0);

            for(sal_uInt32 a(1);;)
            {
                const B2DPoint aNext(rBase.interpolatePoint((double)a / (double)mnEdgeCount));
                const B2DVector aEdge(aNext - aCurrent);

                fLength += aEdge.getLength();
                maLengthArray.push_back(fLength);

                if(++a < mnEdgeCount)
                {
                    aCurrent = aNext;
                }
                else
                {
                    const B2DPoint aLastNext(rBase.getEndPoint());
                    const B2DVector aLastEdge(aLastNext - aNext);

                    fLength += aLastEdge.getLength();
                    maLengthArray.push_back(fLength);
                    break;
                }
            }
        }
        else
        {
            maLengthArray.clear();
            maLengthArray.push_back(rBase.getEdgeLength());
            mnEdgeCount = 1;
        }
    }

    double B2DCubicBezierHelper::distanceToRelative(double fDistance) const
    {
        if(fDistance <= 0.0)
        {
            return 0.0;
        }

        const double fLength(getLength());

        if(fTools::moreOrEqual(fDistance, fLength))
        {
            return 1.0;
        }

        

        if(1 == mnEdgeCount)
        {
            
            return fDistance / fLength;
        }

        
        ::std::vector< double >::const_iterator aIter = ::std::lower_bound(maLengthArray.begin(), maLengthArray.end(), fDistance);
        const sal_uInt32 nIndex(aIter - maLengthArray.begin());
        const double fHighBound(maLengthArray[nIndex]);
        const double fLowBound(nIndex ?  maLengthArray[nIndex - 1] : 0.0);
        const double fLinearInterpolatedLength((fDistance - fLowBound) / (fHighBound - fLowBound));

        return (static_cast< double >(nIndex) + fLinearInterpolatedLength) / static_cast< double >(mnEdgeCount);
    }

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
