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

#include <osl/diagnose.h>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <rtl/instance.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////

struct CoordinateData2D : public basegfx::B2DPoint
{
public:
    CoordinateData2D() {}

    explicit CoordinateData2D(const basegfx::B2DPoint& rData)
    :   B2DPoint(rData)
    {}

    CoordinateData2D& operator=(const basegfx::B2DPoint& rData)
    {
        B2DPoint::operator=(rData);
        return *this;
    }

    void transform(const basegfx::B2DHomMatrix& rMatrix)
    {
        *this *= rMatrix;
    }
};

//////////////////////////////////////////////////////////////////////////////

class CoordinateDataArray2D
{
    typedef ::std::vector< CoordinateData2D > CoordinateData2DVector;

    CoordinateData2DVector                          maVector;

public:
    explicit CoordinateDataArray2D(sal_uInt32 nCount)
    :   maVector(nCount)
    {
    }

    explicit CoordinateDataArray2D(const CoordinateDataArray2D& rOriginal)
    :   maVector(rOriginal.maVector)
    {
    }

    CoordinateDataArray2D(const CoordinateDataArray2D& rOriginal, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   maVector(rOriginal.maVector.begin() + nIndex, rOriginal.maVector.begin() + (nIndex + nCount))
    {
    }

    sal_uInt32 count() const
    {
        return maVector.size();
    }

    bool operator==(const CoordinateDataArray2D& rCandidate) const
    {
        return (maVector == rCandidate.maVector);
    }

    const basegfx::B2DPoint& getCoordinate(sal_uInt32 nIndex) const
    {
        return maVector[nIndex];
    }

    void setCoordinate(sal_uInt32 nIndex, const basegfx::B2DPoint& rValue)
    {
        maVector[nIndex] = rValue;
    }

    void reserve(sal_uInt32 nCount)
    {
        maVector.reserve(nCount);
    }

    void append(const CoordinateData2D& rValue)
    {
        maVector.push_back(rValue);
    }

    void insert(sal_uInt32 nIndex, const CoordinateData2D& rValue, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // add nCount copies of rValue
            CoordinateData2DVector::iterator aIndex(maVector.begin());
            aIndex += nIndex;
            maVector.insert(aIndex, nCount, rValue);
        }
    }

    void insert(sal_uInt32 nIndex, const CoordinateDataArray2D& rSource)
    {
        const sal_uInt32 nCount(rSource.maVector.size());

        if(nCount)
        {
            // insert data
            CoordinateData2DVector::iterator aIndex(maVector.begin());
            aIndex += nIndex;
            CoordinateData2DVector::const_iterator aStart(rSource.maVector.begin());
            CoordinateData2DVector::const_iterator aEnd(rSource.maVector.end());
            maVector.insert(aIndex, aStart, aEnd);
        }
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // remove point data
            CoordinateData2DVector::iterator aStart(maVector.begin());
            aStart += nIndex;
            const CoordinateData2DVector::iterator aEnd(aStart + nCount);
            maVector.erase(aStart, aEnd);
        }
    }

    void flip(bool bIsClosed)
    {
        if(maVector.size() > 1)
        {
            // to keep the same point at index 0, just flip all points except the
            // first one when closed
            const sal_uInt32 nHalfSize(bIsClosed ? (maVector.size() - 1) >> 1 : maVector.size() >> 1);
            CoordinateData2DVector::iterator aStart(bIsClosed ? maVector.begin() + 1 : maVector.begin());
            CoordinateData2DVector::iterator aEnd(maVector.end() - 1);

            for(sal_uInt32 a(0); a < nHalfSize; a++)
            {
                ::std::swap(*aStart, *aEnd);
                ++aStart;
                --aEnd;
            }
        }
    }

    void removeDoublePointsAtBeginEnd()
    {
        // remove from end as long as there are at least two points
        // and begin/end are equal
        while((maVector.size() > 1) && (maVector[0] == maVector[maVector.size() - 1]))
        {
            maVector.pop_back();
        }
    }

    void removeDoublePointsWholeTrack()
    {
        sal_uInt32 nIndex(0);

        // test as long as there are at least two points and as long as the index
        // is smaller or equal second last point
        while((maVector.size() > 1) && (nIndex <= maVector.size() - 2))
        {
            if(maVector[nIndex] == maVector[nIndex + 1])
            {
                // if next is same as index, delete next
                maVector.erase(maVector.begin() + (nIndex + 1));
            }
            else
            {
                // if different, step forward
                nIndex++;
            }
        }
    }

    void transform(const basegfx::B2DHomMatrix& rMatrix)
    {
        CoordinateData2DVector::iterator aStart(maVector.begin());
        CoordinateData2DVector::iterator aEnd(maVector.end());

        for(; aStart != aEnd; ++aStart)
        {
            aStart->transform(rMatrix);
        }
    }

    const basegfx::B2DPoint* begin() const
    {
        if(maVector.empty())
            return 0;
        else
            return &maVector.front();
    }

    const basegfx::B2DPoint* end() const
    {
        if(maVector.empty())
            return 0;
        else
            return (&maVector.back())+1;
    }

    basegfx::B2DPoint* begin()
    {
        if(maVector.empty())
            return 0;
        else
            return &maVector.front();
    }

    basegfx::B2DPoint* end()
    {
        if(maVector.empty())
            return 0;
        else
            return (&maVector.back())+1;
    }
};

//////////////////////////////////////////////////////////////////////////////

class ControlVectorPair2D
{
    basegfx::B2DVector                          maPrevVector;
    basegfx::B2DVector                          maNextVector;

public:
    ControlVectorPair2D() {}

    const basegfx::B2DVector& getPrevVector() const
    {
        return maPrevVector;
    }

    void setPrevVector(const basegfx::B2DVector& rValue)
    {
        if(rValue != maPrevVector)
            maPrevVector = rValue;
    }

    const basegfx::B2DVector& getNextVector() const
    {
        return maNextVector;
    }

    void setNextVector(const basegfx::B2DVector& rValue)
    {
        if(rValue != maNextVector)
            maNextVector = rValue;
    }

    bool operator==(const ControlVectorPair2D& rData) const
    {
        return (maPrevVector == rData.getPrevVector() && maNextVector == rData.getNextVector());
    }

    void flip()
    {
        ::std::swap(maPrevVector, maNextVector);
    }
};

//////////////////////////////////////////////////////////////////////////////

class ControlVectorArray2D
{
    typedef ::std::vector< ControlVectorPair2D > ControlVectorPair2DVector;

    ControlVectorPair2DVector                           maVector;
    sal_uInt32                                          mnUsedVectors;

public:
    explicit ControlVectorArray2D(sal_uInt32 nCount)
    :   maVector(nCount),
        mnUsedVectors(0)
    {}

    ControlVectorArray2D(const ControlVectorArray2D& rOriginal, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   maVector(),
        mnUsedVectors(0)
    {
        ControlVectorPair2DVector::const_iterator aStart(rOriginal.maVector.begin());
        aStart += nIndex;
        ControlVectorPair2DVector::const_iterator aEnd(aStart);
        aEnd += nCount;
        maVector.reserve(nCount);

        for(; aStart != aEnd; ++aStart)
        {
            if(!aStart->getPrevVector().equalZero())
                mnUsedVectors++;

            if(!aStart->getNextVector().equalZero())
                mnUsedVectors++;

            maVector.push_back(*aStart);
        }
    }

    sal_uInt32 count() const
    {
        return maVector.size();
    }

    bool operator==(const ControlVectorArray2D& rCandidate) const
    {
        return (maVector == rCandidate.maVector);
    }

    bool isUsed() const
    {
        return (0 != mnUsedVectors);
    }

    const basegfx::B2DVector& getPrevVector(sal_uInt32 nIndex) const
    {
        return maVector[nIndex].getPrevVector();
    }

    void setPrevVector(sal_uInt32 nIndex, const basegfx::B2DVector& rValue)
    {
        bool bWasUsed(mnUsedVectors && !maVector[nIndex].getPrevVector().equalZero());
        bool bIsUsed(!rValue.equalZero());

        if(bWasUsed)
        {
            if(bIsUsed)
            {
                maVector[nIndex].setPrevVector(rValue);
            }
            else
            {
                maVector[nIndex].setPrevVector(basegfx::B2DVector::getEmptyVector());
                mnUsedVectors--;
            }
        }
        else
        {
            if(bIsUsed)
            {
                maVector[nIndex].setPrevVector(rValue);
                mnUsedVectors++;
            }
        }
    }

    const basegfx::B2DVector& getNextVector(sal_uInt32 nIndex) const
    {
        return maVector[nIndex].getNextVector();
    }

    void setNextVector(sal_uInt32 nIndex, const basegfx::B2DVector& rValue)
    {
        bool bWasUsed(mnUsedVectors && !maVector[nIndex].getNextVector().equalZero());
        bool bIsUsed(!rValue.equalZero());

        if(bWasUsed)
        {
            if(bIsUsed)
            {
                maVector[nIndex].setNextVector(rValue);
            }
            else
            {
                maVector[nIndex].setNextVector(basegfx::B2DVector::getEmptyVector());
                mnUsedVectors--;
            }
        }
        else
        {
            if(bIsUsed)
            {
                maVector[nIndex].setNextVector(rValue);
                mnUsedVectors++;
            }
        }
    }

    void append(const ControlVectorPair2D& rValue)
    {
        maVector.push_back(rValue);

        if(!rValue.getPrevVector().equalZero())
            mnUsedVectors += 1;

        if(!rValue.getNextVector().equalZero())
            mnUsedVectors += 1;
    }

    void insert(sal_uInt32 nIndex, const ControlVectorPair2D& rValue, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // add nCount copies of rValue
            ControlVectorPair2DVector::iterator aIndex(maVector.begin());
            aIndex += nIndex;
            maVector.insert(aIndex, nCount, rValue);

            if(!rValue.getPrevVector().equalZero())
                mnUsedVectors += nCount;

            if(!rValue.getNextVector().equalZero())
                mnUsedVectors += nCount;
        }
    }

    void insert(sal_uInt32 nIndex, const ControlVectorArray2D& rSource)
    {
        const sal_uInt32 nCount(rSource.maVector.size());

        if(nCount)
        {
            // insert data
            ControlVectorPair2DVector::iterator aIndex(maVector.begin());
            aIndex += nIndex;
            ControlVectorPair2DVector::const_iterator aStart(rSource.maVector.begin());
            ControlVectorPair2DVector::const_iterator aEnd(rSource.maVector.end());
            maVector.insert(aIndex, aStart, aEnd);

            for(; aStart != aEnd; ++aStart)
            {
                if(!aStart->getPrevVector().equalZero())
                    mnUsedVectors++;

                if(!aStart->getNextVector().equalZero())
                    mnUsedVectors++;
            }
        }
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(nCount)
        {
            const ControlVectorPair2DVector::iterator aDeleteStart(maVector.begin() + nIndex);
            const ControlVectorPair2DVector::iterator aDeleteEnd(aDeleteStart + nCount);
            ControlVectorPair2DVector::const_iterator aStart(aDeleteStart);

            for(; mnUsedVectors && aStart != aDeleteEnd; ++aStart)
            {
                if(!aStart->getPrevVector().equalZero())
                    mnUsedVectors--;

                if(mnUsedVectors && !aStart->getNextVector().equalZero())
                    mnUsedVectors--;
            }

            // remove point data
            maVector.erase(aDeleteStart, aDeleteEnd);
        }
    }

    void flip(bool bIsClosed)
    {
        if(maVector.size() > 1)
        {
            // to keep the same point at index 0, just flip all points except the
            // first one when closed
            const sal_uInt32 nHalfSize(bIsClosed ? (maVector.size() - 1) >> 1 : maVector.size() >> 1);
            ControlVectorPair2DVector::iterator aStart(bIsClosed ? maVector.begin() + 1 : maVector.begin());
            ControlVectorPair2DVector::iterator aEnd(maVector.end() - 1);

            for(sal_uInt32 a(0); a < nHalfSize; a++)
            {
                // swap Prev and Next
                aStart->flip();
                aEnd->flip();

                // swap entries
                ::std::swap(*aStart, *aEnd);

                ++aStart;
                --aEnd;
            }

            if(aStart == aEnd)
            {
                // swap Prev and Next at middle element (if exists)
                aStart->flip();
            }

            if(bIsClosed)
            {
                // swap Prev and Next at start element
                maVector.begin()->flip();
            }
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

class ImplBufferedData
{
private:
    // Possibility to hold the last subdivision
    boost::scoped_ptr< basegfx::B2DPolygon >        mpDefaultSubdivision;

    // Possibility to hold the last B2DRange calculation
    boost::scoped_ptr< basegfx::B2DRange >          mpB2DRange;

public:
    ImplBufferedData()
    :   mpDefaultSubdivision(),
        mpB2DRange()
    {}

    const basegfx::B2DPolygon& getDefaultAdaptiveSubdivision(const basegfx::B2DPolygon& rSource) const
    {
        if(!mpDefaultSubdivision)
        {
            const_cast< ImplBufferedData* >(this)->mpDefaultSubdivision.reset(new basegfx::B2DPolygon(basegfx::tools::adaptiveSubdivideByCount(rSource, 9)));
        }

        return *mpDefaultSubdivision;
    }

    const basegfx::B2DRange& getB2DRange(const basegfx::B2DPolygon& rSource) const
    {
        if(!mpB2DRange)
        {
            basegfx::B2DRange aNewRange;
            const sal_uInt32 nPointCount(rSource.count());

            if(nPointCount)
            {
                for(sal_uInt32 a(0); a < nPointCount; a++)
                {
                    aNewRange.expand(rSource.getB2DPoint(a));
                }

                if(rSource.areControlPointsUsed())
                {
                    const sal_uInt32 nEdgeCount(rSource.isClosed() ? nPointCount : nPointCount - 1);

                    if(nEdgeCount)
                    {
                        basegfx::B2DCubicBezier aEdge;
                        aEdge.setStartPoint(rSource.getB2DPoint(0));

                        for(sal_uInt32 b(0); b < nEdgeCount; b++)
                        {
                            const sal_uInt32 nNextIndex((b + 1) % nPointCount);
                            aEdge.setControlPointA(rSource.getNextControlPoint(b));
                            aEdge.setControlPointB(rSource.getPrevControlPoint(nNextIndex));
                            aEdge.setEndPoint(rSource.getB2DPoint(nNextIndex));

                            if(aEdge.isBezier())
                            {
                                const basegfx::B2DRange aBezierRangeWithControlPoints(aEdge.getRange());

                                if(!aNewRange.isInside(aBezierRangeWithControlPoints))
                                {
                                    // the range with control points of the current edge is not completely
                                    // inside the current range without control points. Expand current range by
                                    // subdividing the bezier segment.
                                    // Ideal here is a subdivision at the extreme values, so use
                                    // getAllExtremumPositions to get all extremas in one run
                                    ::std::vector< double > aExtremas;

                                    aExtremas.reserve(4);
                                    aEdge.getAllExtremumPositions(aExtremas);

                                    const sal_uInt32 nExtremaCount(aExtremas.size());

                                    for(sal_uInt32 c(0); c < nExtremaCount; c++)
                                    {
                                        aNewRange.expand(aEdge.interpolatePoint(aExtremas[c]));
                                    }
                                }
                            }

                            // prepare next edge
                            aEdge.setStartPoint(aEdge.getEndPoint());
                        }
                    }
                }
            }

            const_cast< ImplBufferedData* >(this)->mpB2DRange.reset(new basegfx::B2DRange(aNewRange));
        }

        return *mpB2DRange;
    }
};

//////////////////////////////////////////////////////////////////////////////

class ImplB2DPolygon
{
private:
    // The point vector. This vector exists always and defines the
    // count of members.
    CoordinateDataArray2D                           maPoints;

    // The control point vectors. This vectors are created on demand
    // and may be zero.
    boost::scoped_ptr< ControlVectorArray2D >       mpControlVector;

    // buffered data for e.g. default subdivision and range
    boost::scoped_ptr< ImplBufferedData >           mpBufferedData;

    // flag which decides if this polygon is opened or closed
    bool                                            mbIsClosed;

public:
    const basegfx::B2DPolygon& getDefaultAdaptiveSubdivision(const basegfx::B2DPolygon& rSource) const
    {
        if(!mpControlVector || !mpControlVector->isUsed())
        {
            return rSource;
        }

        if(!mpBufferedData)
        {
            const_cast< ImplB2DPolygon* >(this)->mpBufferedData.reset(new ImplBufferedData);
        }

        return mpBufferedData->getDefaultAdaptiveSubdivision(rSource);
    }

    const basegfx::B2DRange& getB2DRange(const basegfx::B2DPolygon& rSource) const
    {
        if(!mpBufferedData)
        {
            const_cast< ImplB2DPolygon* >(this)->mpBufferedData.reset(new ImplBufferedData);
        }

        return mpBufferedData->getB2DRange(rSource);
    }

    ImplB2DPolygon()
    :   maPoints(0),
        mpControlVector(),
        mpBufferedData(),
        mbIsClosed(false)
    {}

    ImplB2DPolygon(const ImplB2DPolygon& rToBeCopied)
    :   maPoints(rToBeCopied.maPoints),
        mpControlVector(),
        mpBufferedData(),
        mbIsClosed(rToBeCopied.mbIsClosed)
    {
        // complete initialization using copy
        if(rToBeCopied.mpControlVector && rToBeCopied.mpControlVector->isUsed())
        {
            mpControlVector.reset( new ControlVectorArray2D(*rToBeCopied.mpControlVector) );
        }
    }

    ImplB2DPolygon(const ImplB2DPolygon& rToBeCopied, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   maPoints(rToBeCopied.maPoints, nIndex, nCount),
        mpControlVector(),
        mpBufferedData(),
        mbIsClosed(rToBeCopied.mbIsClosed)
    {
        // complete initialization using partly copy
        if(rToBeCopied.mpControlVector && rToBeCopied.mpControlVector->isUsed())
        {
            mpControlVector.reset( new ControlVectorArray2D(*rToBeCopied.mpControlVector, nIndex, nCount) );

            if(!mpControlVector->isUsed())
                mpControlVector.reset();
        }
    }

    ImplB2DPolygon& operator=( const ImplB2DPolygon& rToBeCopied )
    {
        maPoints = rToBeCopied.maPoints;
        mpControlVector.reset();
        mpBufferedData.reset();
        mbIsClosed = rToBeCopied.mbIsClosed;

        // complete initialization using copy
        if(rToBeCopied.mpControlVector && rToBeCopied.mpControlVector->isUsed())
            mpControlVector.reset( new ControlVectorArray2D(*rToBeCopied.mpControlVector) );

        return *this;
    }

    sal_uInt32 count() const
    {
        return maPoints.count();
    }

    bool isClosed() const
    {
        return mbIsClosed;
    }

    void setClosed(bool bNew)
    {
        if(bNew != mbIsClosed)
        {
            mpBufferedData.reset();
            mbIsClosed = bNew;
        }
    }

    bool operator==(const ImplB2DPolygon& rCandidate) const
    {
        if(mbIsClosed == rCandidate.mbIsClosed)
        {
            if(maPoints == rCandidate.maPoints)
            {
                bool bControlVectorsAreEqual(true);

                if(mpControlVector)
                {
                    if(rCandidate.mpControlVector)
                    {
                        bControlVectorsAreEqual = ((*mpControlVector) == (*rCandidate.mpControlVector));
                    }
                    else
                    {
                        // candidate has no control vector, so it's assumed all unused.
                        bControlVectorsAreEqual = !mpControlVector->isUsed();
                    }
                }
                else
                {
                    if(rCandidate.mpControlVector)
                    {
                        // we have no control vector, so it's assumed all unused.
                        bControlVectorsAreEqual = !rCandidate.mpControlVector->isUsed();
                    }
                }

                if(bControlVectorsAreEqual)
                {
                    return true;
                }
            }
        }

        return false;
    }

    const basegfx::B2DPoint& getPoint(sal_uInt32 nIndex) const
    {
        return maPoints.getCoordinate(nIndex);
    }

    void setPoint(sal_uInt32 nIndex, const basegfx::B2DPoint& rValue)
    {
        mpBufferedData.reset();
        maPoints.setCoordinate(nIndex, rValue);
    }

    void reserve(sal_uInt32 nCount)
    {
        maPoints.reserve(nCount);
    }

    void append(const basegfx::B2DPoint& rPoint)
    {
        mpBufferedData.reset(); // TODO: is this needed?
        const CoordinateData2D aCoordinate(rPoint);
        maPoints.append(aCoordinate);

        if(mpControlVector)
        {
            const ControlVectorPair2D aVectorPair;
            mpControlVector->append(aVectorPair);
        }
    }

    void insert(sal_uInt32 nIndex, const basegfx::B2DPoint& rPoint, sal_uInt32 nCount)
    {
        if(nCount)
        {
            mpBufferedData.reset();
            CoordinateData2D aCoordinate(rPoint);
            maPoints.insert(nIndex, aCoordinate, nCount);

            if(mpControlVector)
            {
                ControlVectorPair2D aVectorPair;
                mpControlVector->insert(nIndex, aVectorPair, nCount);
            }
        }
    }

    const basegfx::B2DVector& getPrevControlVector(sal_uInt32 nIndex) const
    {
        if(mpControlVector)
        {
            return mpControlVector->getPrevVector(nIndex);
        }
        else
        {
            return basegfx::B2DVector::getEmptyVector();
        }
    }

    void setPrevControlVector(sal_uInt32 nIndex, const basegfx::B2DVector& rValue)
    {
        if(!mpControlVector)
        {
            if(!rValue.equalZero())
            {
                mpBufferedData.reset();
                mpControlVector.reset( new ControlVectorArray2D(maPoints.count()) );
                mpControlVector->setPrevVector(nIndex, rValue);
            }
        }
        else
        {
            mpBufferedData.reset();
            mpControlVector->setPrevVector(nIndex, rValue);

            if(!mpControlVector->isUsed())
                mpControlVector.reset();
        }
    }

    const basegfx::B2DVector& getNextControlVector(sal_uInt32 nIndex) const
    {
        if(mpControlVector)
        {
            return mpControlVector->getNextVector(nIndex);
        }
        else
        {
            return basegfx::B2DVector::getEmptyVector();
        }
    }

    void setNextControlVector(sal_uInt32 nIndex, const basegfx::B2DVector& rValue)
    {
        if(!mpControlVector)
        {
            if(!rValue.equalZero())
            {
                mpBufferedData.reset();
                mpControlVector.reset( new ControlVectorArray2D(maPoints.count()) );
                mpControlVector->setNextVector(nIndex, rValue);
            }
        }
        else
        {
            mpBufferedData.reset();
            mpControlVector->setNextVector(nIndex, rValue);

            if(!mpControlVector->isUsed())
                mpControlVector.reset();
        }
    }

    bool areControlPointsUsed() const
    {
        return (mpControlVector && mpControlVector->isUsed());
    }

    void resetControlVectors(sal_uInt32 nIndex)
    {
        setPrevControlVector(nIndex, basegfx::B2DVector::getEmptyVector());
        setNextControlVector(nIndex, basegfx::B2DVector::getEmptyVector());
    }

    void resetControlVectors()
    {
        mpBufferedData.reset();
        mpControlVector.reset();
    }

    void setControlVectors(sal_uInt32 nIndex, const basegfx::B2DVector& rPrev, const basegfx::B2DVector& rNext)
    {
        setPrevControlVector(nIndex, rPrev);
        setNextControlVector(nIndex, rNext);
    }

    void appendBezierSegment(const basegfx::B2DVector& rNext, const basegfx::B2DVector& rPrev, const basegfx::B2DPoint& rPoint)
    {
        mpBufferedData.reset();
        const sal_uInt32 nCount(maPoints.count());

        if(nCount)
        {
            setNextControlVector(nCount - 1, rNext);
        }

        insert(nCount, rPoint, 1);
        setPrevControlVector(nCount, rPrev);
    }

    void insert(sal_uInt32 nIndex, const ImplB2DPolygon& rSource)
    {
        const sal_uInt32 nCount(rSource.maPoints.count());

        if(nCount)
        {
            mpBufferedData.reset();

            if(rSource.mpControlVector && rSource.mpControlVector->isUsed() && !mpControlVector)
            {
                mpControlVector.reset( new ControlVectorArray2D(maPoints.count()) );
            }

            maPoints.insert(nIndex, rSource.maPoints);

            if(rSource.mpControlVector)
            {
                mpControlVector->insert(nIndex, *rSource.mpControlVector);

                if(!mpControlVector->isUsed())
                    mpControlVector.reset();
            }
            else if(mpControlVector)
            {
                ControlVectorPair2D aVectorPair;
                mpControlVector->insert(nIndex, aVectorPair, nCount);
            }
        }
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(nCount)
        {
            mpBufferedData.reset();
            maPoints.remove(nIndex, nCount);

            if(mpControlVector)
            {
                mpControlVector->remove(nIndex, nCount);

                if(!mpControlVector->isUsed())
                    mpControlVector.reset();
            }
        }
    }

    void flip()
    {
        if(maPoints.count() > 1)
        {
            mpBufferedData.reset();

            // flip points
            maPoints.flip(mbIsClosed);

            if(mpControlVector)
            {
                // flip control vector
                mpControlVector->flip(mbIsClosed);
            }
        }
    }

    bool hasDoublePoints() const
    {
        if(mbIsClosed)
        {
            // check for same start and end point
            const sal_uInt32 nIndex(maPoints.count() - 1);

            if(maPoints.getCoordinate(0) == maPoints.getCoordinate(nIndex))
            {
                if(mpControlVector)
                {
                    if(mpControlVector->getNextVector(nIndex).equalZero() && mpControlVector->getPrevVector(0).equalZero())
                    {
                        return true;
                    }
                }
                else
                {
                    return true;
                }
            }
        }

        // test for range
        for(sal_uInt32 a(0); a < maPoints.count() - 1; a++)
        {
            if(maPoints.getCoordinate(a) == maPoints.getCoordinate(a + 1))
            {
                if(mpControlVector)
                {
                    if(mpControlVector->getNextVector(a).equalZero() && mpControlVector->getPrevVector(a + 1).equalZero())
                    {
                        return true;
                    }
                }
                else
                {
                    return true;
                }
            }
        }

        return false;
    }

    void removeDoublePointsAtBeginEnd()
    {
        // Only remove DoublePoints at Begin and End when poly is closed
        if(mbIsClosed)
        {
            mpBufferedData.reset();

            if(mpControlVector)
            {
                bool bRemove;

                do
                {
                    bRemove = false;

                    if(maPoints.count() > 1)
                    {
                        const sal_uInt32 nIndex(maPoints.count() - 1);

                        if(maPoints.getCoordinate(0) == maPoints.getCoordinate(nIndex))
                        {
                            if(mpControlVector)
                            {
                                if(mpControlVector->getNextVector(nIndex).equalZero() && mpControlVector->getPrevVector(0).equalZero())
                                {
                                    bRemove = true;
                                }
                            }
                            else
                            {
                                bRemove = true;
                            }
                        }
                    }

                    if(bRemove)
                    {
                        const sal_uInt32 nIndex(maPoints.count() - 1);

                        if(mpControlVector && !mpControlVector->getPrevVector(nIndex).equalZero())
                        {
                            mpControlVector->setPrevVector(0, mpControlVector->getPrevVector(nIndex));
                        }

                        remove(nIndex, 1);
                    }
                }
                while(bRemove);
            }
            else
            {
                maPoints.removeDoublePointsAtBeginEnd();
            }
        }
    }

    void removeDoublePointsWholeTrack()
    {
        mpBufferedData.reset();

        if(mpControlVector)
        {
            sal_uInt32 nIndex(0);

            // test as long as there are at least two points and as long as the index
            // is smaller or equal second last point
            while((maPoints.count() > 1) && (nIndex <= maPoints.count() - 2))
            {
                bool bRemove(maPoints.getCoordinate(nIndex) == maPoints.getCoordinate(nIndex + 1));

                if(bRemove)
                {
                    if(mpControlVector)
                    {
                        if(!mpControlVector->getNextVector(nIndex).equalZero() || !mpControlVector->getPrevVector(nIndex + 1).equalZero())
                        {
                            bRemove = false;
                        }
                    }
                }

                if(bRemove)
                {
                    if(mpControlVector && !mpControlVector->getPrevVector(nIndex).equalZero())
                    {
                        mpControlVector->setPrevVector(nIndex + 1, mpControlVector->getPrevVector(nIndex));
                    }

                    // if next is same as index and the control vectors are unused, delete index
                    remove(nIndex, 1);
                }
                else
                {
                    // if different, step forward
                    nIndex++;
                }
            }
        }
        else
        {
            maPoints.removeDoublePointsWholeTrack();
        }
    }

    void transform(const basegfx::B2DHomMatrix& rMatrix)
    {
        mpBufferedData.reset();

        if(mpControlVector)
        {
            for(sal_uInt32 a(0); a < maPoints.count(); a++)
            {
                basegfx::B2DPoint aCandidate = maPoints.getCoordinate(a);

                if(mpControlVector->isUsed())
                {
                    const basegfx::B2DVector& rPrevVector(mpControlVector->getPrevVector(a));
                    const basegfx::B2DVector& rNextVector(mpControlVector->getNextVector(a));

                    if(!rPrevVector.equalZero())
                    {
                        basegfx::B2DVector aPrevVector(rMatrix * rPrevVector);
                        mpControlVector->setPrevVector(a, aPrevVector);
                    }

                    if(!rNextVector.equalZero())
                    {
                        basegfx::B2DVector aNextVector(rMatrix * rNextVector);
                        mpControlVector->setNextVector(a, aNextVector);
                    }
                }

                aCandidate *= rMatrix;
                maPoints.setCoordinate(a, aCandidate);
            }

            if(!mpControlVector->isUsed())
                mpControlVector.reset();
        }
        else
        {
            maPoints.transform(rMatrix);
        }
    }

    const basegfx::B2DPoint* begin() const
    {
        return maPoints.begin();
    }

    const basegfx::B2DPoint* end() const
    {
        return maPoints.end();
    }

    basegfx::B2DPoint* begin()
    {
       mpBufferedData.reset();
       return maPoints.begin();
    }

    basegfx::B2DPoint* end()
    {
        mpBufferedData.reset();
        return maPoints.end();
    }
};

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace
    {
        struct DefaultPolygon: public rtl::Static<B2DPolygon::ImplType, DefaultPolygon> {};
    }

    B2DPolygon::B2DPolygon()
    :   mpPolygon(DefaultPolygon::get())
    {}

    B2DPolygon::B2DPolygon(const B2DPolygon& rPolygon)
    :   mpPolygon(rPolygon.mpPolygon)
    {}

    B2DPolygon::B2DPolygon(const B2DPolygon& rPolygon, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   mpPolygon(ImplB2DPolygon(*rPolygon.mpPolygon, nIndex, nCount))
    {
        // TODO(P2): one extra temporary here (cow_wrapper copies
        // given ImplB2DPolygon into its internal impl_t wrapper type)
        OSL_ENSURE(nIndex + nCount <= rPolygon.mpPolygon->count(), "B2DPolygon constructor outside range (!)");
    }

    B2DPolygon::~B2DPolygon()
    {
    }

    B2DPolygon& B2DPolygon::operator=(const B2DPolygon& rPolygon)
    {
        mpPolygon = rPolygon.mpPolygon;
        return *this;
    }

    void B2DPolygon::makeUnique()
    {
        mpPolygon.make_unique();
    }

    bool B2DPolygon::operator==(const B2DPolygon& rPolygon) const
    {
        if(mpPolygon.same_object(rPolygon.mpPolygon))
            return true;

        return ((*mpPolygon) == (*rPolygon.mpPolygon));
    }

    bool B2DPolygon::operator!=(const B2DPolygon& rPolygon) const
    {
        return !(*this == rPolygon);
    }

    sal_uInt32 B2DPolygon::count() const
    {
        return mpPolygon->count();
    }

    B2DPoint B2DPolygon::getB2DPoint(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        return mpPolygon->getPoint(nIndex);
    }

    void B2DPolygon::setB2DPoint(sal_uInt32 nIndex, const B2DPoint& rValue)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        if(getB2DPoint(nIndex) != rValue)
        {
            mpPolygon->setPoint(nIndex, rValue);
        }
    }

    void B2DPolygon::reserve(sal_uInt32 nCount)
    {
        mpPolygon->reserve(nCount);
    }

    void B2DPolygon::insert(sal_uInt32 nIndex, const B2DPoint& rPoint, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex <= mpPolygon->count(), "B2DPolygon Insert outside range (!)");

        if(nCount)
        {
            mpPolygon->insert(nIndex, rPoint, nCount);
        }
    }

    void B2DPolygon::append(const B2DPoint& rPoint, sal_uInt32 nCount)
    {
        if(nCount)
        {
            mpPolygon->insert(mpPolygon->count(), rPoint, nCount);
        }
    }

    void B2DPolygon::append(const B2DPoint& rPoint)
    {
        mpPolygon->append(rPoint);
    }

    B2DPoint B2DPolygon::getPrevControlPoint(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        if(mpPolygon->areControlPointsUsed())
        {
            return mpPolygon->getPoint(nIndex) + mpPolygon->getPrevControlVector(nIndex);
        }
        else
        {
            return mpPolygon->getPoint(nIndex);
        }
    }

    B2DPoint B2DPolygon::getNextControlPoint(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        if(mpPolygon->areControlPointsUsed())
        {
            return mpPolygon->getPoint(nIndex) + mpPolygon->getNextControlVector(nIndex);
        }
        else
        {
            return mpPolygon->getPoint(nIndex);
        }
    }

    void B2DPolygon::setPrevControlPoint(sal_uInt32 nIndex, const B2DPoint& rValue)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");
        const basegfx::B2DVector aNewVector(rValue - mpPolygon->getPoint(nIndex));

        if(mpPolygon->getPrevControlVector(nIndex) != aNewVector)
        {
            mpPolygon->setPrevControlVector(nIndex, aNewVector);
        }
    }

    void B2DPolygon::setNextControlPoint(sal_uInt32 nIndex, const B2DPoint& rValue)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");
        const basegfx::B2DVector aNewVector(rValue - mpPolygon->getPoint(nIndex));

        if(mpPolygon->getNextControlVector(nIndex) != aNewVector)
        {
            mpPolygon->setNextControlVector(nIndex, aNewVector);
        }
    }

    void B2DPolygon::setControlPoints(sal_uInt32 nIndex, const basegfx::B2DPoint& rPrev, const basegfx::B2DPoint& rNext)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");
        const B2DPoint aPoint(mpPolygon->getPoint(nIndex));
        const basegfx::B2DVector aNewPrev(rPrev - aPoint);
        const basegfx::B2DVector aNewNext(rNext - aPoint);

        if(mpPolygon->getPrevControlVector(nIndex) != aNewPrev || mpPolygon->getNextControlVector(nIndex) != aNewNext)
        {
            mpPolygon->setControlVectors(nIndex, aNewPrev, aNewNext);
        }
    }

    void B2DPolygon::resetPrevControlPoint(sal_uInt32 nIndex)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        if(mpPolygon->areControlPointsUsed() && !mpPolygon->getPrevControlVector(nIndex).equalZero())
        {
            mpPolygon->setPrevControlVector(nIndex, B2DVector::getEmptyVector());
        }
    }

    void B2DPolygon::resetNextControlPoint(sal_uInt32 nIndex)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        if(mpPolygon->areControlPointsUsed() && !mpPolygon->getNextControlVector(nIndex).equalZero())
        {
            mpPolygon->setNextControlVector(nIndex, B2DVector::getEmptyVector());
        }
    }

    void B2DPolygon::resetControlPoints()
    {
        if(mpPolygon->areControlPointsUsed())
        {
            mpPolygon->resetControlVectors();
        }
    }

    void B2DPolygon::appendBezierSegment(
        const B2DPoint& rNextControlPoint,
        const B2DPoint& rPrevControlPoint,
        const B2DPoint& rPoint)
    {
        const B2DVector aNewNextVector(mpPolygon->count() ? B2DVector(rNextControlPoint - mpPolygon->getPoint(mpPolygon->count() - 1)) : B2DVector::getEmptyVector());
        const B2DVector aNewPrevVector(rPrevControlPoint - rPoint);

        if(aNewNextVector.equalZero() && aNewPrevVector.equalZero())
        {
            mpPolygon->insert(mpPolygon->count(), rPoint, 1);
        }
        else
        {
            mpPolygon->appendBezierSegment(aNewNextVector, aNewPrevVector, rPoint);
        }
    }

    bool B2DPolygon::areControlPointsUsed() const
    {
        return mpPolygon->areControlPointsUsed();
    }

    bool B2DPolygon::isPrevControlPointUsed(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        return (mpPolygon->areControlPointsUsed() && !mpPolygon->getPrevControlVector(nIndex).equalZero());
    }

    bool B2DPolygon::isNextControlPointUsed(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        return (mpPolygon->areControlPointsUsed() && !mpPolygon->getNextControlVector(nIndex).equalZero());
    }

    B2VectorContinuity B2DPolygon::getContinuityInPoint(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        if(mpPolygon->areControlPointsUsed())
        {
            const B2DVector& rPrev(mpPolygon->getPrevControlVector(nIndex));
            const B2DVector& rNext(mpPolygon->getNextControlVector(nIndex));

            return getContinuity(rPrev, rNext);
        }
        else
        {
            return CONTINUITY_NONE;
        }
    }

    void B2DPolygon::getBezierSegment(sal_uInt32 nIndex, B2DCubicBezier& rTarget) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");
        const bool bNextIndexValidWithoutClose(nIndex + 1 < mpPolygon->count());

        if(bNextIndexValidWithoutClose || mpPolygon->isClosed())
        {
            const sal_uInt32 nNextIndex(bNextIndexValidWithoutClose ? nIndex + 1 : 0);
            rTarget.setStartPoint(mpPolygon->getPoint(nIndex));
            rTarget.setEndPoint(mpPolygon->getPoint(nNextIndex));

            if(mpPolygon->areControlPointsUsed())
            {
                rTarget.setControlPointA(rTarget.getStartPoint() + mpPolygon->getNextControlVector(nIndex));
                rTarget.setControlPointB(rTarget.getEndPoint() + mpPolygon->getPrevControlVector(nNextIndex));
            }
            else
            {
                // no bezier, reset control poins at rTarget
                rTarget.setControlPointA(rTarget.getStartPoint());
                rTarget.setControlPointB(rTarget.getEndPoint());
            }
        }
        else
        {
            // no valid edge at all, reset rTarget to current point
            const B2DPoint aPoint(mpPolygon->getPoint(nIndex));
            rTarget.setStartPoint(aPoint);
            rTarget.setEndPoint(aPoint);
            rTarget.setControlPointA(aPoint);
            rTarget.setControlPointB(aPoint);
        }
    }

    B2DPolygon B2DPolygon::getDefaultAdaptiveSubdivision() const
    {
        return mpPolygon->getDefaultAdaptiveSubdivision(*this);
    }

    B2DRange B2DPolygon::getB2DRange() const
    {
        return mpPolygon->getB2DRange(*this);
    }

    void B2DPolygon::append(const B2DPolygon& rPoly, sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(rPoly.count())
        {
            if(!nCount)
            {
                nCount = rPoly.count();
            }

            if(0 == nIndex && nCount == rPoly.count())
            {
                mpPolygon->insert(mpPolygon->count(), *rPoly.mpPolygon);
            }
            else
            {
                OSL_ENSURE(nIndex + nCount <= rPoly.mpPolygon->count(), "B2DPolygon Append outside range (!)");
                ImplB2DPolygon aTempPoly(*rPoly.mpPolygon, nIndex, nCount);
                mpPolygon->insert(mpPolygon->count(), aTempPoly);
            }
        }
    }

    void B2DPolygon::remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex + nCount <= mpPolygon->count(), "B2DPolygon Remove outside range (!)");

        if(nCount)
        {
            mpPolygon->remove(nIndex, nCount);
        }
    }

    void B2DPolygon::clear()
    {
        mpPolygon = DefaultPolygon::get();
    }

    bool B2DPolygon::isClosed() const
    {
        return mpPolygon->isClosed();
    }

    void B2DPolygon::setClosed(bool bNew)
    {
        if(isClosed() != bNew)
        {
            mpPolygon->setClosed(bNew);
        }
    }

    void B2DPolygon::flip()
    {
        if(count() > 1)
        {
            mpPolygon->flip();
        }
    }

    bool B2DPolygon::hasDoublePoints() const
    {
        return (mpPolygon->count() > 1 && mpPolygon->hasDoublePoints());
    }

    void B2DPolygon::removeDoublePoints()
    {
        if(hasDoublePoints())
        {
            mpPolygon->removeDoublePointsAtBeginEnd();
            mpPolygon->removeDoublePointsWholeTrack();
        }
    }

    void B2DPolygon::transform(const B2DHomMatrix& rMatrix)
    {
        if(mpPolygon->count() && !rMatrix.isIdentity())
        {
            mpPolygon->transform(rMatrix);
        }
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
