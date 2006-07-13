/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolygon.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 09:56:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POINT_B2DPOINT_HXX
#include <basegfx/point/b2dpoint.hxx>
#endif

#ifndef _BGFX_VECTOR_B2DVECTOR_HXX
#include <basegfx/vector/b2dvector.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

#include <boost/scoped_ptr.hpp>
#include <vector>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////

class CoordinateData2D
{
    ::basegfx::B2DPoint                             maPoint;

public:
    CoordinateData2D() : maPoint() {}
    explicit CoordinateData2D(const ::basegfx::B2DPoint& rData) : maPoint(rData) {}

    const ::basegfx::B2DPoint& getCoordinate() const { return maPoint; }
    void setCoordinate(const ::basegfx::B2DPoint& rValue) { if(rValue != maPoint) maPoint = rValue; }
    bool operator==(const CoordinateData2D& rData ) const { return (maPoint == rData.getCoordinate()); }
    void transform(const ::basegfx::B2DHomMatrix& rMatrix) { maPoint *= rMatrix; }
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

    bool isEqual(const CoordinateDataArray2D& rCandidate) const
    {
        return (maVector == rCandidate.maVector);
    }

    const ::basegfx::B2DPoint& getCoordinate(sal_uInt32 nIndex) const
    {
        return maVector[nIndex].getCoordinate();
    }

    void setCoordinate(sal_uInt32 nIndex, const ::basegfx::B2DPoint& rValue)
    {
        maVector[nIndex].setCoordinate(rValue);
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
            const sal_uInt32 nHalfSize(bIsClosed ? (maVector.size() - 1L) >> 1L : maVector.size() >> 1L);
            CoordinateData2DVector::iterator aStart(bIsClosed ? maVector.begin() + 1L : maVector.begin());
            CoordinateData2DVector::iterator aEnd(maVector.end() - 1L);

            for(sal_uInt32 a(0); a < nHalfSize; a++)
            {
                ::std::swap(*aStart, *aEnd);
                aStart++;
                aEnd--;
            }
        }
    }

    void removeDoublePointsAtBeginEnd()
    {
        // remove from end as long as there are at least two points
        // and begin/end are equal
        while((maVector.size() > 1L) && (maVector[0] == maVector[maVector.size() - 1L]))
        {
            maVector.pop_back();
        }
    }

    void removeDoublePointsWholeTrack()
    {
        sal_uInt32 nIndex(0L);

        // test as long as there are at least two points and as long as the index
        // is smaller or equal second last point
        while((maVector.size() > 1L) && (nIndex <= maVector.size() - 2L))
        {
            if(maVector[nIndex] == maVector[nIndex + 1L])
            {
                // if next is same as index, delete next
                maVector.erase(maVector.begin() + (nIndex + 1L));
            }
            else
            {
                // if different, step forward
                nIndex++;
            }
        }
    }

    void transform(const ::basegfx::B2DHomMatrix& rMatrix)
    {
        CoordinateData2DVector::iterator aStart(maVector.begin());
        CoordinateData2DVector::iterator aEnd(maVector.end());

        for(; aStart != aEnd; aStart++)
        {
            aStart->transform(rMatrix);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

class ControlVectorPair2D
{
    ::basegfx::B2DVector                            maVectorA;
    ::basegfx::B2DVector                            maVectorB;

public:
    const ::basegfx::B2DVector& getVectorA() const { return maVectorA; }
    void setVectorA(const ::basegfx::B2DVector& rValue) { if(rValue != maVectorA) maVectorA = rValue; }

    const ::basegfx::B2DVector& getVectorB() const { return maVectorB; }
    void setVectorB(const ::basegfx::B2DVector& rValue) { if(rValue != maVectorB) maVectorB = rValue; }

    bool operator==(const ControlVectorPair2D& rData ) const
        { return (maVectorA == rData.getVectorA() && maVectorB == rData.getVectorB()); }
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
        mnUsedVectors(0L)
    {
    }

    ControlVectorArray2D(const ControlVectorArray2D& rOriginal, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   maVector(),
        mnUsedVectors(0L)
    {
        ControlVectorPair2DVector::const_iterator aStart(rOriginal.maVector.begin());
        aStart += nIndex;
        ControlVectorPair2DVector::const_iterator aEnd(aStart);
        aEnd += nCount;
        maVector.reserve(nCount);

        for(; aStart != aEnd; aStart++)
        {
            if(!aStart->getVectorA().equalZero())
                mnUsedVectors++;

            if(!aStart->getVectorB().equalZero())
                mnUsedVectors++;

            maVector.push_back(*aStart);
        }
    }

    sal_uInt32 count() const
    {
        return maVector.size();
    }

    bool isEqual(const ControlVectorArray2D& rCandidate) const
    {
        return (maVector == rCandidate.maVector);
    }

    bool isUsed() const
    {
        return (0L != mnUsedVectors);
    }

    const ::basegfx::B2DVector& getVectorA(sal_uInt32 nIndex) const
    {
        return maVector[nIndex].getVectorA();
    }

    void setVectorA(sal_uInt32 nIndex, const ::basegfx::B2DVector& rValue)
    {
        bool bWasUsed(mnUsedVectors && !maVector[nIndex].getVectorA().equalZero());
        bool bIsUsed(!rValue.equalZero());

        if(bWasUsed)
        {
            if(bIsUsed)
            {
                maVector[nIndex].setVectorA(rValue);
            }
            else
            {
                maVector[nIndex].setVectorA(::basegfx::B2DVector::getEmptyVector());
                mnUsedVectors--;
            }
        }
        else
        {
            if(bIsUsed)
            {
                maVector[nIndex].setVectorA(rValue);
                mnUsedVectors++;
            }
        }
    }

    const ::basegfx::B2DVector& getVectorB(sal_uInt32 nIndex) const
    {
        return maVector[nIndex].getVectorB();
    }

    void setVectorB(sal_uInt32 nIndex, const ::basegfx::B2DVector& rValue)
    {
        bool bWasUsed(mnUsedVectors && !maVector[nIndex].getVectorB().equalZero());
        bool bIsUsed(!rValue.equalZero());

        if(bWasUsed)
        {
            if(bIsUsed)
            {
                maVector[nIndex].setVectorB(rValue);
            }
            else
            {
                maVector[nIndex].setVectorB(::basegfx::B2DVector::getEmptyVector());
                mnUsedVectors--;
            }
        }
        else
        {
            if(bIsUsed)
            {
                maVector[nIndex].setVectorB(rValue);
                mnUsedVectors++;
            }
        }
    }

    void insert(sal_uInt32 nIndex, const ControlVectorPair2D& rValue, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // add nCount copies of rValue
            ControlVectorPair2DVector::iterator aIndex(maVector.begin());
            aIndex += nIndex;
            maVector.insert(aIndex, nCount, rValue);

            if(!rValue.getVectorA().equalZero())
                mnUsedVectors += nCount;

            if(!rValue.getVectorB().equalZero())
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

            for(; aStart != aEnd; aStart++)
            {
                if(!aStart->getVectorA().equalZero())
                    mnUsedVectors++;

                if(!aStart->getVectorB().equalZero())
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

            for(; mnUsedVectors && aStart != aDeleteEnd; aStart++)
            {
                if(!aStart->getVectorA().equalZero())
                    mnUsedVectors--;

                if(mnUsedVectors && !aStart->getVectorB().equalZero())
                    mnUsedVectors--;
            }

            // remove point data
            maVector.erase(aDeleteStart, aDeleteEnd);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

class ImplB2DPolygon
{
    // The point vector. This vector exists always and defines the
    // count of members.
    CoordinateDataArray2D                           maPoints;

    // The control point vectors. This vectors are created on demand
    // and may be zero.
    boost::scoped_ptr<ControlVectorArray2D>         mpControlVector;

    // flag which decides if this polygon is opened or closed
    bool                                            mbIsClosed;

public:
    ImplB2DPolygon() :
        maPoints(0L),
        mpControlVector(),
        mbIsClosed(false)
    {
    }

    ImplB2DPolygon(const ImplB2DPolygon& rToBeCopied) :
        maPoints(rToBeCopied.maPoints),
        mpControlVector(),
        mbIsClosed(rToBeCopied.mbIsClosed)
    {
        // complete initialization using copy
        if(rToBeCopied.mpControlVector && rToBeCopied.mpControlVector->isUsed())
            mpControlVector.reset( new ControlVectorArray2D(*rToBeCopied.mpControlVector) );
    }

    ImplB2DPolygon(const ImplB2DPolygon& rToBeCopied, sal_uInt32 nIndex, sal_uInt32 nCount) :
        maPoints(rToBeCopied.maPoints, nIndex, nCount),
        mpControlVector(),
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
            mbIsClosed = bNew;
        }
    }

    bool isEqual(const ImplB2DPolygon& rCandidate) const
    {
        if(mbIsClosed == rCandidate.mbIsClosed)
        {
            if(maPoints.isEqual(rCandidate.maPoints))
            {
                bool bControlVectorsAreEqual(true);

                if(mpControlVector)
                {
                    if(rCandidate.mpControlVector)
                    {
                        bControlVectorsAreEqual = mpControlVector->isEqual(*rCandidate.mpControlVector);
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

    const ::basegfx::B2DPoint& getPoint(sal_uInt32 nIndex) const
    {
        return maPoints.getCoordinate(nIndex);
    }

    void setPoint(sal_uInt32 nIndex, const ::basegfx::B2DPoint& rValue)
    {
        maPoints.setCoordinate(nIndex, rValue);
    }

    void insert(sal_uInt32 nIndex, const ::basegfx::B2DPoint& rPoint, sal_uInt32 nCount)
    {
        if(nCount)
        {
            CoordinateData2D aCoordinate(rPoint);
            maPoints.insert(nIndex, aCoordinate, nCount);

            if(mpControlVector)
            {
                ControlVectorPair2D aVectorPair;
                mpControlVector->insert(nIndex, aVectorPair, nCount);
            }
        }
    }

    const ::basegfx::B2DVector& getControlVectorA(sal_uInt32 nIndex) const
    {
        if(mpControlVector)
        {
            return mpControlVector->getVectorA(nIndex);
        }
        else
        {
            return ::basegfx::B2DVector::getEmptyVector();
        }
    }

    void setControlVectorA(sal_uInt32 nIndex, const ::basegfx::B2DVector& rValue)
    {
        if(!mpControlVector)
        {
            if(!rValue.equalZero())
            {
                mpControlVector.reset( new ControlVectorArray2D(maPoints.count()) );
                mpControlVector->setVectorA(nIndex, rValue);
            }
        }
        else
        {
            mpControlVector->setVectorA(nIndex, rValue);

            if(!mpControlVector->isUsed())
                mpControlVector.reset();
        }
    }

    const ::basegfx::B2DVector& getControlVectorB(sal_uInt32 nIndex) const
    {
        if(mpControlVector)
        {
            return mpControlVector->getVectorB(nIndex);
        }
        else
        {
            return ::basegfx::B2DVector::getEmptyVector();
        }
    }

    void setControlVectorB(sal_uInt32 nIndex, const ::basegfx::B2DVector& rValue)
    {
        if(!mpControlVector)
        {
            if(!rValue.equalZero())
            {
                mpControlVector.reset( new ControlVectorArray2D(maPoints.count()) );
                mpControlVector->setVectorB(nIndex, rValue);
            }
        }
        else
        {
            mpControlVector->setVectorB(nIndex, rValue);

            if(!mpControlVector->isUsed())
                mpControlVector.reset();
        }
    }

    bool areControlVectorsUsed() const
    {
        return (mpControlVector && mpControlVector->isUsed());
    }

    void insert(sal_uInt32 nIndex, const ImplB2DPolygon& rSource)
    {
        const sal_uInt32 nCount(rSource.maPoints.count());

        if(nCount)
        {
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
            if(mpControlVector)
            {
                // Here, the vectors need to be completely flipped. The new vectors
                // rely on the predecessor point and A,B need to be flipped. The last
                // vectors need to be moved to first, too. It is also necessary to
                // copy the points (of courcse, just to not forget).
                const sal_uInt32 nCount(maPoints.count());

                // create copies to have access to source data
                boost::scoped_ptr<CoordinateDataArray2D> pCoordinateCopy( new CoordinateDataArray2D(maPoints) );
                boost::scoped_ptr<ControlVectorArray2D>  pVectorCopy( new ControlVectorArray2D(*mpControlVector) );

                // newly fill the local point and vector data
                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    // get index for source point. If closed, use other pattern for calculation of source
                    // point to enable keeping the same point on index 0.
                    const sal_uInt32 nCoorSource(mbIsClosed ? ((nCount - a) % nCount) : nCount - (a + 1));

                    // get index for predecessor point
                    const sal_uInt32 nVectorSource(nCoorSource ? nCoorSource - 1L : nCount - 1L);

                    // get source data
                    const ::basegfx::B2DPoint& rSourceCoor = pCoordinateCopy->getCoordinate(nCoorSource);
                    const ::basegfx::B2DPoint& rVectorSourceCoor = pCoordinateCopy->getCoordinate(nVectorSource);
                    const ::basegfx::B2DVector& rVectorSourceA = pVectorCopy->getVectorA(nVectorSource);
                    const ::basegfx::B2DVector& rVectorSourceB = pVectorCopy->getVectorB(nVectorSource);

                    // copy point data
                    maPoints.setCoordinate(a, rSourceCoor);

                    // copy vector data A to B
                    if(rVectorSourceA.equalZero())
                    {
                        // unused, use zero vector
                        mpControlVector->setVectorB(a, ::basegfx::B2DVector::getEmptyVector());
                    }
                    else
                    {
                        // calculate new vector relative to new point
                        ::basegfx::B2DVector aNewVectorB((rVectorSourceA + rVectorSourceCoor) - rSourceCoor);
                        mpControlVector->setVectorB(a, aNewVectorB);
                    }

                    // copy vector data B to A
                    if(rVectorSourceB.equalZero())
                    {
                        // unused, use zero vector
                        mpControlVector->setVectorA(a, ::basegfx::B2DVector::getEmptyVector());
                    }
                    else
                    {
                        // calculate new vector relative to new point
                        ::basegfx::B2DVector aNewVectorA((rVectorSourceB + rVectorSourceCoor) - rSourceCoor);
                        mpControlVector->setVectorA(a, aNewVectorA);
                    }
                }
            }
            else
            {
                maPoints.flip(mbIsClosed);
            }
        }
    }

    bool hasDoublePoints() const
    {
        if(mbIsClosed)
        {
            // check for same start and end point
            const sal_uInt32 nIndex(maPoints.count() - 1L);

            if(maPoints.getCoordinate(0L) == maPoints.getCoordinate(nIndex))
            {
                if(mpControlVector)
                {
                    if(mpControlVector->getVectorA(nIndex).equalZero()
                        && mpControlVector->getVectorB(nIndex).equalZero())
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
        for(sal_uInt32 a(0L); a < maPoints.count() - 1L; a++)
        {
            if(maPoints.getCoordinate(a) == maPoints.getCoordinate(a + 1L))
            {
                if(mpControlVector)
                {
                    if(mpControlVector->getVectorA(a).equalZero()
                        && mpControlVector->getVectorB(a).equalZero())
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
            if(mpControlVector)
            {
                bool bRemove;

                do
                {
                    bRemove = false;

                    if(maPoints.count() > 1L)
                    {
                        const sal_uInt32 nIndex(maPoints.count() - 1L);

                        if(maPoints.getCoordinate(0L) == maPoints.getCoordinate(nIndex))
                        {
                            if(mpControlVector->getVectorA(nIndex).equalZero()
                                && mpControlVector->getVectorB(nIndex).equalZero())
                            {
                                bRemove = true;
                            }
                        }
                    }

                    if(bRemove)
                    {
                        const sal_uInt32 nIndex(maPoints.count() - 1L);
                        remove(nIndex, 1L);
                    }
                } while(bRemove);
            }
            else
            {
                maPoints.removeDoublePointsAtBeginEnd();
            }
        }
    }

    void removeDoublePointsWholeTrack()
    {
        if(mpControlVector)
        {
            sal_uInt32 nIndex(0L);

            // test as long as there are at least two points and as long as the index
            // is smaller or equal second last point
            while((maPoints.count() > 1L) && (nIndex <= maPoints.count() - 2L))
            {
                if(maPoints.getCoordinate(nIndex) == maPoints.getCoordinate(nIndex + 1L)
                    && mpControlVector->getVectorA(nIndex).equalZero()
                    && mpControlVector->getVectorB(nIndex).equalZero())
                {
                    // if next is same as index and the control vectors are unused, delete index
                    remove(nIndex, 1L);
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

    void transform(const ::basegfx::B2DHomMatrix& rMatrix)
    {
        if(mpControlVector)
        {
            for(sal_uInt32 a(0L); a < maPoints.count(); a++)
            {
                ::basegfx::B2DPoint aCandidate = maPoints.getCoordinate(a);

                if(mpControlVector->isUsed())
                {
                    const ::basegfx::B2DVector& rVectorA(mpControlVector->getVectorA(a));
                    const ::basegfx::B2DVector& rVectorB(mpControlVector->getVectorB(a));

                    if(!rVectorA.equalZero())
                    {
                        ::basegfx::B2DVector aVectorA(rMatrix * rVectorA);
                        mpControlVector->setVectorA(a, aVectorA);
                    }

                    if(!rVectorB.equalZero())
                    {
                        ::basegfx::B2DVector aVectorB(rMatrix * rVectorB);
                        mpControlVector->setVectorB(a, aVectorB);
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
};

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace { struct DefaultPolygon: public rtl::Static<B2DPolygon::ImplType,
                                                          DefaultPolygon> {}; }

    B2DPolygon::B2DPolygon() :
        mpPolygon(DefaultPolygon::get())
    {
    }

    B2DPolygon::B2DPolygon(const B2DPolygon& rPolygon) :
        mpPolygon(rPolygon.mpPolygon)
    {
    }

    B2DPolygon::B2DPolygon(const B2DPolygon& rPolygon, sal_uInt32 nIndex, sal_uInt32 nCount) :
        mpPolygon(ImplB2DPolygon(*rPolygon.mpPolygon, nIndex, nCount))
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

        return mpPolygon->isEqual(*(rPolygon.mpPolygon));
    }

    bool B2DPolygon::operator!=(const B2DPolygon& rPolygon) const
    {
        return !(*this == rPolygon);
    }

    sal_uInt32 B2DPolygon::count() const
    {
        return mpPolygon->count();
    }

    ::basegfx::B2DPoint B2DPolygon::getB2DPoint(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        return mpPolygon->getPoint(nIndex);
    }

    void B2DPolygon::setB2DPoint(sal_uInt32 nIndex, const ::basegfx::B2DPoint& rValue)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        if(getB2DPoint(nIndex) != rValue)
            mpPolygon->setPoint(nIndex, rValue);
    }

    void B2DPolygon::insert(sal_uInt32 nIndex, const ::basegfx::B2DPoint& rPoint, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex <= mpPolygon->count(), "B2DPolygon Insert outside range (!)");

        if(nCount)
            mpPolygon->insert(nIndex, rPoint, nCount);
    }

    void B2DPolygon::append(const ::basegfx::B2DPoint& rPoint, sal_uInt32 nCount)
    {
        if(nCount)
            mpPolygon->insert(mpPolygon->count(), rPoint, nCount);
    }

    ::basegfx::B2DVector B2DPolygon::getControlVectorA(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        return mpPolygon->getControlVectorA(nIndex);
    }

    void B2DPolygon::setControlVectorA(sal_uInt32 nIndex, const ::basegfx::B2DVector& rValue)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        if(getControlVectorA(nIndex) != rValue)
            mpPolygon->setControlVectorA(nIndex, rValue);
    }

    ::basegfx::B2DVector B2DPolygon::getControlVectorB(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        return mpPolygon->getControlVectorB(nIndex);
    }

    void B2DPolygon::setControlVectorB(sal_uInt32 nIndex, const ::basegfx::B2DVector& rValue)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        if(getControlVectorB(nIndex) != rValue)
            mpPolygon->setControlVectorB(nIndex, rValue);
    }

    bool B2DPolygon::areControlVectorsUsed() const
    {
        return mpPolygon->areControlVectorsUsed();
    }

    ::basegfx::B2DPoint B2DPolygon::getControlPointA(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        if(mpPolygon->areControlVectorsUsed())
        {
            return mpPolygon->getPoint(nIndex) + mpPolygon->getControlVectorA(nIndex);
        }
        else
        {
            return mpPolygon->getPoint(nIndex);
        }
    }

    void B2DPolygon::setControlPointA(sal_uInt32 nIndex, const ::basegfx::B2DPoint& rValue)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");
        ::basegfx::B2DVector aNewVector(rValue - mpPolygon->getPoint(nIndex));

        if(getControlVectorA(nIndex) != aNewVector)
            mpPolygon->setControlVectorA(nIndex, aNewVector);
    }

    ::basegfx::B2DPoint B2DPolygon::getControlPointB(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");

        if(mpPolygon->areControlVectorsUsed())
        {
            return mpPolygon->getPoint(nIndex) + mpPolygon->getControlVectorB(nIndex);
        }
        else
        {
            return mpPolygon->getPoint(nIndex);
        }
    }

    void B2DPolygon::setControlPointB(sal_uInt32 nIndex, const ::basegfx::B2DPoint& rValue)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B2DPolygon access outside range (!)");
        ::basegfx::B2DVector aNewVector(rValue - mpPolygon->getPoint(nIndex));

        if(getControlVectorB(nIndex) != aNewVector)
            mpPolygon->setControlVectorB(nIndex, aNewVector);
    }

    void B2DPolygon::insert(sal_uInt32 nIndex, const B2DPolygon& rPoly, sal_uInt32 nIndex2, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex <= mpPolygon->count(), "B2DPolygon Insert outside range (!)");

        if(rPoly.count())
        {
            if(!nCount)
            {
                nCount = rPoly.count();
            }

            if(0L == nIndex2 && nCount == rPoly.count())
            {
                mpPolygon->insert(nIndex, *rPoly.mpPolygon);
            }
            else
            {
                OSL_ENSURE(nIndex2 + nCount <= rPoly.mpPolygon->count(), "B2DPolygon Insert outside range (!)");
                ImplB2DPolygon aTempPoly(*rPoly.mpPolygon, nIndex2, nCount);
                mpPolygon->insert(nIndex, aTempPoly);
            }
        }
    }

    void B2DPolygon::append(const B2DPolygon& rPoly, sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(rPoly.count())
        {
            if(!nCount)
            {
                nCount = rPoly.count();
            }

            if(0L == nIndex && nCount == rPoly.count())
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
            mpPolygon->remove(nIndex, nCount);
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
            mpPolygon->setClosed(bNew);
    }

    void B2DPolygon::flip()
    {
        if(count() > 1)
            mpPolygon->flip();
    }

    bool B2DPolygon::hasDoublePoints() const
    {
        return (mpPolygon->count() > 1L && mpPolygon->hasDoublePoints());
    }

    void B2DPolygon::removeDoublePoints()
    {
        if(hasDoublePoints())
        {
            mpPolygon->removeDoublePointsAtBeginEnd();
            mpPolygon->removeDoublePointsWholeTrack();
        }
    }

    void B2DPolygon::transform(const ::basegfx::B2DHomMatrix& rMatrix)
    {
        if(count())
            mpPolygon->transform(rMatrix);
    }
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
