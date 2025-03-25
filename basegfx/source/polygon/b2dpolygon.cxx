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

#include <sal/config.h>

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/utils/systemdependentdata.hxx>
#include <memory>
#include <vector>

namespace
{

class CoordinateDataArray2D
{
private:
    std::vector<basegfx::B2DPoint> maVector;

public:
    explicit CoordinateDataArray2D(sal_uInt32 nCount)
    :   maVector(nCount)
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
        assert(nIndex < maVector.size());
        return maVector[nIndex];
    }

    void setCoordinate(sal_uInt32 nIndex, const basegfx::B2DPoint& rValue)
    {
        assert(nIndex < maVector.size());
        maVector[nIndex] = rValue;
    }

    void reserve(sal_uInt32 nCount)
    {
        maVector.reserve(nCount);
    }

    void append(const basegfx::B2DPoint& rValue)
    {
        maVector.push_back(rValue);
    }

    void insert(sal_uInt32 nIndex, const basegfx::B2DPoint& rValue, sal_uInt32 nCount)
    {
        assert(nCount > 0);
        assert(nIndex <= maVector.size());
        // add nCount copies of rValue
        maVector.insert(maVector.begin() + nIndex, nCount, rValue);
    }

    void insert(sal_uInt32 nIndex, const CoordinateDataArray2D& rSource)
    {
        assert(rSource.maVector.size() > 0);
        assert(nIndex <= maVector.size());
        // insert data
        auto aIndex = maVector.begin();
        aIndex += nIndex;
        auto aStart = rSource.maVector.cbegin();
        auto aEnd = rSource.maVector.cend();
        maVector.insert(aIndex, aStart, aEnd);
    }

    void insert(sal_uInt32 nIndex, const CoordinateDataArray2D& rSource, sal_uInt32 nSourceIndex, sal_uInt32 nSourceCount)
    {
        // insert data
        auto aIndex = maVector.begin();
        aIndex += nIndex;
        auto aStart = rSource.maVector.cbegin() + nSourceIndex;
        auto aEnd = rSource.maVector.cbegin() + nSourceIndex + nSourceCount;
        maVector.insert(aIndex, aStart, aEnd);
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        assert(nCount > 0);
        assert(nIndex + nCount <= maVector.size());
        // remove point data
        const auto aStart = maVector.begin() + nIndex;
        const auto aEnd = aStart + nCount;
        maVector.erase(aStart, aEnd);
    }

    void flip(bool bIsClosed)
    {
        assert(maVector.size() > 1);

        // to keep the same point at index 0, just flip all points except the
        // first one when closed
        const sal_uInt32 nHalfSize(bIsClosed ? (maVector.size() - 1) >> 1 : maVector.size() >> 1);
        auto aStart = bIsClosed ? maVector.begin() + 1 : maVector.begin();
        auto aEnd = maVector.end() - 1;

        for(sal_uInt32 a(0); a < nHalfSize; a++)
        {
            std::swap(*aStart, *aEnd);
            ++aStart;
            --aEnd;
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
        for (auto& point : maVector)
        {
            point *= rMatrix;
        }
    }

    void translate(double fTranslateX, double fTranslateY)
    {
        for (auto& point : maVector)
        {
            point += basegfx::B2DPoint(fTranslateX, fTranslateY);
        }
    }
};

class ControlVectorPair2D
{
    basegfx::B2DVector                          maPrevVector;
    basegfx::B2DVector                          maNextVector;

public:
    explicit ControlVectorPair2D() {}

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
        std::swap(maPrevVector, maNextVector);
    }
};

class ControlVectorArray2D
{
    typedef std::vector< ControlVectorPair2D > ControlVectorPair2DVector;

    ControlVectorPair2DVector                           maVector;
    sal_uInt32                                          mnUsedVectors;

public:
    explicit ControlVectorArray2D(sal_uInt32 nCount)
    :   maVector(nCount),
        mnUsedVectors(0)
    {}

    ControlVectorArray2D(const ControlVectorArray2D& rOriginal, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   mnUsedVectors(0)
    {
        assert(nIndex + nCount <= rOriginal.maVector.size());
        auto aStart(rOriginal.maVector.begin() + nIndex);
        auto aEnd(aStart + nCount);
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

    bool operator==(const ControlVectorArray2D& rCandidate) const
    {
        return (maVector == rCandidate.maVector);
    }

    bool isUsed() const
    {
        return (mnUsedVectors != 0);
    }

    const basegfx::B2DVector& getPrevVector(sal_uInt32 nIndex) const
    {
        assert(nIndex < maVector.size());
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
        assert(nIndex < maVector.size());
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
        assert(nCount > 0);
        assert(nIndex <= maVector.size());

        // add nCount copies of rValue
        maVector.insert(maVector.begin() + nIndex, nCount, rValue);

        if(!rValue.getPrevVector().equalZero())
            mnUsedVectors += nCount;

        if(!rValue.getNextVector().equalZero())
            mnUsedVectors += nCount;
    }

    void insert(sal_uInt32 nIndex, const ControlVectorArray2D& rSource)
    {
        assert(rSource.maVector.size() > 0);
        assert(nIndex <= maVector.size());

        // insert data
        ControlVectorPair2DVector::iterator aIndex(maVector.begin() + nIndex);
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

    void insert(sal_uInt32 nIndex, const ControlVectorArray2D& rSource, sal_uInt32 nSourceIndex, sal_uInt32 nSourceCount)
    {
        assert(rSource.maVector.size() > 0);
        assert(nIndex + nSourceCount <= maVector.size());

        // insert data
        ControlVectorPair2DVector::iterator aIndex(maVector.begin() + nIndex);
        ControlVectorPair2DVector::const_iterator aStart(rSource.maVector.begin() + nSourceIndex);
        ControlVectorPair2DVector::const_iterator aEnd(rSource.maVector.begin() + nSourceIndex + nSourceCount);
        maVector.insert(aIndex, aStart, aEnd);

        for(; aStart != aEnd; ++aStart)
        {
            if(!aStart->getPrevVector().equalZero())
                mnUsedVectors++;

            if(!aStart->getNextVector().equalZero())
                mnUsedVectors++;
        }
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        assert(nCount > 0);
        assert(nIndex + nCount <= maVector.size());

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

    void flip(bool bIsClosed)
    {
        assert(maVector.size() > 1);

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
            std::swap(*aStart, *aEnd);

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
};

class ImplBufferedData : public basegfx::SystemDependentDataHolder
{
private:
    // Possibility to hold the last subdivision
    mutable std::optional< basegfx::B2DPolygon >  mpDefaultSubdivision;

    // Possibility to hold the last B2DRange calculation
    mutable std::optional< basegfx::B2DRange > moB2DRange;

public:
    ImplBufferedData()
    {
    }

    const basegfx::B2DPolygon& getDefaultAdaptiveSubdivision(const basegfx::B2DPolygon& rSource) const
    {
        if(!mpDefaultSubdivision)
        {
            mpDefaultSubdivision = basegfx::utils::adaptiveSubdivideByAngle(rSource);
        }

        return *mpDefaultSubdivision;
    }

    const basegfx::B2DRange& getB2DRange(const basegfx::B2DPolygon& rSource) const
    {
        if(!moB2DRange)
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
                                    std::vector< double > aExtremas;

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

            moB2DRange = aNewRange;
        }

        return *moB2DRange;
    }
};

}

class ImplB2DPolygon
{
private:
    // The point vector. This vector exists always and defines the
    // count of members.
    CoordinateDataArray2D                         maPoints;

    // The control point vectors. This vectors are created on demand
    // and may be zero.
    std::optional< ControlVectorArray2D >         moControlVector;

    // buffered data for e.g. default subdivision and range
    // we do not want to 'modify' the ImplB2DPolygon,
    // but add buffered data that is valid for all referencing instances
    mutable std::unique_ptr<ImplBufferedData> mpBufferedData;

    // flag which decides if this polygon is opened or closed
    bool                                          mbIsClosed;

public:
    const basegfx::B2DPolygon& getDefaultAdaptiveSubdivision(const basegfx::B2DPolygon& rSource) const
    {
        if(!moControlVector || !moControlVector->isUsed())
        {
            return rSource;
        }

        if(!mpBufferedData)
        {
            mpBufferedData.reset(new ImplBufferedData);
        }

        return mpBufferedData->getDefaultAdaptiveSubdivision(rSource);
    }

    const basegfx::B2DRange& getB2DRange(const basegfx::B2DPolygon& rSource) const
    {
        if(!mpBufferedData)
        {
            mpBufferedData.reset(new ImplBufferedData);
        }

        return mpBufferedData->getB2DRange(rSource);
    }

    ImplB2DPolygon()
    :   maPoints(0),
        mbIsClosed(false)
    {}

    ImplB2DPolygon(const ImplB2DPolygon& rToBeCopied)
    :   maPoints(rToBeCopied.maPoints),
        mbIsClosed(rToBeCopied.mbIsClosed)
    {
        // complete initialization using copy
        if(rToBeCopied.moControlVector && rToBeCopied.moControlVector->isUsed())
        {
            moControlVector.emplace( *rToBeCopied.moControlVector );
        }
    }

    ImplB2DPolygon(ImplB2DPolygon&&) = default;

    ImplB2DPolygon(const ImplB2DPolygon& rToBeCopied, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   maPoints(rToBeCopied.maPoints, nIndex, nCount),
        mbIsClosed(rToBeCopied.mbIsClosed)
    {
        // complete initialization using partly copy
        if(rToBeCopied.moControlVector && rToBeCopied.moControlVector->isUsed())
        {
            moControlVector.emplace( *rToBeCopied.moControlVector, nIndex, nCount );

            if(!moControlVector->isUsed())
                moControlVector.reset();
        }
    }


    ImplB2DPolygon& operator=(ImplB2DPolygon&&) = default;

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
        if(mbIsClosed != rCandidate.mbIsClosed)
            return false;
        if(!(maPoints == rCandidate.maPoints))
            return false;
        bool bControlVectorsAreEqual(true);

        if(moControlVector)
        {
            if(rCandidate.moControlVector)
            {
                bControlVectorsAreEqual = ((*moControlVector) == (*rCandidate.moControlVector));
            }
            else
            {
                // candidate has no control vector, so it's assumed all unused.
                bControlVectorsAreEqual = !moControlVector->isUsed();
            }
        }
        else
        {
            if(rCandidate.moControlVector)
            {
                // we have no control vector, so it's assumed all unused.
                bControlVectorsAreEqual = !rCandidate.moControlVector->isUsed();
            }
        }

        return bControlVectorsAreEqual;
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
        maPoints.append(rPoint);

        if(moControlVector)
        {
            const ControlVectorPair2D aVectorPair;
            moControlVector->append(aVectorPair);
        }
    }

    void insert(sal_uInt32 nIndex, const basegfx::B2DPoint& rPoint, sal_uInt32 nCount)
    {
        assert(nCount > 0);
        mpBufferedData.reset();
        maPoints.insert(nIndex, rPoint, nCount);

        if(moControlVector)
        {
            ControlVectorPair2D aVectorPair;
            moControlVector->insert(nIndex, aVectorPair, nCount);
        }
    }

    void append(const basegfx::B2DPoint& rPoint, sal_uInt32 nCount)
    {
        insert(count(), rPoint, nCount);
    }

    const basegfx::B2DVector& getPrevControlVector(sal_uInt32 nIndex) const
    {
        if(moControlVector)
        {
            return moControlVector->getPrevVector(nIndex);
        }
        else
        {
            return basegfx::B2DVector::getEmptyVector();
        }
    }

    void setPrevControlVector(sal_uInt32 nIndex, const basegfx::B2DVector& rValue)
    {
        if(!moControlVector)
        {
            if(!rValue.equalZero())
            {
                mpBufferedData.reset();
                moControlVector.emplace(maPoints.count());
                moControlVector->setPrevVector(nIndex, rValue);
            }
        }
        else
        {
            mpBufferedData.reset();
            moControlVector->setPrevVector(nIndex, rValue);

            if(!moControlVector->isUsed())
                moControlVector.reset();
        }
    }

    const basegfx::B2DVector& getNextControlVector(sal_uInt32 nIndex) const
    {
        if(moControlVector)
        {
            return moControlVector->getNextVector(nIndex);
        }
        else
        {
            return basegfx::B2DVector::getEmptyVector();
        }
    }

    void setNextControlVector(sal_uInt32 nIndex, const basegfx::B2DVector& rValue)
    {
        if(!moControlVector)
        {
            if(!rValue.equalZero())
            {
                mpBufferedData.reset();
                moControlVector.emplace(maPoints.count());
                moControlVector->setNextVector(nIndex, rValue);
            }
        }
        else
        {
            mpBufferedData.reset();
            moControlVector->setNextVector(nIndex, rValue);

            if(!moControlVector->isUsed())
                moControlVector.reset();
        }
    }

    bool areControlPointsUsed() const
    {
        return (moControlVector && moControlVector->isUsed());
    }

    void resetControlVectors()
    {
        mpBufferedData.reset();
        moControlVector.reset();
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

    void append(const ImplB2DPolygon& rSource, sal_uInt32 nSourceIndex, sal_uInt32 nSourceCount)
    {
        assert(rSource.maPoints.count() > 0);
        const sal_uInt32 nIndex = count();

        mpBufferedData.reset();

        maPoints.insert(nIndex, rSource.maPoints, nSourceIndex, nSourceCount);

        if(rSource.moControlVector && rSource.moControlVector->isUsed())
        {
            if (!moControlVector)
                moControlVector.emplace(nIndex);
            moControlVector->insert(nIndex, *rSource.moControlVector, nSourceIndex, nSourceCount);

            if(!moControlVector->isUsed())
                moControlVector.reset();
        }
        else if(moControlVector)
        {
            ControlVectorPair2D aVectorPair;
            moControlVector->insert(nIndex, aVectorPair, rSource.count());
        }
    }

    void append(const ImplB2DPolygon& rSource)
    {
        assert(rSource.maPoints.count() > 0);
        const sal_uInt32 nIndex = count();

        mpBufferedData.reset();

        maPoints.insert(nIndex, rSource.maPoints);

        if(rSource.moControlVector)
        {
            if (!moControlVector)
                moControlVector.emplace(nIndex);
            moControlVector->insert(nIndex, *rSource.moControlVector);

            if(!moControlVector->isUsed())
                moControlVector.reset();
        }
        else if(moControlVector)
        {
            ControlVectorPair2D aVectorPair;
            moControlVector->insert(nIndex, aVectorPair, rSource.count());
        }
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        assert(nCount > 0);
        mpBufferedData.reset();
        maPoints.remove(nIndex, nCount);

        if(moControlVector)
        {
            moControlVector->remove(nIndex, nCount);

            if(!moControlVector->isUsed())
                moControlVector.reset();
        }
    }

    void flip()
    {
        assert(maPoints.count() > 1);

        mpBufferedData.reset();

        // flip points
        maPoints.flip(mbIsClosed);

        if(moControlVector)
        {
            // flip control vector
            moControlVector->flip(mbIsClosed);
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
                if(moControlVector)
                {
                    if(moControlVector->getNextVector(nIndex).equalZero() && moControlVector->getPrevVector(0).equalZero())
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
                if(moControlVector)
                {
                    if(moControlVector->getNextVector(a).equalZero() && moControlVector->getPrevVector(a + 1).equalZero())
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
        if(!mbIsClosed)
            return;

        mpBufferedData.reset();

        if(moControlVector)
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
                        if(moControlVector)
                        {
                            if(moControlVector->getNextVector(nIndex).equalZero() && moControlVector->getPrevVector(0).equalZero())
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

                    if(moControlVector && !moControlVector->getPrevVector(nIndex).equalZero())
                    {
                        moControlVector->setPrevVector(0, moControlVector->getPrevVector(nIndex));
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

    void removeDoublePointsWholeTrack()
    {
        mpBufferedData.reset();

        if(moControlVector)
        {
            sal_uInt32 nIndex(0);

            // test as long as there are at least two points and as long as the index
            // is smaller or equal second last point
            while((maPoints.count() > 1) && (nIndex <= maPoints.count() - 2))
            {
                bool bRemove(maPoints.getCoordinate(nIndex) == maPoints.getCoordinate(nIndex + 1));

                if(bRemove && moControlVector)
                {
                    if(!moControlVector->getNextVector(nIndex).equalZero() || !moControlVector->getPrevVector(nIndex + 1).equalZero())
                    {
                        bRemove = false;
                    }
                }

                if(bRemove)
                {
                    if(moControlVector && !moControlVector->getPrevVector(nIndex).equalZero())
                    {
                        moControlVector->setPrevVector(nIndex + 1, moControlVector->getPrevVector(nIndex));
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

        if(moControlVector)
        {
            for(sal_uInt32 a(0); a < maPoints.count(); a++)
            {
                basegfx::B2DPoint aCandidate = maPoints.getCoordinate(a);

                if(moControlVector->isUsed())
                {
                    const basegfx::B2DVector& rPrevVector(moControlVector->getPrevVector(a));
                    const basegfx::B2DVector& rNextVector(moControlVector->getNextVector(a));

                    if(!rPrevVector.equalZero())
                    {
                        basegfx::B2DVector aPrevVector(rMatrix * rPrevVector);
                        moControlVector->setPrevVector(a, aPrevVector);
                    }

                    if(!rNextVector.equalZero())
                    {
                        basegfx::B2DVector aNextVector(rMatrix * rNextVector);
                        moControlVector->setNextVector(a, aNextVector);
                    }
                }

                aCandidate *= rMatrix;
                maPoints.setCoordinate(a, aCandidate);
            }

            if(!moControlVector->isUsed())
                moControlVector.reset();
        }
        else
        {
            maPoints.transform(rMatrix);
        }
    }

    void translate(double fTranslateX, double fTranslateY)
    {
        mpBufferedData.reset();

        if(moControlVector)
        {
            for(sal_uInt32 a(0); a < maPoints.count(); a++)
            {
                basegfx::B2DPoint aCandidate = maPoints.getCoordinate(a);
                aCandidate += basegfx::B2DPoint(fTranslateX, fTranslateY);
                maPoints.setCoordinate(a, aCandidate);
            }

            if(!moControlVector->isUsed())
                moControlVector.reset();
        }
        else
        {
            maPoints.translate(fTranslateX, fTranslateY);
        }
    }

    void addOrReplaceSystemDependentData(basegfx::SystemDependentData_SharedPtr& rData) const
    {
        if(!mpBufferedData)
        {
            mpBufferedData.reset(new ImplBufferedData);
        }

        mpBufferedData->addOrReplaceSystemDependentData(rData);
    }

    basegfx::SystemDependentData_SharedPtr getSystemDependentData(basegfx::SDD_Type aType) const
    {
        if(mpBufferedData)
        {
            return mpBufferedData->getSystemDependentData(aType);
        }

        return basegfx::SystemDependentData_SharedPtr();
    }
};

namespace basegfx
{
    static o3tl::cow_wrapper<ImplB2DPolygon> DEFAULT;

    B2DPolygon::B2DPolygon()
        : mpPolygon(DEFAULT) {}

    B2DPolygon::B2DPolygon(std::initializer_list<basegfx::B2DPoint> aPoints)
    {
        for (const basegfx::B2DPoint& rPoint : aPoints)
        {
            append(rPoint);
        }
    }

    B2DPolygon::B2DPolygon(const B2DPolygon&) = default;

    B2DPolygon::B2DPolygon(B2DPolygon&&) = default;

    B2DPolygon::B2DPolygon(const B2DPolygon& rPolygon, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   mpPolygon(ImplB2DPolygon(*rPolygon.mpPolygon, nIndex, nCount))
    {
    }

    B2DPolygon::~B2DPolygon() = default;

    B2DPolygon& B2DPolygon::operator=(const B2DPolygon&) = default;

    B2DPolygon& B2DPolygon::operator=(B2DPolygon&&) = default;

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

    sal_uInt32 B2DPolygon::count() const
    {
        return mpPolygon->count();
    }

    B2DPoint const & B2DPolygon::getB2DPoint(sal_uInt32 nIndex) const
    {
        return mpPolygon->getPoint(nIndex);
    }

    void B2DPolygon::setB2DPoint(sal_uInt32 nIndex, const B2DPoint& rValue)
    {
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
        if(nCount)
        {
            mpPolygon->insert(nIndex, rPoint, nCount);
        }
    }

    void B2DPolygon::append(const B2DPoint& rPoint, sal_uInt32 nCount)
    {
        if(nCount)
        {
            mpPolygon->append(rPoint, nCount);
        }
    }

    void B2DPolygon::append(const B2DPoint& rPoint)
    {
        mpPolygon->append(rPoint);
    }

    const basegfx::B2DVector& B2DPolygon::getPrevControlVector(sal_uInt32 nIndex) const
    {
        return mpPolygon->getPrevControlVector(nIndex);
    }

    const basegfx::B2DVector& B2DPolygon::getNextControlVector(sal_uInt32 nIndex) const
    {
        return mpPolygon->getNextControlVector(nIndex);
    }

    B2DPoint B2DPolygon::getPrevControlPoint(sal_uInt32 nIndex) const
    {
        if(areControlPointsUsed())
        {
            return getB2DPoint(nIndex) + getPrevControlVector(nIndex);
        }
        else
        {
            return getB2DPoint(nIndex);
        }
    }

    B2DPoint B2DPolygon::getNextControlPoint(sal_uInt32 nIndex) const
    {
        if(areControlPointsUsed())
        {
            return getB2DPoint(nIndex) + getNextControlVector(nIndex);
        }
        else
        {
            return getB2DPoint(nIndex);
        }
    }

    void B2DPolygon::setPrevControlPoint(sal_uInt32 nIndex, const B2DPoint& rValue)
    {
        const basegfx::B2DVector aNewVector(rValue - getB2DPoint(nIndex));

        if(getPrevControlVector(nIndex) != aNewVector)
        {
            mpPolygon->setPrevControlVector(nIndex, aNewVector);
        }
    }

    void B2DPolygon::setNextControlPoint(sal_uInt32 nIndex, const B2DPoint& rValue)
    {
        const basegfx::B2DVector aNewVector(rValue - getB2DPoint(nIndex));

        if(getNextControlVector(nIndex) != aNewVector)
        {
            mpPolygon->setNextControlVector(nIndex, aNewVector);
        }
    }

    void B2DPolygon::setControlPoints(sal_uInt32 nIndex, const basegfx::B2DPoint& rPrev, const basegfx::B2DPoint& rNext)
    {
        const B2DPoint aPoint(getB2DPoint(nIndex));
        const basegfx::B2DVector aNewPrev(rPrev - aPoint);
        const basegfx::B2DVector aNewNext(rNext - aPoint);

        if(getPrevControlVector(nIndex) != aNewPrev || getNextControlVector(nIndex) != aNewNext)
        {
            mpPolygon->setControlVectors(nIndex, aNewPrev, aNewNext);
        }
    }

    void B2DPolygon::resetPrevControlPoint(sal_uInt32 nIndex)
    {
        if(areControlPointsUsed() && !getPrevControlVector(nIndex).equalZero())
        {
            mpPolygon->setPrevControlVector(nIndex, B2DVector::getEmptyVector());
        }
    }

    void B2DPolygon::resetNextControlPoint(sal_uInt32 nIndex)
    {
        if(areControlPointsUsed() && !getNextControlVector(nIndex).equalZero())
        {
            mpPolygon->setNextControlVector(nIndex, B2DVector::getEmptyVector());
        }
    }

    void B2DPolygon::resetControlPoints()
    {
        if(areControlPointsUsed())
        {
            mpPolygon->resetControlVectors();
        }
    }

    void B2DPolygon::appendBezierSegment(
        const B2DPoint& rNextControlPoint,
        const B2DPoint& rPrevControlPoint,
        const B2DPoint& rPoint)
    {
        const B2DVector aNewNextVector(count() ? B2DVector(rNextControlPoint - getB2DPoint(count() - 1)) : B2DVector::getEmptyVector());
        const B2DVector aNewPrevVector(rPrevControlPoint - rPoint);

        if(aNewNextVector.equalZero() && aNewPrevVector.equalZero())
        {
            mpPolygon->append(rPoint);
        }
        else
        {
            mpPolygon->appendBezierSegment(aNewNextVector, aNewPrevVector, rPoint);
        }
    }

    void B2DPolygon::appendQuadraticBezierSegment(const B2DPoint& rControlPoint, const B2DPoint& rPoint)
    {
        if (count() == 0)
        {
            mpPolygon->append(rPoint);
            const double nX((rControlPoint.getX() * 2.0 + rPoint.getX()) / 3.0);
            const double nY((rControlPoint.getY() * 2.0 + rPoint.getY()) / 3.0);
            setPrevControlPoint(0, B2DPoint(nX, nY));
        }
        else
        {
            const B2DPoint aPreviousPoint(getB2DPoint(count() - 1));

            const double nX1((rControlPoint.getX() * 2.0 + aPreviousPoint.getX()) / 3.0);
            const double nY1((rControlPoint.getY() * 2.0 + aPreviousPoint.getY()) / 3.0);
            const double nX2((rControlPoint.getX() * 2.0 + rPoint.getX()) / 3.0);
            const double nY2((rControlPoint.getY() * 2.0 + rPoint.getY()) / 3.0);

            appendBezierSegment(B2DPoint(nX1, nY1), B2DPoint(nX2, nY2), rPoint);
        }
    }

    bool B2DPolygon::areControlPointsUsed() const
    {
        return mpPolygon->areControlPointsUsed();
    }

    bool B2DPolygon::isPrevControlPointUsed(sal_uInt32 nIndex) const
    {
        return (areControlPointsUsed() && !getPrevControlVector(nIndex).equalZero());
    }

    bool B2DPolygon::isNextControlPointUsed(sal_uInt32 nIndex) const
    {
        return (areControlPointsUsed() && !getNextControlVector(nIndex).equalZero());
    }

    B2VectorContinuity B2DPolygon::getContinuityInPoint(sal_uInt32 nIndex) const
    {
        if(areControlPointsUsed())
        {
            const B2DVector& rPrev(getPrevControlVector(nIndex));
            const B2DVector& rNext(getNextControlVector(nIndex));

            return getContinuity(rPrev, rNext);
        }
        else
        {
            return B2VectorContinuity::NONE;
        }
    }

    void B2DPolygon::getBezierSegment(sal_uInt32 nIndex, B2DCubicBezier& rTarget) const
    {
        const bool bNextIndexValidWithoutClose(nIndex + 1 < count());

        if(bNextIndexValidWithoutClose || isClosed())
        {
            const sal_uInt32 nNextIndex(bNextIndexValidWithoutClose ? nIndex + 1 : 0);
            rTarget.setStartPoint(getB2DPoint(nIndex));
            rTarget.setEndPoint(getB2DPoint(nNextIndex));

            if(areControlPointsUsed())
            {
                rTarget.setControlPointA(rTarget.getStartPoint() + getNextControlVector(nIndex));
                rTarget.setControlPointB(rTarget.getEndPoint() + getPrevControlVector(nNextIndex));
            }
            else
            {
                // no bezier, reset control points at rTarget
                rTarget.setControlPointA(rTarget.getStartPoint());
                rTarget.setControlPointB(rTarget.getEndPoint());
            }
        }
        else
        {
            // no valid edge at all, reset rTarget to current point
            const B2DPoint aPoint(getB2DPoint(nIndex));
            rTarget.setStartPoint(aPoint);
            rTarget.setEndPoint(aPoint);
            rTarget.setControlPointA(aPoint);
            rTarget.setControlPointB(aPoint);
        }
    }

    B2DPolygon const & B2DPolygon::getDefaultAdaptiveSubdivision() const
    {
        return mpPolygon->getDefaultAdaptiveSubdivision(*this);
    }

    B2DRange const & B2DPolygon::getB2DRange() const
    {
        return mpPolygon->getB2DRange(*this);
    }

    void B2DPolygon::append(const B2DPolygon& rPoly, sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        assert(nIndex + nCount <= rPoly.count());

        if(!nCount)
        {
            nCount = rPoly.count() - nIndex;
            if (!nCount)
                return;
        }

        if(nIndex == 0 && nCount == rPoly.count())
        {
            mpPolygon->append(*rPoly.mpPolygon);
        }
        else
        {
            mpPolygon->append(*rPoly.mpPolygon, nIndex, nCount);
        }
    }

    void B2DPolygon::remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(nCount)
        {
            mpPolygon->remove(nIndex, nCount);
        }
    }

    void B2DPolygon::clear()
    {
        *mpPolygon = ImplB2DPolygon();
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
        return (count() > 1 && mpPolygon->hasDoublePoints());
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
        if(count() && !rMatrix.isIdentity())
        {
            mpPolygon->transform(rMatrix);
        }
    }

    void B2DPolygon::translate(double fTranslateX, double fTranslateY)
    {
        if(count())
        {
            mpPolygon->translate(fTranslateX, fTranslateY);
        }
    }

    void B2DPolygon::addOrReplaceSystemDependentDataInternal(SystemDependentData_SharedPtr& rData) const
    {
        mpPolygon->addOrReplaceSystemDependentData(rData);
    }

    SystemDependentData_SharedPtr B2DPolygon::getSystemDependantDataInternal(SDD_Type aType) const
    {
        return mpPolygon->getSystemDependentData(aType);
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
