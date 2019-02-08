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
#include <basegfx/polygon/b3dpolypolygon.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <vector>

class ImplB3DPolyPolygon
{
    typedef std::vector< ::basegfx::B3DPolygon >  PolygonVector;

    PolygonVector                                   maPolygons;

public:
    ImplB3DPolyPolygon() : maPolygons()
    {
    }

    explicit ImplB3DPolyPolygon(const ::basegfx::B3DPolygon& rToBeCopied) :
        maPolygons(1,rToBeCopied)
    {
    }

    bool operator==(const ImplB3DPolyPolygon& rPolygonList) const
    {
        // same polygon count?
        if(maPolygons.size() != rPolygonList.maPolygons.size())
            return false;

        // compare polygon content
        if(maPolygons != rPolygonList.maPolygons)
            return false;

        return true;
    }

    const ::basegfx::B3DPolygon& getB3DPolygon(sal_uInt32 nIndex) const
    {
        return maPolygons[nIndex];
    }

    void setB3DPolygon(sal_uInt32 nIndex, const ::basegfx::B3DPolygon& rPolygon)
    {
        maPolygons[nIndex] = rPolygon;
    }

    void insert(sal_uInt32 nIndex, const ::basegfx::B3DPolygon& rPolygon, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // add nCount copies of rPolygon
            PolygonVector::iterator aIndex(maPolygons.begin());
            if( nIndex )
                aIndex += nIndex;
            maPolygons.insert(aIndex, nCount, rPolygon);
        }
    }

    void insert(sal_uInt32 nIndex, const ::basegfx::B3DPolyPolygon& rPolyPolygon)
    {
        // add all polygons from rPolyPolygon
        PolygonVector::iterator aIndex(maPolygons.begin());
        if( nIndex )
            aIndex += nIndex;
        maPolygons.insert(aIndex, rPolyPolygon.begin(), rPolyPolygon.end());
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // remove polygon data
            PolygonVector::iterator aStart(maPolygons.begin());
            aStart += nIndex;
            const PolygonVector::iterator aEnd(aStart + nCount);

            maPolygons.erase(aStart, aEnd);
        }
    }

    sal_uInt32 count() const
    {
        return maPolygons.size();
    }

    void flip()
    {
        for (auto& aPolygon : maPolygons)
            aPolygon.flip();
    }

    void removeDoublePoints()
    {
        for (auto& aPolygon : maPolygons)
            aPolygon.removeDoublePoints();
    }

    void transform(const ::basegfx::B3DHomMatrix& rMatrix)
    {
        for (auto& aPolygon : maPolygons)
            aPolygon.transform(rMatrix);
    }

    void clearBColors()
    {
        for (auto& aPolygon : maPolygons)
            aPolygon.clearBColors();
    }

    void transformNormals(const ::basegfx::B3DHomMatrix& rMatrix)
    {
        for (auto& aPolygon : maPolygons)
            aPolygon.transformNormals(rMatrix);
    }

    void clearNormals()
    {
        for (auto& aPolygon : maPolygons)
            aPolygon.clearNormals();
    }

    void transformTextureCoordinates(const ::basegfx::B2DHomMatrix& rMatrix)
    {
        for (auto& aPolygon : maPolygons)
            aPolygon.transformTextureCoordinates(rMatrix);
    }

    void clearTextureCoordinates()
    {
        for (auto& aPolygon : maPolygons)
            aPolygon.clearTextureCoordinates();
    }

    const basegfx::B3DPolygon* begin() const
    {
        if (maPolygons.empty())
            return nullptr;
        else
            return maPolygons.data();
    }

    const basegfx::B3DPolygon* end() const
    {
        if (maPolygons.empty())
            return nullptr;
        else
            return maPolygons.data() + maPolygons.size();
    }

    basegfx::B3DPolygon* begin()
    {
        if (maPolygons.empty())
            return nullptr;
        else
            return maPolygons.data();
    }

    basegfx::B3DPolygon* end()
    {
        if (maPolygons.empty())
            return nullptr;
        else
            return maPolygons.data() + maPolygons.size();
    }
};

namespace basegfx
{
    namespace {

    B3DPolyPolygon::ImplType const & getDefaultPolyPolygon() {
        static B3DPolyPolygon::ImplType const singleton;
        return singleton;
    }

    }

    B3DPolyPolygon::B3DPolyPolygon() :
        mpPolyPolygon(getDefaultPolyPolygon())
    {
    }

    B3DPolyPolygon::B3DPolyPolygon(const B3DPolyPolygon&) = default;

    B3DPolyPolygon::B3DPolyPolygon(B3DPolyPolygon&&) = default;

    B3DPolyPolygon::B3DPolyPolygon(const B3DPolygon& rPolygon) :
        mpPolyPolygon( ImplB3DPolyPolygon(rPolygon) )
    {
    }

    B3DPolyPolygon::~B3DPolyPolygon() = default;

    B3DPolyPolygon& B3DPolyPolygon::operator=(const B3DPolyPolygon&) = default;

    B3DPolyPolygon& B3DPolyPolygon::operator=(B3DPolyPolygon&&) = default;

    bool B3DPolyPolygon::operator==(const B3DPolyPolygon& rPolyPolygon) const
    {
        if(mpPolyPolygon.same_object(rPolyPolygon.mpPolyPolygon))
            return true;

        return ((*mpPolyPolygon) == (*rPolyPolygon.mpPolyPolygon));
    }

    bool B3DPolyPolygon::operator!=(const B3DPolyPolygon& rPolyPolygon) const
    {
        return !(*this == rPolyPolygon);
    }

    sal_uInt32 B3DPolyPolygon::count() const
    {
        return mpPolyPolygon->count();
    }

    B3DPolygon const & B3DPolyPolygon::getB3DPolygon(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolyPolygon->count(), "B3DPolyPolygon access outside range (!)");

        return mpPolyPolygon->getB3DPolygon(nIndex);
    }

    void B3DPolyPolygon::setB3DPolygon(sal_uInt32 nIndex, const B3DPolygon& rPolygon)
    {
        OSL_ENSURE(nIndex < mpPolyPolygon->count(), "B3DPolyPolygon access outside range (!)");

        if(getB3DPolygon(nIndex) != rPolygon)
            mpPolyPolygon->setB3DPolygon(nIndex, rPolygon);
    }

    bool B3DPolyPolygon::areBColorsUsed() const
    {
        for(sal_uInt32 a(0); a < mpPolyPolygon->count(); a++)
        {
            if(mpPolyPolygon->getB3DPolygon(a).areBColorsUsed())
            {
                return true;
            }
        }

        return false;
    }

    void B3DPolyPolygon::clearBColors()
    {
        if(areBColorsUsed())
            mpPolyPolygon->clearBColors();
    }

    void B3DPolyPolygon::transformNormals(const B3DHomMatrix& rMatrix)
    {
        if(!rMatrix.isIdentity())
            mpPolyPolygon->transformNormals(rMatrix);
    }

    bool B3DPolyPolygon::areNormalsUsed() const
    {
        for(sal_uInt32 a(0); a < mpPolyPolygon->count(); a++)
        {
            if(mpPolyPolygon->getB3DPolygon(a).areNormalsUsed())
            {
                return true;
            }
        }

        return false;
    }

    void B3DPolyPolygon::clearNormals()
    {
        if(areNormalsUsed())
            mpPolyPolygon->clearNormals();
    }

    void B3DPolyPolygon::transformTextureCoordinates(const B2DHomMatrix& rMatrix)
    {
        if(!rMatrix.isIdentity())
            mpPolyPolygon->transformTextureCoordinates(rMatrix);
    }

    bool B3DPolyPolygon::areTextureCoordinatesUsed() const
    {
        for(sal_uInt32 a(0); a < mpPolyPolygon->count(); a++)
        {
            if(mpPolyPolygon->getB3DPolygon(a).areTextureCoordinatesUsed())
            {
                return true;
            }
        }

        return false;
    }

    void B3DPolyPolygon::clearTextureCoordinates()
    {
        if(areTextureCoordinatesUsed())
            mpPolyPolygon->clearTextureCoordinates();
    }

    void B3DPolyPolygon::append(const B3DPolygon& rPolygon, sal_uInt32 nCount)
    {
        if(nCount)
            mpPolyPolygon->insert(mpPolyPolygon->count(), rPolygon, nCount);
    }

    void B3DPolyPolygon::append(const B3DPolyPolygon& rPolyPolygon)
    {
        if(rPolyPolygon.count())
            mpPolyPolygon->insert(mpPolyPolygon->count(), rPolyPolygon);
    }

    void B3DPolyPolygon::remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex + nCount <= mpPolyPolygon->count(), "B3DPolyPolygon Remove outside range (!)");

        if(nCount)
            mpPolyPolygon->remove(nIndex, nCount);
    }

    void B3DPolyPolygon::clear()
    {
        mpPolyPolygon = getDefaultPolyPolygon();
    }

    void B3DPolyPolygon::flip()
    {
        mpPolyPolygon->flip();
    }

    bool B3DPolyPolygon::hasDoublePoints() const
    {
        bool bRetval(false);

        for(sal_uInt32 a(0); !bRetval && a < mpPolyPolygon->count(); a++)
        {
            if(mpPolyPolygon->getB3DPolygon(a).hasDoublePoints())
            {
                bRetval = true;
            }
        }

        return bRetval;
    }

    void B3DPolyPolygon::removeDoublePoints()
    {
        if(hasDoublePoints())
            mpPolyPolygon->removeDoublePoints();
    }

    void B3DPolyPolygon::transform(const B3DHomMatrix& rMatrix)
    {
        if(mpPolyPolygon->count() && !rMatrix.isIdentity())
        {
            mpPolyPolygon->transform(rMatrix);
        }
    }

    const B3DPolygon* B3DPolyPolygon::begin() const
    {
        return mpPolyPolygon->begin();
    }

    const B3DPolygon* B3DPolyPolygon::end() const
    {
        return mpPolyPolygon->end();
    }

    B3DPolygon* B3DPolyPolygon::begin()
    {
        return mpPolyPolygon->begin();
    }

    B3DPolygon* B3DPolyPolygon::end()
    {
        return mpPolyPolygon->end();
    }
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
