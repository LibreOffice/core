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

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <osl/diagnose.h>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/utils/systemdependentdata.hxx>

class ImplB2DPolyPolygon
{
    basegfx::B2DPolygonVector                               maPolygons;
    std::unique_ptr< basegfx::SystemDependentDataHolder >   mpSystemDependentDataHolder;

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

    void addOrReplaceSystemDependentData(basegfx::SystemDependentData_SharedPtr& rData)
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
        return maPolygons[nIndex];
    }

    void setB2DPolygon(sal_uInt32 nIndex, const basegfx::B2DPolygon& rPolygon)
    {
        maPolygons[nIndex] = rPolygon;
    }

    void insert(sal_uInt32 nIndex, const basegfx::B2DPolygon& rPolygon, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // add nCount copies of rPolygon
            basegfx::B2DPolygonVector::iterator aIndex(maPolygons.begin());
            if( nIndex )
                aIndex += nIndex;
            maPolygons.insert(aIndex, nCount, rPolygon);
        }
    }

    void insert(sal_uInt32 nIndex, const basegfx::B2DPolyPolygon& rPolyPolygon)
    {
        // add nCount polygons from rPolyPolygon
        basegfx::B2DPolygonVector::iterator aIndex(maPolygons.begin());
        if( nIndex )
            aIndex += nIndex;
        maPolygons.insert(aIndex, rPolyPolygon.begin(), rPolyPolygon.end());
    }

    void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // remove polygon data
            basegfx::B2DPolygonVector::iterator aStart(maPolygons.begin());
            aStart += nIndex;
            const basegfx::B2DPolygonVector::iterator aEnd(aStart + nCount);

            maPolygons.erase(aStart, aEnd);
        }
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

namespace basegfx
{

    static o3tl::cow_wrapper<ImplB2DPolyPolygon> DEFAULT;

    B2DPolyPolygon::B2DPolyPolygon() :
        mpImpl(DEFAULT) {}

    B2DPolyPolygon::B2DPolyPolygon(std::nullopt_t) noexcept : mpImpl(std::nullopt) {}

    B2DPolyPolygon::B2DPolyPolygon(const B2DPolyPolygon& rOther, std::nullopt_t) noexcept : mpImpl(rOther.mpImpl, std::nullopt) {}

    B2DPolyPolygon::B2DPolyPolygon(const B2DPolyPolygon&) = default;

    B2DPolyPolygon::B2DPolyPolygon(B2DPolyPolygon&&) = default;

    B2DPolyPolygon::B2DPolyPolygon(const B2DPolygon& rPolygon) :
        mpImpl( ImplB2DPolyPolygon(rPolygon) )
    {
    }

    B2DPolyPolygon::~B2DPolyPolygon() = default;

    B2DPolyPolygon& B2DPolyPolygon::operator=(const B2DPolyPolygon&) = default;

    B2DPolyPolygon& B2DPolyPolygon::operator=(B2DPolyPolygon&&) = default;

    void B2DPolyPolygon::makeUnique()
    {
        mpImpl.make_unique();
        mpImpl->makeUnique();
    }

    bool B2DPolyPolygon::operator==(const B2DPolyPolygon& rPolyPolygon) const
    {
        if(mpImpl.same_object(rPolyPolygon.mpImpl))
            return true;

        return ((*mpImpl) == (*rPolyPolygon.mpImpl));
    }

    bool B2DPolyPolygon::operator!=(const B2DPolyPolygon& rPolyPolygon) const
    {
        return !((*this) == rPolyPolygon);
    }

    sal_uInt32 B2DPolyPolygon::count() const
    {
        return mpImpl->count();
    }

    B2DPolygon const & B2DPolyPolygon::getB2DPolygon(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpImpl->count(), "B2DPolyPolygon access outside range (!)");

        return mpImpl->getB2DPolygon(nIndex);
    }

    void B2DPolyPolygon::setB2DPolygon(sal_uInt32 nIndex, const B2DPolygon& rPolygon)
    {
        OSL_ENSURE(nIndex < std::as_const(*mpImpl).count(), "B2DPolyPolygon access outside range (!)");

        if(getB2DPolygon(nIndex) != rPolygon)
            mpImpl->setB2DPolygon(nIndex, rPolygon);
    }

    bool B2DPolyPolygon::areControlPointsUsed() const
    {
        for(sal_uInt32 a(0); a < mpImpl->count(); a++)
        {
            const B2DPolygon& rPolygon = mpImpl->getB2DPolygon(a);

            if(rPolygon.areControlPointsUsed())
            {
                return true;
            }
        }

        return false;
    }

    void B2DPolyPolygon::insert(sal_uInt32 nIndex, const B2DPolygon& rPolygon, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex <= std::as_const(*mpImpl).count(), "B2DPolyPolygon Insert outside range (!)");

        if(nCount)
            mpImpl->insert(nIndex, rPolygon, nCount);
    }

    void B2DPolyPolygon::append(const B2DPolygon& rPolygon, sal_uInt32 nCount)
    {
        if(nCount)
            mpImpl->insert(std::as_const(*mpImpl).count(), rPolygon, nCount);
    }

    B2DPolyPolygon B2DPolyPolygon::getDefaultAdaptiveSubdivision() const
    {
        B2DPolyPolygon aRetval;

        for(sal_uInt32 a(0); a < mpImpl->count(); a++)
        {
            aRetval.append(mpImpl->getB2DPolygon(a).getDefaultAdaptiveSubdivision());
        }

        return aRetval;
    }

    B2DRange B2DPolyPolygon::getB2DRange() const
    {
        B2DRange aRetval;

        for(sal_uInt32 a(0); a < mpImpl->count(); a++)
        {
            aRetval.expand(mpImpl->getB2DPolygon(a).getB2DRange());
        }

        return aRetval;
    }

    void B2DPolyPolygon::insert(sal_uInt32 nIndex, const B2DPolyPolygon& rPolyPolygon)
    {
        OSL_ENSURE(nIndex <= std::as_const(*mpImpl).count(), "B2DPolyPolygon Insert outside range (!)");

        if(rPolyPolygon.count())
            mpImpl->insert(nIndex, rPolyPolygon);
    }

    void B2DPolyPolygon::append(const B2DPolyPolygon& rPolyPolygon)
    {
        if(rPolyPolygon.count())
            mpImpl->insert(std::as_const(*mpImpl).count(), rPolyPolygon);
    }

    void B2DPolyPolygon::remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex + nCount <= std::as_const(*mpImpl).count(), "B2DPolyPolygon Remove outside range (!)");

        if(nCount)
            mpImpl->remove(nIndex, nCount);
    }

    void B2DPolyPolygon::clear()
    {
        *mpImpl = ImplB2DPolyPolygon();
    }

    bool B2DPolyPolygon::isClosed() const
    {
        bool bRetval(true);

        // PolyPOlygon is closed when all contained Polygons are closed or
        // no Polygon exists.
        for(sal_uInt32 a(0); bRetval && a < mpImpl->count(); a++)
        {
            if(!mpImpl->getB2DPolygon(a).isClosed())
            {
                bRetval = false;
            }
        }

        return bRetval;
    }

    void B2DPolyPolygon::setClosed(bool bNew)
    {
        if(bNew != isClosed())
            mpImpl->setClosed(bNew);
    }

    void B2DPolyPolygon::flip()
    {
        if(std::as_const(*mpImpl).count())
        {
            mpImpl->flip();
        }
    }

    bool B2DPolyPolygon::hasDoublePoints() const
    {
        bool bRetval(false);

        for(sal_uInt32 a(0); !bRetval && a < mpImpl->count(); a++)
        {
            if(mpImpl->getB2DPolygon(a).hasDoublePoints())
            {
                bRetval = true;
            }
        }

        return bRetval;
    }

    void B2DPolyPolygon::removeDoublePoints()
    {
        if(hasDoublePoints())
            mpImpl->removeDoublePoints();
    }

    void B2DPolyPolygon::transform(const B2DHomMatrix& rMatrix)
    {
        if(std::as_const(*mpImpl).count() && !rMatrix.isIdentity())
        {
            mpImpl->transform(rMatrix);
        }
    }

    const B2DPolygon* B2DPolyPolygon::begin() const
    {
        return mpImpl->begin();
    }

    const B2DPolygon* B2DPolyPolygon::end() const
    {
        return mpImpl->end();
    }

    B2DPolygon* B2DPolyPolygon::begin()
    {
        return mpImpl->begin();
    }

    B2DPolygon* B2DPolyPolygon::end()
    {
        return mpImpl->end();
    }

    void B2DPolyPolygon::addOrReplaceSystemDependentDataInternal(SystemDependentData_SharedPtr& rData) const
    {
        // Need to get ImplB2DPolyPolygon* from cow_wrapper *without*
        // calling make_unique() here - we do not want to
        // 'modify' the ImplB2DPolyPolygon, but add buffered data that
        // is valid for all referencing instances
        const B2DPolyPolygon* pMe(this);
        const ImplB2DPolyPolygon* pMyImpl(pMe->mpImpl.get());

        const_cast<ImplB2DPolyPolygon*>(pMyImpl)->addOrReplaceSystemDependentData(rData);
    }

    SystemDependentData_SharedPtr B2DPolyPolygon::getSystemDependantDataInternal(size_t hash_code) const
    {
        return mpImpl->getSystemDependentData(hash_code);
    }

} // end of namespace basegfx

std::optional<basegfx::B2DPolyPolygon>::~optional() {}
void std::optional<basegfx::B2DPolyPolygon>::reset() { cow_optional::reset(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
