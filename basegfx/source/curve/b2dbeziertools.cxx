/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <basegfx/curve/b2dbeziertools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    B2DCubicBezierHelper::B2DCubicBezierHelper(const B2DCubicBezier& rBase, sal_uInt32 nDivisions)
    :   maLengthArray(),
        mnEdgeCount(0)
    {
        const bool bIsBezier(rBase.isBezier());

        if(bIsBezier)
        {
            // check nDivisions; at least one is needed, but also prevent too big values
            if(nDivisions < 1)
            {
                nDivisions = 1;
            }
            else if(nDivisions > 1000)
            {
                nDivisions = 1000;
            }

            // set nEdgeCount
            mnEdgeCount = nDivisions + 1;

            // fill in maLengthArray
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

        // fDistance is in ]0.0 .. fLength[

        if(1 == mnEdgeCount)
        {
            // not a bezier, linear edge
            return fDistance / fLength;
        }

        // it is a bezier
        ::std::vector< double >::const_iterator aIter = ::std::lower_bound(maLengthArray.begin(), maLengthArray.end(), fDistance);
        const sal_uInt32 nIndex(aIter - maLengthArray.begin());
        const double fHighBound(maLengthArray[nIndex]);
        const double fLowBound(nIndex ?  maLengthArray[nIndex - 1] : 0.0);
        const double fLinearInterpolatedLength((fDistance - fLowBound) / (fHighBound - fLowBound));

        return (static_cast< double >(nIndex) + fLinearInterpolatedLength) / static_cast< double >(mnEdgeCount);
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
