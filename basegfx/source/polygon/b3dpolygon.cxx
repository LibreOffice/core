/*************************************************************************
 *
 *  $RCSfile: b3dpolygon.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 17:18:08 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

#ifndef _BGFX_POINT_B3DPOINT_HXX
#include <basegfx/point/b3dpoint.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

#include <vector>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////

class CoordinateData3D
{
    ::basegfx::B3DPoint                             maPoint;

public:
    CoordinateData3D() {}
    CoordinateData3D(const ::basegfx::B3DPoint& rData) : maPoint(rData) {}
    ~CoordinateData3D() {}

    const ::basegfx::B3DPoint& getCoordinate() const { return maPoint; }
    void setCoordinate(const ::basegfx::B3DPoint& rValue) { if(rValue != maPoint) maPoint = rValue; }
    bool operator==(const CoordinateData3D& rData ) const { return (maPoint == rData.getCoordinate()); }
    void transform(const ::basegfx::B3DHomMatrix& rMatrix) { maPoint *= rMatrix; }
};

//////////////////////////////////////////////////////////////////////////////

class CoordinateDataArray3D
{
    typedef ::std::vector< CoordinateData3D > CoordinateData3DVector;

    CoordinateData3DVector                          maVector;

public:
    CoordinateDataArray3D(sal_uInt32 nCount)
    :   maVector(nCount)
    {
    }

    CoordinateDataArray3D(const CoordinateDataArray3D& rOriginal)
    :   maVector(rOriginal.maVector)
    {
    }

    CoordinateDataArray3D(const CoordinateDataArray3D& rOriginal, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   maVector(rOriginal.maVector.begin() + nIndex, rOriginal.maVector.begin() + (nIndex + nCount))
    {
    }

    ~CoordinateDataArray3D()
    {
    }

    sal_uInt32 count() const
    {
        return maVector.size();
    }

    bool isEqual(const CoordinateDataArray3D& rCandidate) const
    {
        return (maVector == rCandidate.maVector);
    }

    const ::basegfx::B3DPoint& getCoordinate(sal_uInt32 nIndex) const
    {
        return maVector[nIndex].getCoordinate();
    }

    void setCoordinate(sal_uInt32 nIndex, const ::basegfx::B3DPoint& rValue)
    {
        maVector[nIndex].setCoordinate(rValue);
    }

    void insert(sal_uInt32 nIndex, const CoordinateData3D& rValue, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // add nCount copies of rValue
            CoordinateData3DVector::iterator aIndex(maVector.begin());
            aIndex += nIndex;
            maVector.insert(aIndex, nCount, rValue);
        }
    }

    void insert(sal_uInt32 nIndex, const CoordinateDataArray3D& rSource)
    {
        const sal_uInt32 nCount(rSource.maVector.size());

        if(nCount)
        {
            // insert data
            CoordinateData3DVector::iterator aIndex(maVector.begin());
            aIndex += nIndex;
            CoordinateData3DVector::const_iterator aStart(rSource.maVector.begin());
            CoordinateData3DVector::const_iterator aEnd(rSource.maVector.end());
            maVector.insert(aIndex, aStart, aEnd);
        }
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // remove point data
            CoordinateData3DVector::iterator aStart(maVector.begin());
            aStart += nIndex;
            const CoordinateData3DVector::iterator aEnd(aStart + nCount);
            maVector.erase(aStart, aEnd);
        }
    }

    void flip()
    {
        if(maVector.size() > 1)
        {
            const sal_uInt32 nHalfSize(maVector.size() >> 1L);
            CoordinateData3DVector::iterator aStart(maVector.begin());
            CoordinateData3DVector::iterator aEnd(maVector.end() - 1L);

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

    void transform(const ::basegfx::B3DHomMatrix& rMatrix)
    {
        CoordinateData3DVector::iterator aStart(maVector.begin());
        CoordinateData3DVector::iterator aEnd(maVector.end());

        for(; aStart != aEnd; aStart++)
        {
            aStart->transform(rMatrix);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

class ImplB3DPolygon
{
    // the internal RefCount
    sal_uInt32                                      mnRefCount;

    // The point vector. This vector exists always and defines the
    // count of members.
    CoordinateDataArray3D                           maPoints;

    // bitfield
    // flag which decides if this polygon is opened or closed
    unsigned                                        mbIsClosed : 1;

public:
    // This constructor is only used from the static identity polygon, thus
    // the RefCount is set to 1 to never 'delete' this static incarnation.
    ImplB3DPolygon()
    :   mnRefCount(1),
        maPoints(0L),
        mbIsClosed(false)
    {
        // complete initialization with defaults
    }

    ImplB3DPolygon(const ImplB3DPolygon& rToBeCopied)
    :   mnRefCount(0),
        maPoints(rToBeCopied.maPoints),
        mbIsClosed(rToBeCopied.mbIsClosed)
    {
        // complete initialization using copy
    }

    ImplB3DPolygon(const ImplB3DPolygon& rToBeCopied, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   mnRefCount(0),
        maPoints(rToBeCopied.maPoints, nIndex, nCount),
        mbIsClosed(rToBeCopied.mbIsClosed)
    {
        // complete initialization using partly copy
    }

    ~ImplB3DPolygon()
    {
    }

    const sal_uInt32 getRefCount() const
    {
        return mnRefCount;
    }

    void incRefCount()
    {
        mnRefCount++;
    }

    void decRefCount()
    {
        mnRefCount--;
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

    bool isEqual(const ImplB3DPolygon& rCandidate) const
    {
        if(mbIsClosed == rCandidate.mbIsClosed)
        {
            if(maPoints.isEqual(rCandidate.maPoints))
            {
                return true;
            }
        }

        return false;
    }

    const ::basegfx::B3DPoint& getPoint(sal_uInt32 nIndex) const
    {
        return maPoints.getCoordinate(nIndex);
    }

    void setPoint(sal_uInt32 nIndex, const ::basegfx::B3DPoint& rValue)
    {
        maPoints.setCoordinate(nIndex, rValue);
    }

    void insert(sal_uInt32 nIndex, const ::basegfx::B3DPoint& rPoint, sal_uInt32 nCount)
    {
        if(nCount)
        {
            CoordinateData3D aCoordinate(rPoint);
            maPoints.insert(nIndex, aCoordinate, nCount);
        }
    }

    void insert(sal_uInt32 nIndex, const ImplB3DPolygon& rSource)
    {
        const sal_uInt32 nCount(rSource.maPoints.count());

        if(nCount)
        {
            maPoints.insert(nIndex, rSource.maPoints);
        }
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(nCount)
        {
            maPoints.remove(nIndex, nCount);
        }
    }

    void flip()
    {
        if(maPoints.count() > 1)
        {
            maPoints.flip();
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
                return true;
            }
        }

        // test for range
        for(sal_uInt32 a(0L); a < maPoints.count() - 1L; a++)
        {
            if(maPoints.getCoordinate(a) == maPoints.getCoordinate(a + 1L))
            {
                return true;
            }
        }

        return false;
    }

    void removeDoublePointsAtBeginEnd()
    {
        // Only remove DoublePoints at Begin and End when poly is closed
        if(mbIsClosed)
        {
            maPoints.removeDoublePointsAtBeginEnd();
        }
    }

    void removeDoublePointsWholeTrack()
    {
        maPoints.removeDoublePointsWholeTrack();
    }

    void transform(const ::basegfx::B3DHomMatrix& rMatrix)
    {
        maPoints.transform(rMatrix);
    }
};

//////////////////////////////////////////////////////////////////////////////

namespace { struct DefaultPolygon : public rtl::Static< ImplB3DPolygon, DefaultPolygon > {}; }

namespace basegfx
{
    void B3DPolygon::implForceUniqueCopy()
    {
        if(mpPolygon->getRefCount())
        {
            mpPolygon->decRefCount();
            mpPolygon = new ImplB3DPolygon(*mpPolygon);
        }
    }

    B3DPolygon::B3DPolygon()
    :   mpPolygon(&DefaultPolygon::get())
    {
        mpPolygon->incRefCount();
    }

    B3DPolygon::B3DPolygon(const B3DPolygon& rPolygon)
    :   mpPolygon(rPolygon.mpPolygon)
    {
        mpPolygon->incRefCount();
    }

    B3DPolygon::B3DPolygon(const B3DPolygon& rPolygon, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   mpPolygon(new ImplB3DPolygon(*rPolygon.mpPolygon, nIndex, nCount))
    {
        OSL_ENSURE(nIndex + nCount > rPolygon.mpPolygon->count(), "B3DPolygon constructor outside range (!)");
    }

    B3DPolygon::~B3DPolygon()
    {
        if(mpPolygon->getRefCount())
        {
            mpPolygon->decRefCount();
        }
        else
        {
            delete mpPolygon;
        }
    }

    B3DPolygon& B3DPolygon::operator=(const B3DPolygon& rPolygon)
    {
        if(mpPolygon->getRefCount())
        {
            mpPolygon->decRefCount();
        }
        else
        {
            delete mpPolygon;
        }

        mpPolygon = rPolygon.mpPolygon;
        mpPolygon->incRefCount();

        return *this;
    }

    bool B3DPolygon::operator==(const B3DPolygon& rPolygon) const
    {
        if(mpPolygon == rPolygon.mpPolygon)
        {
            return true;
        }

        return mpPolygon->isEqual(*(rPolygon.mpPolygon));
    }

    bool B3DPolygon::operator!=(const B3DPolygon& rPolygon) const
    {
        if(mpPolygon == rPolygon.mpPolygon)
        {
            return false;
        }

        return !mpPolygon->isEqual(*(rPolygon.mpPolygon));
    }

    sal_uInt32 B3DPolygon::count() const
    {
        return mpPolygon->count();
    }

    ::basegfx::B3DPoint B3DPolygon::getB3DPoint(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B3DPolygon access outside range (!)");

        return mpPolygon->getPoint(nIndex);
    }

    void B3DPolygon::setB3DPoint(sal_uInt32 nIndex, const ::basegfx::B3DPoint& rValue)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B3DPolygon access outside range (!)");

        if(mpPolygon->getPoint(nIndex) != rValue)
        {
            implForceUniqueCopy();
            mpPolygon->setPoint(nIndex, rValue);
        }
    }

    void B3DPolygon::insert(sal_uInt32 nIndex, const ::basegfx::B3DPoint& rPoint, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex <= mpPolygon->count(), "B3DPolygon Insert outside range (!)");

        if(nCount)
        {
            implForceUniqueCopy();
            mpPolygon->insert(nIndex, rPoint, nCount);
        }
    }

    void B3DPolygon::append(const ::basegfx::B3DPoint& rPoint, sal_uInt32 nCount)
    {
        if(nCount)
        {
            implForceUniqueCopy();
            mpPolygon->insert(mpPolygon->count(), rPoint, nCount);
        }
    }

    void B3DPolygon::insert(sal_uInt32 nIndex, const B3DPolygon& rPoly, sal_uInt32 nIndex2, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex <= mpPolygon->count(), "B3DPolygon Insert outside range (!)");

        if(rPoly.count())
        {
            implForceUniqueCopy();

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
                OSL_ENSURE(nIndex2 + nCount > rPoly.mpPolygon->count(), "B3DPolygon Insert outside range (!)");
                ImplB3DPolygon aTempPoly(*rPoly.mpPolygon, nIndex2, nCount);
                mpPolygon->insert(nIndex, aTempPoly);
            }
        }
    }

    void B3DPolygon::append(const B3DPolygon& rPoly, sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(rPoly.count())
        {
            implForceUniqueCopy();

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
                OSL_ENSURE(nIndex + nCount > rPoly.mpPolygon->count(), "B3DPolygon Append outside range (!)");
                ImplB3DPolygon aTempPoly(*rPoly.mpPolygon, nIndex, nCount);
                mpPolygon->insert(mpPolygon->count(), aTempPoly);
            }
        }
    }

    void B3DPolygon::remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex + nCount <= mpPolygon->count(), "B3DPolygon Remove outside range (!)");

        if(nCount)
        {
            implForceUniqueCopy();
            mpPolygon->remove(nIndex, nCount);
        }
    }

    void B3DPolygon::clear()
    {
        if(mpPolygon->getRefCount())
        {
            mpPolygon->decRefCount();
        }
        else
        {
            delete mpPolygon;
        }

        mpPolygon = &DefaultPolygon::get();
        mpPolygon->incRefCount();
    }

    bool B3DPolygon::isClosed() const
    {
        return mpPolygon->isClosed();
    }

    void B3DPolygon::setClosed(bool bNew)
    {
        if(mpPolygon->isClosed() != bNew)
        {
            implForceUniqueCopy();
            mpPolygon->setClosed(bNew);
        }
    }

    void B3DPolygon::flip()
    {
        if(mpPolygon->count() > 1)
        {
            implForceUniqueCopy();
            mpPolygon->flip();
        }
    }

    bool B3DPolygon::hasDoublePoints() const
    {
        return mpPolygon->hasDoublePoints();
    }

    void B3DPolygon::removeDoublePoints()
    {
        if(mpPolygon->count() > 1)
        {
            implForceUniqueCopy();
            mpPolygon->removeDoublePointsAtBeginEnd();
            mpPolygon->removeDoublePointsWholeTrack();
        }
    }

    void B3DPolygon::transform(const ::basegfx::B3DHomMatrix& rMatrix)
    {
        if(mpPolygon->count())
        {
            implForceUniqueCopy();
            mpPolygon->transform(rMatrix);
        }
    }
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
