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
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <rtl/instance.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <functional>
#include <algorithm>

class ImplB2DPolyPolygon
{
    basegfx::B2DPolygonVector                   maPolygons;

public:
    ImplB2DPolyPolygon() : maPolygons()
    {
    }

    explicit ImplB2DPolyPolygon(const basegfx::B2DPolygon& rToBeCopied) :
        maPolygons(1,rToBeCopied)
    {
    }

    bool operator==(const ImplB2DPolyPolygon& rPolygonList) const
    {
        // same polygon count?
        if(maPolygons.size() != rPolygonList.maPolygons.size())
            return false;

        // compare polygon content
        if(!(maPolygons == rPolygonList.maPolygons))
            return false;

        return true;
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
        for(size_t a(0L); a < maPolygons.size(); a++)
        {
            maPolygons[a].setClosed(bNew);
        }
    }

    void flip()
    {
        std::for_each( maPolygons.begin(),
                       maPolygons.end(),
                       std::mem_fun_ref( &basegfx::B2DPolygon::flip ));
    }

    void removeDoublePoints()
    {
        std::for_each( maPolygons.begin(),
                       maPolygons.end(),
                       std::mem_fun_ref( &basegfx::B2DPolygon::removeDoublePoints ));
    }

    void transform(const basegfx::B2DHomMatrix& rMatrix)
    {
        for(size_t a(0L); a < maPolygons.size(); a++)
        {
            maPolygons[a].transform(rMatrix);
        }
    }

    void makeUnique()
    {
        std::for_each( maPolygons.begin(),
                       maPolygons.end(),
                       std::mem_fun_ref( &basegfx::B2DPolygon::makeUnique ));
    }

    const basegfx::B2DPolygon* begin() const
    {
        if(maPolygons.empty())
            return 0;
        else
            return &maPolygons.front();
    }

    const basegfx::B2DPolygon* end() const
    {
        if(maPolygons.empty())
            return 0;
        else
            return (&maPolygons.back())+1;
    }

    basegfx::B2DPolygon* begin()
    {
        if(maPolygons.empty())
            return 0;
        else
            return &maPolygons.front();
    }

    basegfx::B2DPolygon* end()
    {
        if(maPolygons.empty())
            return 0;
        else
            return &(maPolygons.back())+1;
    }
};

namespace basegfx
{
    namespace { struct DefaultPolyPolygon: public rtl::Static<B2DPolyPolygon::ImplType,
                                                              DefaultPolyPolygon> {}; }

    B2DPolyPolygon::B2DPolyPolygon() :
        mpPolyPolygon(DefaultPolyPolygon::get())
    {
    }

    B2DPolyPolygon::B2DPolyPolygon(const B2DPolyPolygon& rPolyPolygon) :
        mpPolyPolygon(rPolyPolygon.mpPolyPolygon)
    {
    }

    B2DPolyPolygon::B2DPolyPolygon(const B2DPolygon& rPolygon) :
        mpPolyPolygon( ImplB2DPolyPolygon(rPolygon) )
    {
    }

    B2DPolyPolygon::~B2DPolyPolygon()
    {
    }

    B2DPolyPolygon& B2DPolyPolygon::operator=(const B2DPolyPolygon& rPolyPolygon)
    {
        mpPolyPolygon = rPolyPolygon.mpPolyPolygon;
        return *this;
    }

    void B2DPolyPolygon::makeUnique()
    {
        mpPolyPolygon.make_unique();
        mpPolyPolygon->makeUnique();
    }

    bool B2DPolyPolygon::operator==(const B2DPolyPolygon& rPolyPolygon) const
    {
        if(mpPolyPolygon.same_object(rPolyPolygon.mpPolyPolygon))
            return true;

        return ((*mpPolyPolygon) == (*rPolyPolygon.mpPolyPolygon));
    }

    bool B2DPolyPolygon::operator!=(const B2DPolyPolygon& rPolyPolygon) const
    {
        return !((*this) == rPolyPolygon);
    }

    sal_uInt32 B2DPolyPolygon::count() const
    {
        return mpPolyPolygon->count();
    }

    B2DPolygon B2DPolyPolygon::getB2DPolygon(sal_uInt32 nIndex) const
    {
        OSL_ENSURE(nIndex < mpPolyPolygon->count(), "B2DPolyPolygon access outside range (!)");

        return mpPolyPolygon->getB2DPolygon(nIndex);
    }

    void B2DPolyPolygon::setB2DPolygon(sal_uInt32 nIndex, const B2DPolygon& rPolygon)
    {
        OSL_ENSURE(nIndex < mpPolyPolygon->count(), "B2DPolyPolygon access outside range (!)");

        if(getB2DPolygon(nIndex) != rPolygon)
            mpPolyPolygon->setB2DPolygon(nIndex, rPolygon);
    }

    bool B2DPolyPolygon::areControlPointsUsed() const
    {
        for(sal_uInt32 a(0L); a < mpPolyPolygon->count(); a++)
        {
            const B2DPolygon& rPolygon = mpPolyPolygon->getB2DPolygon(a);

            if(rPolygon.areControlPointsUsed())
            {
                return true;
            }
        }

        return false;
    }

    void B2DPolyPolygon::insert(sal_uInt32 nIndex, const B2DPolygon& rPolygon, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex <= mpPolyPolygon->count(), "B2DPolyPolygon Insert outside range (!)");

        if(nCount)
            mpPolyPolygon->insert(nIndex, rPolygon, nCount);
    }

    void B2DPolyPolygon::append(const B2DPolygon& rPolygon, sal_uInt32 nCount)
    {
        if(nCount)
            mpPolyPolygon->insert(mpPolyPolygon->count(), rPolygon, nCount);
    }

    B2DPolyPolygon B2DPolyPolygon::getDefaultAdaptiveSubdivision() const
    {
        B2DPolyPolygon aRetval;

        for(sal_uInt32 a(0L); a < mpPolyPolygon->count(); a++)
        {
            aRetval.append(mpPolyPolygon->getB2DPolygon(a).getDefaultAdaptiveSubdivision());
        }

        return aRetval;
    }

    B2DRange B2DPolyPolygon::getB2DRange() const
    {
        B2DRange aRetval;

        for(sal_uInt32 a(0L); a < mpPolyPolygon->count(); a++)
        {
            aRetval.expand(mpPolyPolygon->getB2DPolygon(a).getB2DRange());
        }

        return aRetval;
    }

    void B2DPolyPolygon::insert(sal_uInt32 nIndex, const B2DPolyPolygon& rPolyPolygon)
    {
        OSL_ENSURE(nIndex <= mpPolyPolygon->count(), "B2DPolyPolygon Insert outside range (!)");

        if(rPolyPolygon.count())
            mpPolyPolygon->insert(nIndex, rPolyPolygon);
    }

    void B2DPolyPolygon::append(const B2DPolyPolygon& rPolyPolygon)
    {
        if(rPolyPolygon.count())
            mpPolyPolygon->insert(mpPolyPolygon->count(), rPolyPolygon);
    }

    void B2DPolyPolygon::remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex + nCount <= mpPolyPolygon->count(), "B2DPolyPolygon Remove outside range (!)");

        if(nCount)
            mpPolyPolygon->remove(nIndex, nCount);
    }

    void B2DPolyPolygon::clear()
    {
        mpPolyPolygon = DefaultPolyPolygon::get();
    }

    bool B2DPolyPolygon::isClosed() const
    {
        bool bRetval(true);

        // PolyPolygon is closed when all contained Polygons are closed or
        // no Polygon exists.
        for(sal_uInt32 a(0L); bRetval && a < mpPolyPolygon->count(); a++)
        {
            if(!(mpPolyPolygon->getB2DPolygon(a)).isClosed())
            {
                bRetval = false;
            }
        }

        return bRetval;
    }

    bool B2DPolyPolygon::isMixedOpenAndClosed() const
    {
        bool bRetval(false);
        bool bOpen(false);
        bool bClosed(false);

        // PolyPolygon is mixed open and closed if there is more than one
        // polygon and there are both closed and open polygons.
        for(sal_uInt32 a(0L); !bRetval && a < mpPolyPolygon->count(); a++)
        {
            if ((mpPolyPolygon->getB2DPolygon(a)).isClosed())
            {
                bClosed = true;
            }
            else
            {
                bOpen = true;
            }

            bRetval = (bClosed && bOpen);
        }

        return bRetval;
    }

    void B2DPolyPolygon::setClosed(bool bNew)
    {
        if(bNew != isClosed())
            mpPolyPolygon->setClosed(bNew);
    }

    void B2DPolyPolygon::flip()
    {
        if(mpPolyPolygon->count())
        {
            mpPolyPolygon->flip();
        }
    }

    bool B2DPolyPolygon::hasDoublePoints() const
    {
        bool bRetval(false);

        for(sal_uInt32 a(0L); !bRetval && a < mpPolyPolygon->count(); a++)
        {
            if((mpPolyPolygon->getB2DPolygon(a)).hasDoublePoints())
            {
                bRetval = true;
            }
        }

        return bRetval;
    }

    void B2DPolyPolygon::removeDoublePoints()
    {
        if(hasDoublePoints())
            mpPolyPolygon->removeDoublePoints();
    }

    void B2DPolyPolygon::transform(const B2DHomMatrix& rMatrix)
    {
        if(mpPolyPolygon->count() && !rMatrix.isIdentity())
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
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
