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

#include <cassert>
#include <utility>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/utils/systemdependentdata.hxx>

namespace basegfx
{

class ImplB2DPolyPolygon
{
    basegfx::B2DPolygonVector                               maPolygons;
    // we do not want to 'modify' the ImplB2DPolyPolygon,
    // but add buffered data that is valid for all referencing instances
    mutable std::unique_ptr<basegfx::SystemDependentDataHolder> mpSystemDependentDataHolder;

public:
    ImplB2DPolyPolygon()
    {
    }

    explicit ImplB2DPolyPolygon(const ImplB2DPolyPolygon& rSource)
    :   maPolygons(rSource.maPolygons)
    {
    }

    explicit ImplB2DPolyPolygon(const basegfx::B2DPolygon& rToBeCopied)
    :   maPolygons(1,rToBeCopied)
    {
    }

    ImplB2DPolyPolygon& operator=(const ImplB2DPolyPolygon& rSource)
    {
        if (this != &rSource)
        {
            maPolygons = rSource.maPolygons;
            mpSystemDependentDataHolder.reset();
        }

        return *this;
    }

    void addOrReplaceSystemDependentData(basegfx::SystemDependentData_SharedPtr& rData) const
    {
        if(!mpSystemDependentDataHolder)
        {
            mpSystemDependentDataHolder.reset(new basegfx::SystemDependentDataHolder());
        }

        mpSystemDependentDataHolder->addOrReplaceSystemDependentData(rData);
    }

    basegfx::SystemDependentData_SharedPtr getSystemDependentData(size_t hash_code) const
    {
        if(!mpSystemDependentDataHolder)
        {
            return basegfx::SystemDependentData_SharedPtr();
        }

        return mpSystemDependentDataHolder->getSystemDependentData(hash_code);
    }

    bool operator==(const ImplB2DPolyPolygon& rPolygonList) const
    {
        return maPolygons == rPolygonList.maPolygons;
    }

    const basegfx::B2DPolygon& getB2DPolygon(sal_uInt32 nIndex) const
    {
        assert(nIndex < maPolygons.size());
        return maPolygons[nIndex];
    }

    void setB2DPolygon(sal_uInt32 nIndex, const basegfx::B2DPolygon& rPolygon)
    {
        assert(nIndex < maPolygons.size());
        maPolygons[nIndex] = rPolygon;
    }

    void insert(sal_uInt32 nIndex, const basegfx::B2DPolygon& rPolygon, sal_uInt32 nCount)
    {
        assert(nCount > 0);
        assert(nIndex <= maPolygons.size());
        // add nCount copies of rPolygon
        maPolygons.insert(maPolygons.begin() + nIndex, nCount, rPolygon);
    }

    void append(const basegfx::B2DPolygon& rPolygon, sal_uInt32 nCount)
    {
        insert(maPolygons.size(), rPolygon, nCount);
    }

    void reserve(sal_uInt32 nCount)
    {
        maPolygons.reserve(nCount);
    }

    void insert(sal_uInt32 nIndex, const basegfx::B2DPolyPolygon& rPolyPolygon)
    {
        assert(nIndex <= maPolygons.size());
        // add nCount polygons from rPolyPolygon
        maPolygons.insert(maPolygons.begin() + nIndex, rPolyPolygon.begin(), rPolyPolygon.end());
    }

    void append(const basegfx::B2DPolyPolygon& rPolyPolygon)
    {
        insert(maPolygons.size(), rPolyPolygon);
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        assert(nCount > 0);
        assert(nIndex + nCount <= maPolygons.size());
        // remove polygon data
        auto aStart(maPolygons.begin() + nIndex);
        auto aEnd(aStart + nCount);

        maPolygons.erase(aStart, aEnd);
    }

    sal_uInt32 count() const
    {
        return maPolygons.size();
    }

    void setClosed(bool bNew)
    {
        for(basegfx::B2DPolygon & rPolygon : maPolygons)
        {
            rPolygon.setClosed(bNew);
        }
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

    void transform(const basegfx::B2DHomMatrix& rMatrix)
    {
        for (auto& aPolygon : maPolygons)
            aPolygon.transform(rMatrix);
    }

    void makeUnique()
    {
        for (auto& aPolygon : maPolygons)
            aPolygon.makeUnique();
    }

    const basegfx::B2DPolygon* begin() const
    {
        if (maPolygons.empty())
            return nullptr;
        else
            return maPolygons.data();
    }

    const basegfx::B2DPolygon* end() const
    {
        if (maPolygons.empty())
            return nullptr;
        else
            return maPolygons.data() + maPolygons.size();
    }

    basegfx::B2DPolygon* begin()
    {
        if (maPolygons.empty())
            return nullptr;
        else
            return maPolygons.data();
    }

    basegfx::B2DPolygon* end()
    {
        if (maPolygons.empty())
            return nullptr;
        else
            return maPolygons.data() + maPolygons.size();
    }
};

    B2DPolyPolygon::B2DPolyPolygon() = default;

    B2DPolyPolygon::B2DPolyPolygon(const B2DPolyPolygon&) = default;

    B2DPolyPolygon::B2DPolyPolygon(B2DPolyPolygon&&) = default;

    B2DPolyPolygon::B2DPolyPolygon(const B2DPolygon& rPolygon) :
        mpPolyPolygon( ImplB2DPolyPolygon(rPolygon) )
    {
    }

    B2DPolyPolygon::~B2DPolyPolygon() = default;

    B2DPolyPolygon& B2DPolyPolygon::operator=(const B2DPolyPolygon&) = default;

    B2DPolyPolygon& B2DPolyPolygon::operator=(B2DPolyPolygon&&) = default;

    void B2DPolyPolygon::makeUnique()
    {
        mpPolyPolygon->makeUnique(); // non-const cow_wrapper::operator-> calls make_unique
    }

    bool B2DPolyPolygon::operator==(const B2DPolyPolygon& rPolyPolygon) const
    {
        if(mpPolyPolygon.same_object(rPolyPolygon.mpPolyPolygon))
            return true;

        return ((*mpPolyPolygon) == (*rPolyPolygon.mpPolyPolygon));
    }

    sal_uInt32 B2DPolyPolygon::count() const
    {
        return mpPolyPolygon->count();
    }

    B2DPolygon const & B2DPolyPolygon::getB2DPolygon(sal_uInt32 nIndex) const
    {
        return mpPolyPolygon->getB2DPolygon(nIndex);
    }

    void B2DPolyPolygon::setB2DPolygon(sal_uInt32 nIndex, const B2DPolygon& rPolygon)
    {
        if(getB2DPolygon(nIndex) != rPolygon)
            mpPolyPolygon->setB2DPolygon(nIndex, rPolygon);
    }

    bool B2DPolyPolygon::areControlPointsUsed() const
    {
        for(sal_uInt32 a(0); a < count(); a++)
        {
            if(getB2DPolygon(a).areControlPointsUsed())
            {
                return true;
            }
        }

        return false;
    }

    void B2DPolyPolygon::insert(sal_uInt32 nIndex, const B2DPolygon& rPolygon, sal_uInt32 nCount)
    {
        if(nCount)
            mpPolyPolygon->insert(nIndex, rPolygon, nCount);
    }

    void B2DPolyPolygon::append(const B2DPolygon& rPolygon, sal_uInt32 nCount)
    {
        if(nCount)
            mpPolyPolygon->append(rPolygon, nCount);
    }

    void B2DPolyPolygon::reserve(sal_uInt32 nCount)
    {
        if(nCount)
            mpPolyPolygon->reserve(nCount);
    }

    B2DPolyPolygon B2DPolyPolygon::getDefaultAdaptiveSubdivision() const
    {
        B2DPolyPolygon aRetval;
        if (count())
        {
            ImplB2DPolyPolygon& dest = *aRetval.mpPolyPolygon;
            dest.reserve(count());

            for (sal_uInt32 a(0); a < count(); a++)
            {
                dest.append(getB2DPolygon(a).getDefaultAdaptiveSubdivision(), 1);
            }
        }

        return aRetval;
    }

    B2DRange B2DPolyPolygon::getB2DRange() const
    {
        B2DRange aRetval;

        for(sal_uInt32 a(0); a < count(); a++)
        {
            aRetval.expand(getB2DPolygon(a).getB2DRange());
        }

        return aRetval;
    }

    void B2DPolyPolygon::insert(sal_uInt32 nIndex, const B2DPolyPolygon& rPolyPolygon)
    {
        if(rPolyPolygon.count())
            mpPolyPolygon->insert(nIndex, rPolyPolygon);
    }

    void B2DPolyPolygon::append(const B2DPolyPolygon& rPolyPolygon)
    {
        if(rPolyPolygon.count())
            mpPolyPolygon->append(rPolyPolygon);
    }

    void B2DPolyPolygon::remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(nCount)
            mpPolyPolygon->remove(nIndex, nCount);
    }

    void B2DPolyPolygon::clear()
    {
        *mpPolyPolygon = ImplB2DPolyPolygon();
    }

    bool B2DPolyPolygon::isClosed() const
    {
        // PolyPOlygon is closed when all contained Polygons are closed or
        // no Polygon exists.
        for(sal_uInt32 a(0); a < count(); a++)
        {
            if(!getB2DPolygon(a).isClosed())
                return false;
        }

        return true;
    }

    void B2DPolyPolygon::setClosed(bool bNew)
    {
        if(bNew != isClosed())
            mpPolyPolygon->setClosed(bNew);
    }

    void B2DPolyPolygon::flip()
    {
        if(count())
        {
            mpPolyPolygon->flip();
        }
    }

    bool B2DPolyPolygon::hasDoublePoints() const
    {
        for(sal_uInt32 a(0); a < count(); a++)
        {
            if(getB2DPolygon(a).hasDoublePoints())
                return true;
        }

        return false;
    }

    void B2DPolyPolygon::removeDoublePoints()
    {
        if(hasDoublePoints())
            mpPolyPolygon->removeDoublePoints();
    }

    void B2DPolyPolygon::transform(const B2DHomMatrix& rMatrix)
    {
        if(count() && !rMatrix.isIdentity())
        {
            mpPolyPolygon->transform(rMatrix);
        }
    }

    const B2DPolygon* B2DPolyPolygon::begin() const
    {
        return mpPolyPolygon->begin();
    }

    const B2DPolygon* B2DPolyPolygon::end() const
    {
        return mpPolyPolygon->end();
    }

    B2DPolygon* B2DPolyPolygon::begin()
    {
        return mpPolyPolygon->begin();
    }

    B2DPolygon* B2DPolyPolygon::end()
    {
        return mpPolyPolygon->end();
    }

    void B2DPolyPolygon::addOrReplaceSystemDependentDataInternal(SystemDependentData_SharedPtr& rData) const
    {
        mpPolyPolygon->addOrReplaceSystemDependentData(rData);
    }

    SystemDependentData_SharedPtr B2DPolyPolygon::getSystemDependantDataInternal(size_t hash_code) const
    {
        return mpPolyPolygon->getSystemDependentData(hash_code);
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
