/*************************************************************************
 *
 *  $RCSfile: PolygonPoint.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2003-02-27 15:42:05 $
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

#ifndef _POLYGON_POINT_HXX
#define _POLYGON_POINT_HXX

#include <vector>

//////////////////////////////////////////////////////////////////////////////

template < class Point, class Vector > class SimplePolygonPoint
{
    Point                                           maPoint;

public:
    SimplePolygonPoint()
    :   maPoint()
    {
    }

    SimplePolygonPoint(const Point& rInitPoint)
    :   maPoint(rInitPoint)
    {
    }

    const Point& GetPoint() const
    {
        return maPoint;
    }

    void SetPoint(const Point& rValue)
    {
        maPoint = rValue;
    }

    const Vector& GetBackwardVector() const
    {
        return Vector::GetEmptyVector();
    }

    void SetBackwardVector(const Vector& rValue)
    {
        DBG_ASSERT(Vector::GetEmptyVector() == rValue, "Setting backward vector different from zero vector not allowed at SimpleNode (!)");
    }

    const Vector& GetForwardVector() const
    {
        return Vector::GetEmptyVector();
    }

    void SetForwardVector(const Vector& rValue)
    {
        DBG_ASSERT(Vector::GetEmptyVector() == rValue, "Setting forward vector different from zero vector not allowed at SimpleNode (!)");
    }
};

//////////////////////////////////////////////////////////////////////////////

template < class Point, class Vector > class BezierPolygonPoint
{
    Point                                           maPoint;
    Vector                                          maBackward;
    Vector                                          maForward;

public:
    BezierPolygonPoint()
    :   maPoint(),
        maBackward(),
        maForward()
    {
    }

    BezierPolygonPoint(const Point& rInitPoint)
    :   maPoint(rInitPoint),
        maBackward(),
        maForward()
    {
    }

    const Point& GetPoint() const
    {
        return maPoint;
    }

    void SetPoint(const Point& rValue)
    {
        maPoint = rValue;
    }

    const Vector& GetBackwardVector() const
    {
        return maBackward;
    }

    void SetBackwardVector(const Vector& rValue)
    {
        maBackward = rValue;
    }

    const Vector& GetForwardVector() const
    {
        return maForward;
    }

    void SetForwardVector(const Vector& rValue)
    {
        maForward = rValue;
    }

    sal_Bool IsBezierNeeded()
    {
        if(maBackward != Vector::GetEmptyVector() || maForward != Vector::GetEmptyVector())
            return sal_True;
        return sal_False;
    }
};

//////////////////////////////////////////////////////////////////////////////

template < class Point, class Vector > class PolygonPointList
{
    typedef SimplePolygonPoint< Point, Vector > LocalSimplePolygonPoint;
    typedef BezierPolygonPoint< Point, Vector > LocalBezierPolygonPoint;
    typedef ::std::vector< LocalSimplePolygonPoint > SimplePolygonVector;
    typedef ::std::vector< LocalBezierPolygonPoint > BezierPolygonVector;

    sal_uInt32                                      mnCount;
    sal_uInt32                                      mnBezierCount;

    union
    {
        SimplePolygonVector*                        mpSimple;
        BezierPolygonVector*                        mpBezier;
    }
    maList;

    unsigned                                        mbBezierFormat : 1;
    unsigned                                        mbIsClosed : 1;

    void ImplCopyToBezierVector(BezierPolygonVector* pBezier, SimplePolygonVector* pSimple)
    {
        SimplePolygonVector::iterator aSimpleIter(pSimple->begin());
        const SimplePolygonVector::iterator aSimpleEnd(pSimple->end());
        BezierPolygonVector::iterator aBezierIter(pBezier->begin());

        for( ; aSimpleIter != aSimpleEnd; ++aSimpleIter , ++aBezierIter)
        {
            aBezierIter->SetPoint(aSimpleIter->GetPoint());
        }
    }

    void ImplForceToBezier()
    {
        if(!mbBezierFormat)
        {
            SimplePolygonVector* pOldSimple = maList.mpSimple;

            if(mnCount)
            {
                maList.mpBezier = new BezierPolygonVector(mnCount);
                ImplCopyToBezierVector(maList.mpBezier, pOldSimple);
            }

            delete pOldSimple;
            mbBezierFormat = sal_True;
        }
    }

    sal_Bool ImplCompareDifferentFormats(SimplePolygonVector* pSimple, BezierPolygonVector* pBezier) const
    {
        SimplePolygonVector::iterator aSimpleIter(pSimple->begin());
        const SimplePolygonVector::iterator aSimpleEnd(pSimple->end());
        BezierPolygonVector::iterator aBezierIter(pBezier->begin());

        for( ; aSimpleIter != aSimpleEnd; ++aSimpleIter , ++aBezierIter)
        {
            if(aBezierIter->GetPoint() != aSimpleIter->GetPoint())
                return sal_False;
        }

        return sal_True;
    }

    void ImplCopyToSimpleVector(SimplePolygonVector* pSimple, BezierPolygonVector* pBezier)
    {
        BezierPolygonVector::iterator aBezierIter(pBezier->begin());
        const BezierPolygonVector::iterator aBezierEnd(pBezier->end());
        SimplePolygonVector::iterator aSimpleIter(pSimple->begin());

        for( ; aBezierIter != aBezierEnd; ++aBezierIter , ++aSimpleIter)
        {
            aSimpleIter->SetPoint(aBezierIter->GetPoint());
        }
    }

    void ImplTryToChangeToSimple()
    {
        if(mbBezierFormat && !mnBezierCount)
        {
            BezierPolygonVector* pOldBezier = maList.mpBezier;

            if(mnCount)
            {
                maList.mpSimple = new SimplePolygonVector(mnCount);
                ImplCopyToSimpleVector(maList.mpSimple, pOldBezier);
            }

            delete pOldBezier;
            mbBezierFormat = sal_False;
        }
    }

public:
    sal_Bool IsBezier() const
    {
        return sal_Bool(mnBezierCount);
    }

    sal_Bool IsClosed() const
    {
        return sal_Bool(mbIsClosed);
    }

    void SetClosed(sal_Bool bNew)
    {
        mbIsClosed = bNew;
    }

    sal_uInt32 GetPointCount() const
    {
        return mnCount;
    }

    PolygonPointList(sal_Bool bBezier)
    :   mnCount(0L),
        mnBezierCount(0L),
        mbBezierFormat(bBezier),
        mbIsClosed(sal_False)
    {
        // complete initialization with defaults
        maList.mpSimple = 0L;
    }

    PolygonPointList(const PolygonPointList& rSource)
    :   mnCount(rSource.mnCount),
        mnBezierCount(rSource.mnBezierCount),
        mbBezierFormat(rSource.mbBezierFormat),
        mbIsClosed(sal_False)
    {
        // complete initialization using copy
        maList.mpSimple = 0L;

        if(mnCount)
        {
            if(mbBezierFormat)
            {
                if(mnBezierCount)
                {
                    maList.mpBezier = new BezierPolygonVector(*rSource.maList.mpBezier);
                }
                else
                {
                    // here, a reduction at copy time can be done
                    maList.mpSimple = new SimplePolygonVector(mnCount);
                    ImplCopyToSimpleVector(maList.mpSimple, rSource.maList.mpBezier);
                    mbBezierFormat = sal_False;
                }
            }
            else
            {
                maList.mpSimple = new SimplePolygonVector(*rSource.maList.mpSimple);
            }
        }
    }

    PolygonPointList(const PolygonPointList& rSource, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   mnCount(0L),
        mnBezierCount(0L),
        mbBezierFormat(rSource.mbBezierFormat),
        mbIsClosed(sal_False)
    {
        // complete initialization using partly copy
        maList.mpSimple = 0L;

        if(nCount)
        {
            if(rSource.IsBezier())
            {
                maList.mpBezier = new BezierPolygonVector();
                maList.mpBezier->reserve(nCount);

                BezierPolygonVector::iterator aStart(rSource.maList.mpBezier->begin());
                aStart += nIndex;
                BezierPolygonVector::iterator aEnd(aStart);
                aEnd += nCount;

                for( ; aStart != aEnd; ++aStart )
                {
                    if(aStart->IsBezierNeeded())
                    {
                        mnBezierCount++;
                    }

                    maList.mpBezier->push_back(*aStart);
                }

                // maybe 0L == mbBezierCount, try to reduce
                mnCount = nCount;
                ImplTryToChangeToSimple();
            }
            else
            {
                maList.mpSimple = new SimplePolygonVector();
                maList.mpSimple->reserve(nCount);

                SimplePolygonVector::iterator aStart(rSource.maList.mpSimple->begin());
                aStart += nIndex;
                SimplePolygonVector::iterator aEnd(aStart);
                aEnd += nCount;

                maList.mpSimple->insert(0L, aStart, aEnd);
                mnCount = nCount;
            }
        }
    }

    ~PolygonPointList()
    {
        if(mbBezierFormat)
        {
            if(maList.mpBezier)
            {
                delete maList.mpBezier;
            }
        }
        else
        {
            if(maList.mpSimple)
            {
                delete maList.mpSimple;
            }
        }
    }

    sal_Bool IsEqual(const PolygonPointList& rPointList) const
    {
        // same point count?
        if(mnCount != rPointList.mnCount)
            return sal_False;

        // if zero points the polys are equal
        if(!mnCount)
            return sal_True;

        // if bezier count used it needs to be equal
        if(mnBezierCount != rPointList.mnBezierCount)
            return sal_False;

        // compare content if same format
        if(mbBezierFormat && rPointList.mbBezierFormat)
            return (maList.mpBezier == rPointList.maList.mpBezier);

        if(!mbBezierFormat && !rPointList.mbBezierFormat)
            return (maList.mpSimple == rPointList.maList.mpSimple);

        // here we have a combination of bezier and simple. Thus, mnBezierCount
        // needs to be zero, else we have an error here.
        DBG_ASSERT(0L == mnBezierCount, "Error: Bezier count needs to be zero here (!)");

        if(mbBezierFormat)
        {
            // Hint: here would be another chance for reduction, but this method should stay const
            return ImplCompareDifferentFormats(rPointList.maList.mpSimple, maList.mpBezier);
        }
        else
        {
            return ImplCompareDifferentFormats(maList.mpSimple, rPointList.maList.mpBezier);
        }
    }

    const Point& GetPoint(sal_uInt32 nIndex) const
    {
        if(mbBezierFormat)
            return ((*maList.mpBezier)[nIndex]).GetPoint();
        else
            return ((*maList.mpSimple)[nIndex]).GetPoint();
    }

    void SetPoint(sal_uInt32 nIndex, const Point& rValue)
    {
        if(mbBezierFormat)
            ((*maList.mpBezier)[nIndex]).SetPoint(rValue);
        else
            ((*maList.mpSimple)[nIndex]).SetPoint(rValue);
    }

    const Vector& GetBackwardVector(sal_uInt32 nIndex) const
    {
        if(mbBezierFormat)
            return ((*maList.mpBezier)[nIndex]).GetBackwardVector();
        else
            return Vector::GetEmptyVector();
    }

    void SetBackwardVector(sal_uInt32 nIndex, const Vector& rValue)
    {
        if(!mbBezierFormat && rValue != Vector::GetEmptyVector())
            ImplForceToBezier();

        if(mbBezierFormat)
        {
            LocalBezierPolygonPoint& rDest = (*maList.mpBezier)[nIndex];
            sal_Bool bBezierNeededBefore(rDest.IsBezierNeeded());

            rDest.SetBackwardVector(rValue);

            sal_Bool bBezierNeededAfter(rDest.IsBezierNeeded());

            if(bBezierNeededBefore != bBezierNeededAfter)
            {
                if(bBezierNeededAfter)
                    mnBezierCount++;
                else
                    mnBezierCount--;
            }
        }
    }

    const Vector& GetForwardVector(sal_uInt32 nIndex) const
    {
        if(mbBezierFormat)
            return ((*maList.mpBezier)[nIndex]).GetForwardVector();
        else
            return Vector::GetEmptyVector();
    }

    void SetForwardVector(sal_uInt32 nIndex, const Vector& rValue)
    {
        if(!mbBezierFormat && rValue != Vector::GetEmptyVector())
            ImplForceToBezier();

        if(mbBezierFormat)
        {
            LocalBezierPolygonPoint& rDest = (*maList.mpBezier)[nIndex];
            sal_Bool bBezierNeededBefore(rDest.IsBezierNeeded());

            rDest.SetForwardVector(rValue);

            sal_Bool bBezierNeededAfter(rDest.IsBezierNeeded());

            if(bBezierNeededBefore != bBezierNeededAfter)
            {
                if(bBezierNeededAfter)
                    mnBezierCount++;
                else
                    mnBezierCount--;
            }
        }
    }

    void Insert(sal_uInt32 nIndex, const Point& rPoint, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // before inserting, eventually reduce memory usage
            ImplTryToChangeToSimple();

            if(mbBezierFormat)
            {
                LocalBezierPolygonPoint aNode(rPoint);

                if(!maList.mpBezier)
                {
                    maList.mpBezier = new BezierPolygonVector(mnCount, aNode);
                }
                else
                {
                    BezierPolygonVector::iterator aIndex(maList.mpBezier->begin());
                    aIndex += nIndex;
                    maList.mpBezier->insert(aIndex, nCount, aNode);
                }
            }
            else
            {
                LocalSimplePolygonPoint aNode(rPoint);

                if(!maList.mpSimple)
                {
                    maList.mpSimple = new SimplePolygonVector(mnCount, aNode);
                }
                else
                {
                    SimplePolygonVector::iterator aIndex(maList.mpSimple->begin());
                    aIndex += nIndex;
                    maList.mpSimple->insert(aIndex, nCount, aNode);
                }
            }

            mnCount += nCount;
        }
    }

    void Insert(sal_uInt32 nIndex, const PolygonPointList& rSource)
    {
        const sal_uInt32 nCount(rSource.mnCount);

        if(nCount)
        {
            if(rSource.IsBezier())
            {
                ImplForceToBezier();

                if(!maList.mpBezier)
                {
                    // copy other bezier
                    maList.mpBezier = new BezierPolygonVector(*rSource.maList.mpBezier);
                    mnBezierCount = rSource.mnBezierCount;
                }
                else
                {
                    // insert bezier into bezier
                    BezierPolygonVector::iterator aIndex(maList.mpBezier->begin());
                    aIndex += nIndex;

                    BezierPolygonVector::iterator aStart(rSource.maList.mpBezier->begin());
                    BezierPolygonVector::iterator aEnd(rSource.maList.mpBezier->end());

                    maList.mpBezier->insert(aIndex, aStart, aEnd);
                    mnBezierCount += rSource.mnBezierCount;
                }
            }
            else
            {
                // before inserting, eventually reduce memory usage
                ImplTryToChangeToSimple();

                if(mnBezierCount)
                {
                    // local is still bezier, source is simple. Insert
                    // simple into bezier.
                    LocalBezierPolygonPoint aNode(Point::GetEmptyPoint());
                    BezierPolygonVector::iterator aIndex(maList.mpBezier->begin());
                    aIndex += nIndex;

                    // insert nCount empty elements
                    maList.mpBezier->insert(aIndex, nCount, aNode);

                    // copy coordinate data to new locations
                    SimplePolygonVector::iterator aSimpleIter(rSource.maList.mpSimple->begin());
                    const SimplePolygonVector::iterator aSimpleEnd(rSource.maList.mpSimple->end());

                    for( ; aSimpleIter != aSimpleEnd; ++aSimpleIter , ++aIndex)
                    {
                        aIndex->SetPoint(aSimpleIter->GetPoint());
                    }
                }
                else
                {
                    // insert simple into simple
                    if(!maList.mpSimple)
                    {
                        maList.mpSimple = new SimplePolygonVector(*rSource.maList.mpSimple);
                    }
                    else
                    {
                        SimplePolygonVector::iterator aIndex(maList.mpSimple->begin());
                        aIndex += nIndex;

                        SimplePolygonVector::iterator aStart(rSource.maList.mpSimple->begin());
                        SimplePolygonVector::iterator aEnd(rSource.maList.mpSimple->end());

                        maList.mpSimple->insert(aIndex, aStart, aEnd);
                    }
                }
            }

            mnCount += nCount;
        }
    }

    void Remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(nCount)
        {
            if(mbBezierFormat)
            {
                BezierPolygonVector::iterator aStart(maList.mpBezier->begin());
                aStart += nIndex;
                const BezierPolygonVector::iterator aEnd(aStart + nCount);

                // take care for correct mnBezierCount BEFORE erase
                if(mnBezierCount)
                {
                    BezierPolygonVector::iterator aTestIter(aStart);

                    for( ; mnBezierCount && aTestIter != aEnd; ++aTestIter)
                    {
                        if(aTestIter->IsBezierNeeded())
                            mnBezierCount--;
                    }
                }

                // erase nodes
                maList.mpBezier->erase(aStart, aEnd);

                // try to reduce, maybe 0L == mnBezierCount
                ImplTryToChangeToSimple();
            }
            else
            {
                SimplePolygonVector::iterator aStart(maList.mpSimple->begin());
                aStart += nIndex;
                const SimplePolygonVector::iterator aEnd(aStart + nCount);

                maList.mpSimple->erase(aStart, aEnd);
            }

            mnCount -= nCount;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

#endif _POLYGON_POINT_HXX
