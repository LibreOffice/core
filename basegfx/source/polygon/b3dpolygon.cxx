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

#include <osl/diagnose.h>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <rtl/instance.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <vector>
#include <algorithm>

class CoordinateData3D
{
    basegfx::B3DPoint                               maPoint;

public:
    CoordinateData3D()
    :   maPoint()
    {
    }

    explicit CoordinateData3D(const basegfx::B3DPoint& rData)
    :   maPoint(rData)
    {
    }

    const basegfx::B3DPoint& getCoordinate() const
    {
        return maPoint;
    }

    void setCoordinate(const basegfx::B3DPoint& rValue)
    {
        if(rValue != maPoint)
            maPoint = rValue;
    }

    bool operator==(const CoordinateData3D& rData) const
    {
        return (maPoint == rData.getCoordinate());
    }

    void transform(const basegfx::B3DHomMatrix& rMatrix)
    {
        maPoint *= rMatrix;
    }
};

class CoordinateDataArray3D
{
    typedef ::std::vector< CoordinateData3D > CoordinateData3DVector;

    CoordinateData3DVector                          maVector;

public:
    explicit CoordinateDataArray3D(sal_uInt32 nCount)
    :   maVector(nCount)
    {
    }

    explicit CoordinateDataArray3D(const CoordinateDataArray3D& rOriginal)
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

    ::basegfx::B3DVector getNormal() const
    {
        ::basegfx::B3DVector aRetval;
        const sal_uInt32 nPointCount(maVector.size());

        if(nPointCount > 2)
        {
            sal_uInt32 nISmallest(0);
            sal_uInt32 a(0);
            const basegfx::B3DPoint* pSmallest(&maVector[0].getCoordinate());
            const basegfx::B3DPoint* pNext(nullptr);
            const basegfx::B3DPoint* pPrev(nullptr);

            // To guarantee a correctly oriented point, choose an outmost one
            // which then cannot be concave
            for(a = 1; a < nPointCount; a++)
            {
                const basegfx::B3DPoint& rCandidate = maVector[a].getCoordinate();

                if((rCandidate.getX() < pSmallest->getX())
                    || (rCandidate.getX() == pSmallest->getX() && rCandidate.getY() < pSmallest->getY())
                    || (rCandidate.getX() == pSmallest->getX() && rCandidate.getY() == pSmallest->getY() && rCandidate.getZ() < pSmallest->getZ()))
                {
                    nISmallest = a;
                    pSmallest = &rCandidate;
                }
            }

            // look for a next point different from minimal one
            for(a = (nISmallest + 1) % nPointCount; a != nISmallest; a = (a + 1) % nPointCount)
            {
                const basegfx::B3DPoint& rCandidate = maVector[a].getCoordinate();

                if(!rCandidate.equal(*pSmallest))
                {
                    pNext = &rCandidate;
                    break;
                }
            }

            // look for a previous point different from minimal one
            for(a = (nISmallest + nPointCount - 1) % nPointCount; a != nISmallest; a = (a + nPointCount - 1) % nPointCount)
            {
                const basegfx::B3DPoint& rCandidate = maVector[a].getCoordinate();

                if(!rCandidate.equal(*pSmallest))
                {
                    pPrev = &rCandidate;
                    break;
                }
            }

            // we always have a minimal point. If we also have a different next and previous,
            // we can calculate the normal
            if(pNext && pPrev)
            {
                const basegfx::B3DVector aPrev(*pPrev - *pSmallest);
                const basegfx::B3DVector aNext(*pNext - *pSmallest);

                aRetval = cross(aPrev, aNext);
                aRetval.normalize();
            }
        }

        return aRetval;
    }

    sal_uInt32 count() const
    {
        return maVector.size();
    }

    bool operator==(const CoordinateDataArray3D& rCandidate) const
    {
        return (maVector == rCandidate.maVector);
    }

    const basegfx::B3DPoint& getCoordinate(sal_uInt32 nIndex) const
    {
        return maVector[nIndex].getCoordinate();
    }

    void setCoordinate(sal_uInt32 nIndex, const basegfx::B3DPoint& rValue)
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
                ++aStart;
                --aEnd;
            }
        }
    }

    void transform(const ::basegfx::B3DHomMatrix& rMatrix)
    {
        CoordinateData3DVector::iterator aStart(maVector.begin());
        CoordinateData3DVector::iterator aEnd(maVector.end());

        for(; aStart != aEnd; ++aStart)
        {
            aStart->transform(rMatrix);
        }
    }
};

class BColorArray
{
    typedef ::std::vector< ::basegfx::BColor > BColorDataVector;

    BColorDataVector                                    maVector;
    sal_uInt32                                          mnUsedEntries;

public:
    explicit BColorArray(sal_uInt32 nCount)
    :   maVector(nCount),
        mnUsedEntries(0)
    {
    }

    explicit BColorArray(const BColorArray& rOriginal)
    :   maVector(rOriginal.maVector),
        mnUsedEntries(rOriginal.mnUsedEntries)
    {
    }

    BColorArray(const BColorArray& rOriginal, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   maVector(),
        mnUsedEntries(0)
    {
        BColorDataVector::const_iterator aStart(rOriginal.maVector.begin());
        aStart += nIndex;
        BColorDataVector::const_iterator aEnd(aStart);
        assert(nCount <= rOriginal.maVector.size());
        aEnd += nCount;
        maVector.reserve(nCount);

        for(; aStart != aEnd; ++aStart)
        {
            if(!aStart->equalZero())
                mnUsedEntries++;

            maVector.push_back(*aStart);
        }
    }

    ~BColorArray()
    {
    }

    bool operator==(const BColorArray& rCandidate) const
    {
        return (maVector == rCandidate.maVector);
    }

    bool isUsed() const
    {
        return (0 != mnUsedEntries);
    }

    const ::basegfx::BColor& getBColor(sal_uInt32 nIndex) const
    {
        return maVector[nIndex];
    }

    void setBColor(sal_uInt32 nIndex, const ::basegfx::BColor& rValue)
    {
        bool bWasUsed(mnUsedEntries && !maVector[nIndex].equalZero());
        bool bIsUsed(!rValue.equalZero());

        if(bWasUsed)
        {
            if(bIsUsed)
            {
                maVector[nIndex] = rValue;
            }
            else
            {
                maVector[nIndex] = ::basegfx::BColor::getEmptyBColor();
                mnUsedEntries--;
            }
        }
        else
        {
            if(bIsUsed)
            {
                maVector[nIndex] = rValue;
                mnUsedEntries++;
            }
        }
    }

    void insert(sal_uInt32 nIndex, const ::basegfx::BColor& rValue, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // add nCount copies of rValue
            BColorDataVector::iterator aIndex(maVector.begin());
            aIndex += nIndex;
            maVector.insert(aIndex, nCount, rValue);

            if(!rValue.equalZero())
                mnUsedEntries += nCount;
        }
    }

    void insert(sal_uInt32 nIndex, const BColorArray& rSource)
    {
        const sal_uInt32 nCount(rSource.maVector.size());

        if(nCount)
        {
            // insert data
            BColorDataVector::iterator aIndex(maVector.begin());
            aIndex += nIndex;
            BColorDataVector::const_iterator aStart(rSource.maVector.begin());
            BColorDataVector::const_iterator aEnd(rSource.maVector.end());
            maVector.insert(aIndex, aStart, aEnd);

            for(; aStart != aEnd; ++aStart)
            {
                if(!aStart->equalZero())
                    mnUsedEntries++;
            }
        }
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(nCount)
        {
            const BColorDataVector::iterator aDeleteStart(maVector.begin() + nIndex);
            const BColorDataVector::iterator aDeleteEnd(aDeleteStart + nCount);
            BColorDataVector::const_iterator aStart(aDeleteStart);

            for(; mnUsedEntries && aStart != aDeleteEnd; ++aStart)
            {
                if(!aStart->equalZero())
                    mnUsedEntries--;
            }

            // remove point data
            maVector.erase(aDeleteStart, aDeleteEnd);
        }
    }

    void flip()
    {
        if(maVector.size() > 1)
        {
            const sal_uInt32 nHalfSize(maVector.size() >> 1L);
            BColorDataVector::iterator aStart(maVector.begin());
            BColorDataVector::iterator aEnd(maVector.end() - 1L);

            for(sal_uInt32 a(0); a < nHalfSize; a++)
            {
                ::std::swap(*aStart, *aEnd);
                ++aStart;
                --aEnd;
            }
        }
    }
};

class NormalsArray3D
{
    typedef ::std::vector< ::basegfx::B3DVector > NormalsData3DVector;

    NormalsData3DVector                                 maVector;
    sal_uInt32                                          mnUsedEntries;

public:
    explicit NormalsArray3D(sal_uInt32 nCount)
    :   maVector(nCount),
        mnUsedEntries(0)
    {
    }

    explicit NormalsArray3D(const NormalsArray3D& rOriginal)
    :   maVector(rOriginal.maVector),
        mnUsedEntries(rOriginal.mnUsedEntries)
    {
    }

    NormalsArray3D(const NormalsArray3D& rOriginal, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   maVector(),
        mnUsedEntries(0)
    {
        NormalsData3DVector::const_iterator aStart(rOriginal.maVector.begin());
        aStart += nIndex;
        NormalsData3DVector::const_iterator aEnd(aStart);
        aEnd += nCount;
        maVector.reserve(nCount);

        for(; aStart != aEnd; ++aStart)
        {
            if(!aStart->equalZero())
                mnUsedEntries++;

            maVector.push_back(*aStart);
        }
    }

    ~NormalsArray3D()
    {
    }

    bool operator==(const NormalsArray3D& rCandidate) const
    {
        return (maVector == rCandidate.maVector);
    }

    bool isUsed() const
    {
        return (0 != mnUsedEntries);
    }

    const ::basegfx::B3DVector& getNormal(sal_uInt32 nIndex) const
    {
        return maVector[nIndex];
    }

    void setNormal(sal_uInt32 nIndex, const ::basegfx::B3DVector& rValue)
    {
        bool bWasUsed(mnUsedEntries && !maVector[nIndex].equalZero());
        bool bIsUsed(!rValue.equalZero());

        if(bWasUsed)
        {
            if(bIsUsed)
            {
                maVector[nIndex] = rValue;
            }
            else
            {
                maVector[nIndex] = ::basegfx::B3DVector::getEmptyVector();
                mnUsedEntries--;
            }
        }
        else
        {
            if(bIsUsed)
            {
                maVector[nIndex] = rValue;
                mnUsedEntries++;
            }
        }
    }

    void insert(sal_uInt32 nIndex, const ::basegfx::B3DVector& rValue, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // add nCount copies of rValue
            NormalsData3DVector::iterator aIndex(maVector.begin());
            aIndex += nIndex;
            maVector.insert(aIndex, nCount, rValue);

            if(!rValue.equalZero())
                mnUsedEntries += nCount;
        }
    }

    void insert(sal_uInt32 nIndex, const NormalsArray3D& rSource)
    {
        const sal_uInt32 nCount(rSource.maVector.size());

        if(nCount)
        {
            // insert data
            NormalsData3DVector::iterator aIndex(maVector.begin());
            aIndex += nIndex;
            NormalsData3DVector::const_iterator aStart(rSource.maVector.begin());
            NormalsData3DVector::const_iterator aEnd(rSource.maVector.end());
            maVector.insert(aIndex, aStart, aEnd);

            for(; aStart != aEnd; ++aStart)
            {
                if(!aStart->equalZero())
                    mnUsedEntries++;
            }
        }
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(nCount)
        {
            const NormalsData3DVector::iterator aDeleteStart(maVector.begin() + nIndex);
            const NormalsData3DVector::iterator aDeleteEnd(aDeleteStart + nCount);
            NormalsData3DVector::const_iterator aStart(aDeleteStart);

            for(; mnUsedEntries && aStart != aDeleteEnd; ++aStart)
            {
                if(!aStart->equalZero())
                    mnUsedEntries--;
            }

            // remove point data
            maVector.erase(aDeleteStart, aDeleteEnd);
        }
    }

    void flip()
    {
        if(maVector.size() > 1)
        {
            const sal_uInt32 nHalfSize(maVector.size() >> 1L);
            NormalsData3DVector::iterator aStart(maVector.begin());
            NormalsData3DVector::iterator aEnd(maVector.end() - 1L);

            for(sal_uInt32 a(0); a < nHalfSize; a++)
            {
                ::std::swap(*aStart, *aEnd);
                ++aStart;
                --aEnd;
            }
        }
    }

    void transform(const basegfx::B3DHomMatrix& rMatrix)
    {
        const NormalsData3DVector::const_iterator aEnd(maVector.end());

        for(NormalsData3DVector::iterator aStart(maVector.begin()); aStart != aEnd; ++aStart)
        {
            (*aStart) *= rMatrix;
        }
    }
};

class TextureCoordinate2D
{
    typedef ::std::vector< ::basegfx::B2DPoint > TextureData2DVector;

    TextureData2DVector                                 maVector;
    sal_uInt32                                          mnUsedEntries;

public:
    explicit TextureCoordinate2D(sal_uInt32 nCount)
    :   maVector(nCount),
        mnUsedEntries(0)
    {
    }

    explicit TextureCoordinate2D(const TextureCoordinate2D& rOriginal)
    :   maVector(rOriginal.maVector),
        mnUsedEntries(rOriginal.mnUsedEntries)
    {
    }

    TextureCoordinate2D(const TextureCoordinate2D& rOriginal, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   maVector(),
        mnUsedEntries(0)
    {
        TextureData2DVector::const_iterator aStart(rOriginal.maVector.begin());
        aStart += nIndex;
        TextureData2DVector::const_iterator aEnd(aStart);
        aEnd += nCount;
        maVector.reserve(nCount);

        for(; aStart != aEnd; ++aStart)
        {
            if(!aStart->equalZero())
                mnUsedEntries++;

            maVector.push_back(*aStart);
        }
    }

    ~TextureCoordinate2D()
    {
    }

    bool operator==(const TextureCoordinate2D& rCandidate) const
    {
        return (maVector == rCandidate.maVector);
    }

    bool isUsed() const
    {
        return (0 != mnUsedEntries);
    }

    const ::basegfx::B2DPoint& getTextureCoordinate(sal_uInt32 nIndex) const
    {
        return maVector[nIndex];
    }

    void setTextureCoordinate(sal_uInt32 nIndex, const ::basegfx::B2DPoint& rValue)
    {
        bool bWasUsed(mnUsedEntries && !maVector[nIndex].equalZero());
        bool bIsUsed(!rValue.equalZero());

        if(bWasUsed)
        {
            if(bIsUsed)
            {
                maVector[nIndex] = rValue;
            }
            else
            {
                maVector[nIndex] = ::basegfx::B2DPoint::getEmptyPoint();
                mnUsedEntries--;
            }
        }
        else
        {
            if(bIsUsed)
            {
                maVector[nIndex] = rValue;
                mnUsedEntries++;
            }
        }
    }

    void insert(sal_uInt32 nIndex, const ::basegfx::B2DPoint& rValue, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // add nCount copies of rValue
            TextureData2DVector::iterator aIndex(maVector.begin());
            aIndex += nIndex;
            maVector.insert(aIndex, nCount, rValue);

            if(!rValue.equalZero())
                mnUsedEntries += nCount;
        }
    }

    void insert(sal_uInt32 nIndex, const TextureCoordinate2D& rSource)
    {
        const sal_uInt32 nCount(rSource.maVector.size());

        if(nCount)
        {
            // insert data
            TextureData2DVector::iterator aIndex(maVector.begin());
            aIndex += nIndex;
            TextureData2DVector::const_iterator aStart(rSource.maVector.begin());
            TextureData2DVector::const_iterator aEnd(rSource.maVector.end());
            maVector.insert(aIndex, aStart, aEnd);

            for(; aStart != aEnd; ++aStart)
            {
                if(!aStart->equalZero())
                    mnUsedEntries++;
            }
        }
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(nCount)
        {
            const TextureData2DVector::iterator aDeleteStart(maVector.begin() + nIndex);
            const TextureData2DVector::iterator aDeleteEnd(aDeleteStart + nCount);
            TextureData2DVector::const_iterator aStart(aDeleteStart);

            for(; mnUsedEntries && aStart != aDeleteEnd; ++aStart)
            {
                if(!aStart->equalZero())
                    mnUsedEntries--;
            }

            // remove point data
            maVector.erase(aDeleteStart, aDeleteEnd);
        }
    }

    void flip()
    {
        if(maVector.size() > 1)
        {
            const sal_uInt32 nHalfSize(maVector.size() >> 1L);
            TextureData2DVector::iterator aStart(maVector.begin());
            TextureData2DVector::iterator aEnd(maVector.end() - 1L);

            for(sal_uInt32 a(0); a < nHalfSize; a++)
            {
                ::std::swap(*aStart, *aEnd);
                ++aStart;
                --aEnd;
            }
        }
    }

    void transform(const ::basegfx::B2DHomMatrix& rMatrix)
    {
        const TextureData2DVector::const_iterator aEnd(maVector.end());

        for(TextureData2DVector::iterator aStart(maVector.begin()); aStart != aEnd; ++aStart)
        {
            (*aStart) *= rMatrix;
        }
    }
};

class ImplB3DPolygon
{
    // The point vector. This vector exists always and defines the
    // count of members.
    CoordinateDataArray3D                           maPoints;

    // The BColor vector. This vectors are created on demand
    // and may be zero.
    BColorArray*                                    mpBColors;

    // The Normals vector. This vectors are created on demand
    // and may be zero.
    NormalsArray3D*                                 mpNormals;

    // The TextureCoordinates vector. This vectors are created on demand
    // and may be zero.
    TextureCoordinate2D*                            mpTextureCoordinates;

    // The calculated plane normal. mbPlaneNormalValid says if it's valid.
    ::basegfx::B3DVector                            maPlaneNormal;

    // bitfield
    // flag which decides if this polygon is opened or closed
    bool                                            mbIsClosed : 1;

    // flag which says if maPlaneNormal is up-to-date
    bool                                            mbPlaneNormalValid : 1;

protected:
    void invalidatePlaneNormal()
    {
        if(mbPlaneNormalValid)
        {
            mbPlaneNormalValid = false;
        }
    }

public:
    // This constructor is only used from the static identity polygon, thus
    // the RefCount is set to 1 to never 'delete' this static incarnation.
    ImplB3DPolygon()
    :   maPoints(0),
        mpBColors(nullptr),
        mpNormals(nullptr),
        mpTextureCoordinates(nullptr),
        maPlaneNormal(::basegfx::B3DVector::getEmptyVector()),
        mbIsClosed(false),
        mbPlaneNormalValid(true)
    {
        // complete initialization with defaults
    }

    ImplB3DPolygon(const ImplB3DPolygon& rToBeCopied)
    :   maPoints(rToBeCopied.maPoints),
        mpBColors(nullptr),
        mpNormals(nullptr),
        mpTextureCoordinates(nullptr),
        maPlaneNormal(rToBeCopied.maPlaneNormal),
        mbIsClosed(rToBeCopied.mbIsClosed),
        mbPlaneNormalValid(rToBeCopied.mbPlaneNormalValid)
    {
        // complete initialization using copy
        if(rToBeCopied.mpBColors && rToBeCopied.mpBColors->isUsed())
        {
            mpBColors = new BColorArray(*rToBeCopied.mpBColors);
        }

        if(rToBeCopied.mpNormals && rToBeCopied.mpNormals->isUsed())
        {
            mpNormals = new NormalsArray3D(*rToBeCopied.mpNormals);
        }

        if(rToBeCopied.mpTextureCoordinates && rToBeCopied.mpTextureCoordinates->isUsed())
        {
            mpTextureCoordinates = new TextureCoordinate2D(*rToBeCopied.mpTextureCoordinates);
        }
    }

    ImplB3DPolygon(const ImplB3DPolygon& rToBeCopied, sal_uInt32 nIndex, sal_uInt32 nCount)
    :   maPoints(rToBeCopied.maPoints, nIndex, nCount),
        mpBColors(nullptr),
        mpNormals(nullptr),
        mpTextureCoordinates(nullptr),
        maPlaneNormal(::basegfx::B3DVector::getEmptyVector()),
        mbIsClosed(rToBeCopied.mbIsClosed),
        mbPlaneNormalValid(false)
    {
        // complete initialization using partly copy
        if(rToBeCopied.mpBColors && rToBeCopied.mpBColors->isUsed())
        {
            mpBColors = new BColorArray(*rToBeCopied.mpBColors, nIndex, nCount);

            if(!mpBColors->isUsed())
            {
                delete mpBColors;
                mpBColors = nullptr;
            }
        }

        if(rToBeCopied.mpNormals && rToBeCopied.mpNormals->isUsed())
        {
            mpNormals = new NormalsArray3D(*rToBeCopied.mpNormals, nIndex, nCount);

            if(!mpNormals->isUsed())
            {
                delete mpNormals;
                mpNormals = nullptr;
            }
        }

        if(rToBeCopied.mpTextureCoordinates && rToBeCopied.mpTextureCoordinates->isUsed())
        {
            mpTextureCoordinates = new TextureCoordinate2D(*rToBeCopied.mpTextureCoordinates, nIndex, nCount);

            if(!mpTextureCoordinates->isUsed())
            {
                delete mpTextureCoordinates;
                mpTextureCoordinates = nullptr;
            }
        }
    }

    ~ImplB3DPolygon()
    {
        if(mpBColors)
        {
            delete mpBColors;
            mpBColors = nullptr;
        }

        if(mpNormals)
        {
            delete mpNormals;
            mpNormals = nullptr;
        }

        if(mpTextureCoordinates)
        {
            delete mpTextureCoordinates;
            mpTextureCoordinates = nullptr;
        }
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
        if(bNew != (bool)mbIsClosed)
        {
            mbIsClosed = bNew;
        }
    }

    inline bool impBColorsAreEqual(const ImplB3DPolygon& rCandidate) const
    {
        bool bBColorsAreEqual(true);

        if(mpBColors)
        {
            if(rCandidate.mpBColors)
            {
                bBColorsAreEqual = (*mpBColors == *rCandidate.mpBColors);
            }
            else
            {
                // candidate has no BColors, so it's assumed all unused.
                bBColorsAreEqual = !mpBColors->isUsed();
            }
        }
        else
        {
            if(rCandidate.mpBColors)
            {
                // we have no TextureCoordinates, so it's assumed all unused.
                bBColorsAreEqual = !rCandidate.mpBColors->isUsed();
            }
        }

        return bBColorsAreEqual;
    }

    inline bool impNormalsAreEqual(const ImplB3DPolygon& rCandidate) const
    {
        bool bNormalsAreEqual(true);

        if(mpNormals)
        {
            if(rCandidate.mpNormals)
            {
                bNormalsAreEqual = (*mpNormals == *rCandidate.mpNormals);
            }
            else
            {
                // candidate has no normals, so it's assumed all unused.
                bNormalsAreEqual = !mpNormals->isUsed();
            }
        }
        else
        {
            if(rCandidate.mpNormals)
            {
                // we have no normals, so it's assumed all unused.
                bNormalsAreEqual = !rCandidate.mpNormals->isUsed();
            }
        }

        return bNormalsAreEqual;
    }

    inline bool impTextureCoordinatesAreEqual(const ImplB3DPolygon& rCandidate) const
    {
        bool bTextureCoordinatesAreEqual(true);

        if(mpTextureCoordinates)
        {
            if(rCandidate.mpTextureCoordinates)
            {
                bTextureCoordinatesAreEqual = (*mpTextureCoordinates == *rCandidate.mpTextureCoordinates);
            }
            else
            {
                // candidate has no TextureCoordinates, so it's assumed all unused.
                bTextureCoordinatesAreEqual = !mpTextureCoordinates->isUsed();
            }
        }
        else
        {
            if(rCandidate.mpTextureCoordinates)
            {
                // we have no TextureCoordinates, so it's assumed all unused.
                bTextureCoordinatesAreEqual = !rCandidate.mpTextureCoordinates->isUsed();
            }
        }

        return bTextureCoordinatesAreEqual;
    }

    bool operator==(const ImplB3DPolygon& rCandidate) const
    {
        if(mbIsClosed == rCandidate.mbIsClosed)
        {
            if(maPoints == rCandidate.maPoints)
            {
                if(impBColorsAreEqual(rCandidate))
                {
                    if(impNormalsAreEqual(rCandidate))
                    {
                        if(impTextureCoordinatesAreEqual(rCandidate))
                        {
                            return true;
                        }
                    }
                }
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
        invalidatePlaneNormal();
    }

    void insert(sal_uInt32 nIndex, const ::basegfx::B3DPoint& rPoint, sal_uInt32 nCount)
    {
        if(nCount)
        {
            CoordinateData3D aCoordinate(rPoint);
            maPoints.insert(nIndex, aCoordinate, nCount);
            invalidatePlaneNormal();

            if(mpBColors)
            {
                mpBColors->insert(nIndex, ::basegfx::BColor::getEmptyBColor(), nCount);
            }

            if(mpNormals)
            {
                mpNormals->insert(nIndex, ::basegfx::B3DVector::getEmptyVector(), nCount);
            }

            if(mpTextureCoordinates)
            {
                mpTextureCoordinates->insert(nIndex, ::basegfx::B2DPoint::getEmptyPoint(), nCount);
            }
        }
    }

    const ::basegfx::BColor& getBColor(sal_uInt32 nIndex) const
    {
        if(mpBColors)
        {
            return mpBColors->getBColor(nIndex);
        }
        else
        {
            return ::basegfx::BColor::getEmptyBColor();
        }
    }

    void setBColor(sal_uInt32 nIndex, const ::basegfx::BColor& rValue)
    {
        if(!mpBColors)
        {
            if(!rValue.equalZero())
            {
                mpBColors = new BColorArray(maPoints.count());
                mpBColors->setBColor(nIndex, rValue);
            }
        }
        else
        {
            mpBColors->setBColor(nIndex, rValue);

            if(!mpBColors->isUsed())
            {
                delete mpBColors;
                mpBColors = nullptr;
            }
        }
    }

    bool areBColorsUsed() const
    {
        return (mpBColors && mpBColors->isUsed());
    }

    void clearBColors()
    {
        if(mpBColors)
        {
            delete mpBColors;
            mpBColors = nullptr;
        }
    }

    const ::basegfx::B3DVector& getNormal() const
    {
        if(!mbPlaneNormalValid)
        {
            const_cast< ImplB3DPolygon* >(this)->maPlaneNormal = maPoints.getNormal();
            const_cast< ImplB3DPolygon* >(this)->mbPlaneNormalValid = true;
        }

        return maPlaneNormal;
    }

    const ::basegfx::B3DVector& getNormal(sal_uInt32 nIndex) const
    {
        if(mpNormals)
        {
            return mpNormals->getNormal(nIndex);
        }
        else
        {
            return ::basegfx::B3DVector::getEmptyVector();
        }
    }

    void setNormal(sal_uInt32 nIndex, const ::basegfx::B3DVector& rValue)
    {
        if(!mpNormals)
        {
            if(!rValue.equalZero())
            {
                mpNormals = new NormalsArray3D(maPoints.count());
                mpNormals->setNormal(nIndex, rValue);
            }
        }
        else
        {
            mpNormals->setNormal(nIndex, rValue);

            if(!mpNormals->isUsed())
            {
                delete mpNormals;
                mpNormals = nullptr;
            }
        }
    }

    void transformNormals(const ::basegfx::B3DHomMatrix& rMatrix)
    {
        if(mpNormals)
        {
            mpNormals->transform(rMatrix);
        }
    }

    bool areNormalsUsed() const
    {
        return (mpNormals && mpNormals->isUsed());
    }

    void clearNormals()
    {
        if(mpNormals)
        {
            delete mpNormals;
            mpNormals = nullptr;
        }
    }

    const ::basegfx::B2DPoint& getTextureCoordinate(sal_uInt32 nIndex) const
    {
        if(mpTextureCoordinates)
        {
            return mpTextureCoordinates->getTextureCoordinate(nIndex);
        }
        else
        {
            return ::basegfx::B2DPoint::getEmptyPoint();
        }
    }

    void setTextureCoordinate(sal_uInt32 nIndex, const ::basegfx::B2DPoint& rValue)
    {
        if(!mpTextureCoordinates)
        {
            if(!rValue.equalZero())
            {
                mpTextureCoordinates = new TextureCoordinate2D(maPoints.count());
                mpTextureCoordinates->setTextureCoordinate(nIndex, rValue);
            }
        }
        else
        {
            mpTextureCoordinates->setTextureCoordinate(nIndex, rValue);

            if(!mpTextureCoordinates->isUsed())
            {
                delete mpTextureCoordinates;
                mpTextureCoordinates = nullptr;
            }
        }
    }

    bool areTextureCoordinatesUsed() const
    {
        return (mpTextureCoordinates && mpTextureCoordinates->isUsed());
    }

    void clearTextureCoordinates()
    {
        if(mpTextureCoordinates)
        {
            delete mpTextureCoordinates;
            mpTextureCoordinates = nullptr;
        }
    }

    void transformTextureCoordinates(const ::basegfx::B2DHomMatrix& rMatrix)
    {
        if(mpTextureCoordinates)
        {
            mpTextureCoordinates->transform(rMatrix);
        }
    }

    void insert(sal_uInt32 nIndex, const ImplB3DPolygon& rSource)
    {
        const sal_uInt32 nCount(rSource.maPoints.count());

        if(nCount)
        {
            maPoints.insert(nIndex, rSource.maPoints);
            invalidatePlaneNormal();

            if(rSource.mpBColors && rSource.mpBColors->isUsed())
            {
                if(!mpBColors)
                {
                    mpBColors = new BColorArray(maPoints.count());
                }

                mpBColors->insert(nIndex, *rSource.mpBColors);
            }
            else
            {
                if(mpBColors)
                {
                    mpBColors->insert(nIndex, ::basegfx::BColor::getEmptyBColor(), nCount);
                }
            }

            if(rSource.mpNormals && rSource.mpNormals->isUsed())
            {
                if(!mpNormals)
                {
                    mpNormals = new NormalsArray3D(maPoints.count());
                }

                mpNormals->insert(nIndex, *rSource.mpNormals);
            }
            else
            {
                if(mpNormals)
                {
                    mpNormals->insert(nIndex, ::basegfx::B3DVector::getEmptyVector(), nCount);
                }
            }

            if(rSource.mpTextureCoordinates && rSource.mpTextureCoordinates->isUsed())
            {
                if(!mpTextureCoordinates)
                {
                    mpTextureCoordinates = new TextureCoordinate2D(maPoints.count());
                }

                mpTextureCoordinates->insert(nIndex, *rSource.mpTextureCoordinates);
            }
            else
            {
                if(mpTextureCoordinates)
                {
                    mpTextureCoordinates->insert(nIndex, ::basegfx::B2DPoint::getEmptyPoint(), nCount);
                }
            }
        }
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(nCount)
        {
            maPoints.remove(nIndex, nCount);
            invalidatePlaneNormal();

            if(mpBColors)
            {
                mpBColors->remove(nIndex, nCount);

                if(!mpBColors->isUsed())
                {
                    delete mpBColors;
                    mpBColors = nullptr;
                }
            }

            if(mpNormals)
            {
                mpNormals->remove(nIndex, nCount);

                if(!mpNormals->isUsed())
                {
                    delete mpNormals;
                    mpNormals = nullptr;
                }
            }

            if(mpTextureCoordinates)
            {
                mpTextureCoordinates->remove(nIndex, nCount);

                if(!mpTextureCoordinates->isUsed())
                {
                    delete mpTextureCoordinates;
                    mpTextureCoordinates = nullptr;
                }
            }
        }
    }

    void flip()
    {
        if(maPoints.count() > 1)
        {
            maPoints.flip();

            if(mbPlaneNormalValid)
            {
                // mirror plane normal
                maPlaneNormal = -maPlaneNormal;
            }

            if(mpBColors)
            {
                mpBColors->flip();
            }

            if(mpNormals)
            {
                mpNormals->flip();
            }

            if(mpTextureCoordinates)
            {
                mpTextureCoordinates->flip();
            }
        }
    }

    bool hasDoublePoints() const
    {
        if(mbIsClosed)
        {
            // check for same start and end point
            const sal_uInt32 nIndex(maPoints.count() - 1L);

            if(maPoints.getCoordinate(0) == maPoints.getCoordinate(nIndex))
            {
                const bool bBColorEqual(!mpBColors || (mpBColors->getBColor(0) == mpBColors->getBColor(nIndex)));

                if(bBColorEqual)
                {
                    const bool bNormalsEqual(!mpNormals || (mpNormals->getNormal(0) == mpNormals->getNormal(nIndex)));

                    if(bNormalsEqual)
                    {
                        const bool bTextureCoordinatesEqual(!mpTextureCoordinates || (mpTextureCoordinates->getTextureCoordinate(0) == mpTextureCoordinates->getTextureCoordinate(nIndex)));

                        if(bTextureCoordinatesEqual)
                        {
                            return true;
                        }
                    }
                }
            }
        }

        // test for range
        for(sal_uInt32 a(0); a < maPoints.count() - 1L; a++)
        {
            if(maPoints.getCoordinate(a) == maPoints.getCoordinate(a + 1L))
            {
                const bool bBColorEqual(!mpBColors || (mpBColors->getBColor(a) == mpBColors->getBColor(a + 1L)));

                if(bBColorEqual)
                {
                    const bool bNormalsEqual(!mpNormals || (mpNormals->getNormal(a) == mpNormals->getNormal(a + 1L)));

                    if(bNormalsEqual)
                    {
                        const bool bTextureCoordinatesEqual(!mpTextureCoordinates || (mpTextureCoordinates->getTextureCoordinate(a) == mpTextureCoordinates->getTextureCoordinate(a + 1L)));

                        if(bTextureCoordinatesEqual)
                        {
                            return true;
                        }
                    }
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
            bool bRemove;

            do
            {
                bRemove = false;

                if(maPoints.count() > 1L)
                {
                    const sal_uInt32 nIndex(maPoints.count() - 1L);
                    bRemove = (maPoints.getCoordinate(0) == maPoints.getCoordinate(nIndex));

                    if(bRemove && mpBColors && !(mpBColors->getBColor(0) == mpBColors->getBColor(nIndex)))
                    {
                        bRemove = false;
                    }

                    if(bRemove && mpNormals && !(mpNormals->getNormal(0) == mpNormals->getNormal(nIndex)))
                    {
                        bRemove = false;
                    }

                    if(bRemove && mpTextureCoordinates && !(mpTextureCoordinates->getTextureCoordinate(0) == mpTextureCoordinates->getTextureCoordinate(nIndex)))
                    {
                        bRemove = false;
                    }
                }

                if(bRemove)
                {
                    const sal_uInt32 nIndex(maPoints.count() - 1L);
                    remove(nIndex, 1L);
                }
            } while(bRemove);
        }
    }

    void removeDoublePointsWholeTrack()
    {
        sal_uInt32 nIndex(0);

        // test as long as there are at least two points and as long as the index
        // is smaller or equal second last point
        while((maPoints.count() > 1L) && (nIndex <= maPoints.count() - 2L))
        {
            const sal_uInt32 nNextIndex(nIndex + 1L);
            bool bRemove(maPoints.getCoordinate(nIndex) == maPoints.getCoordinate(nNextIndex));

            if(bRemove && mpBColors && !(mpBColors->getBColor(nIndex) == mpBColors->getBColor(nNextIndex)))
            {
                bRemove = false;
            }

            if(bRemove && mpNormals && !(mpNormals->getNormal(nIndex) == mpNormals->getNormal(nNextIndex)))
            {
                bRemove = false;
            }

            if(bRemove && mpTextureCoordinates && !(mpTextureCoordinates->getTextureCoordinate(nIndex) == mpTextureCoordinates->getTextureCoordinate(nNextIndex)))
            {
                bRemove = false;
            }

            if(bRemove)
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

    void transform(const ::basegfx::B3DHomMatrix& rMatrix)
    {
        maPoints.transform(rMatrix);

        // Here, it seems to be possible to transform a valid plane normal and to avoid
        // invalidation, but it's not true. If the transformation contains shears or e.g.
        // perspective projection, the orthogonality to the transformed plane will not
        // be preserved. It may be possible to test that at the matrix to not invalidate in
        // all cases or to extract a matrix which does not 'shear' the vector which is
        // a normal in this case. As long as this is not sure, i will just invalidate.
        invalidatePlaneNormal();
    }
};

namespace basegfx
{
    namespace { struct DefaultPolygon : public rtl::Static< B3DPolygon::ImplType,
                                                            DefaultPolygon > {}; }

    B3DPolygon::B3DPolygon() :
        mpPolygon(DefaultPolygon::get())
    {
    }

    B3DPolygon::B3DPolygon(const B3DPolygon& rPolygon) :
        mpPolygon(rPolygon.mpPolygon)
    {
    }

    B3DPolygon::~B3DPolygon()
    {
    }

    B3DPolygon& B3DPolygon::operator=(const B3DPolygon& rPolygon)
    {
        mpPolygon = rPolygon.mpPolygon;
        return *this;
    }

    bool B3DPolygon::operator==(const B3DPolygon& rPolygon) const
    {
        if(mpPolygon.same_object(rPolygon.mpPolygon))
            return true;

        return (*mpPolygon == *rPolygon.mpPolygon);
    }

    bool B3DPolygon::operator!=(const B3DPolygon& rPolygon) const
    {
        return !(*this == rPolygon);
    }

    sal_uInt32 B3DPolygon::count() const
    {
        return mpPolygon->count();
    }

    basegfx::B3DPoint B3DPolygon::getB3DPoint(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B3DPolygon access outside range (!)");

        return mpPolygon->getPoint(nIndex);
    }

    void B3DPolygon::setB3DPoint(sal_uInt32 nIndex, const basegfx::B3DPoint& rValue)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B3DPolygon access outside range (!)");

        if(getB3DPoint(nIndex) != rValue)
            mpPolygon->setPoint(nIndex, rValue);
    }

    BColor B3DPolygon::getBColor(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B3DPolygon access outside range (!)");

        return mpPolygon->getBColor(nIndex);
    }

    void B3DPolygon::setBColor(sal_uInt32 nIndex, const BColor& rValue)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B3DPolygon access outside range (!)");

        if(mpPolygon->getBColor(nIndex) != rValue)
            mpPolygon->setBColor(nIndex, rValue);
    }

    bool B3DPolygon::areBColorsUsed() const
    {
        return mpPolygon->areBColorsUsed();
    }

    void B3DPolygon::clearBColors()
    {
        if(mpPolygon->areBColorsUsed())
            mpPolygon->clearBColors();
    }

    B3DVector B3DPolygon::getNormal() const
    {
        return mpPolygon->getNormal();
    }

    B3DVector B3DPolygon::getNormal(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B3DPolygon access outside range (!)");

        return mpPolygon->getNormal(nIndex);
    }

    void B3DPolygon::setNormal(sal_uInt32 nIndex, const B3DVector& rValue)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B3DPolygon access outside range (!)");

        if(mpPolygon->getNormal(nIndex) != rValue)
            mpPolygon->setNormal(nIndex, rValue);
    }

    void B3DPolygon::transformNormals(const B3DHomMatrix& rMatrix)
    {
        if(mpPolygon->areNormalsUsed() && !rMatrix.isIdentity())
            mpPolygon->transformNormals(rMatrix);
    }

    bool B3DPolygon::areNormalsUsed() const
    {
        return mpPolygon->areNormalsUsed();
    }

    void B3DPolygon::clearNormals()
    {
        if(mpPolygon->areNormalsUsed())
            mpPolygon->clearNormals();
    }

    B2DPoint B3DPolygon::getTextureCoordinate(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B3DPolygon access outside range (!)");

        return mpPolygon->getTextureCoordinate(nIndex);
    }

    void B3DPolygon::setTextureCoordinate(sal_uInt32 nIndex, const B2DPoint& rValue)
    {
        OSL_ENSURE(nIndex < mpPolygon->count(), "B3DPolygon access outside range (!)");

        if(mpPolygon->getTextureCoordinate(nIndex) != rValue)
            mpPolygon->setTextureCoordinate(nIndex, rValue);
    }

    void B3DPolygon::transformTextureCoordinates(const B2DHomMatrix& rMatrix)
    {
        if(mpPolygon->areTextureCoordinatesUsed() && !rMatrix.isIdentity())
            mpPolygon->transformTextureCoordinates(rMatrix);
    }

    bool B3DPolygon::areTextureCoordinatesUsed() const
    {
        return mpPolygon->areTextureCoordinatesUsed();
    }

    void B3DPolygon::clearTextureCoordinates()
    {
        if(mpPolygon->areTextureCoordinatesUsed())
            mpPolygon->clearTextureCoordinates();
    }

    void B3DPolygon::append(const basegfx::B3DPoint& rPoint, sal_uInt32 nCount)
    {
        if(nCount)
            mpPolygon->insert(mpPolygon->count(), rPoint, nCount);
    }

    void B3DPolygon::append(const B3DPolygon& rPoly, sal_uInt32 nIndex, sal_uInt32 nCount)
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
                OSL_ENSURE(nIndex + nCount <= rPoly.mpPolygon->count(), "B3DPolygon Append outside range (!)");
                ImplB3DPolygon aTempPoly(*rPoly.mpPolygon, nIndex, nCount);
                mpPolygon->insert(mpPolygon->count(), aTempPoly);
            }
        }
    }

    void B3DPolygon::remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex + nCount <= mpPolygon->count(), "B3DPolygon Remove outside range (!)");

        if(nCount)
            mpPolygon->remove(nIndex, nCount);
    }

    void B3DPolygon::clear()
    {
        mpPolygon = DefaultPolygon::get();
    }

    bool B3DPolygon::isClosed() const
    {
        return mpPolygon->isClosed();
    }

    void B3DPolygon::setClosed(bool bNew)
    {
        if(isClosed() != bNew)
            mpPolygon->setClosed(bNew);
    }

    void B3DPolygon::flip()
    {
        if(count() > 1)
            mpPolygon->flip();
    }

    bool B3DPolygon::hasDoublePoints() const
    {
        return (mpPolygon->count() > 1L && mpPolygon->hasDoublePoints());
    }

    void B3DPolygon::removeDoublePoints()
    {
        if(hasDoublePoints())
        {
            mpPolygon->removeDoublePointsAtBeginEnd();
            mpPolygon->removeDoublePointsWholeTrack();
        }
    }

    void B3DPolygon::transform(const basegfx::B3DHomMatrix& rMatrix)
    {
        if(mpPolygon->count() && !rMatrix.isIdentity())
        {
            mpPolygon->transform(rMatrix);
        }
    }
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
