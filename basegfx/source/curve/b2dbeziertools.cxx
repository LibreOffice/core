/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"
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

    double B2DCubicBezierHelper::relativeToDistance(double fRelative) const
    {
        if(fRelative <= 0.0)
        {
            return 0.0;
        }

        const double fLength(getLength());

        if(fTools::moreOrEqual(fRelative, 1.0))
        {
            return fLength;
        }

        // fRelative is in ]0.0 .. 1.0[

        if(1 == mnEdgeCount)
        {
            // not a bezier, linear edge
            return fRelative * fLength;
        }

        // fRelative is in ]0.0 .. 1.0[
        const double fIndex(fRelative * static_cast< double >(mnEdgeCount));
        double fIntIndex;
        const double fFractIndex(modf(fIndex, &fIntIndex));
        const sal_uInt32 nIntIndex(static_cast< sal_uInt32 >(fIntIndex));
        const double fStartDistance(nIntIndex ? maLengthArray[nIntIndex - 1] : 0.0);

        return fStartDistance + ((maLengthArray[nIntIndex] - fStartDistance) * fFractIndex);
    }
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////

// eof
