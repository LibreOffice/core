/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b2dpolypolygon.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:08:20 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#include <functional>
#include <vector>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////

class ImplB2DPolyPolygon
{
    typedef ::std::vector< ::basegfx::B2DPolygon >  PolygonVector;

    PolygonVector                                   maPolygons;

public:
    ImplB2DPolyPolygon() : maPolygons()
    {
    }

    ImplB2DPolyPolygon(const ::basegfx::B2DPolygon& rToBeCopied) :
        maPolygons(1,rToBeCopied)
    {
    }

    bool isEqual(const ImplB2DPolyPolygon& rPolygonList) const
    {
        // same polygon count?
        if(maPolygons.size() != rPolygonList.maPolygons.size())
            return false;

        // if zero polygons the polys are equal
        if(!maPolygons.size())
            return true;

        // compare polygon content
        if(maPolygons != rPolygonList.maPolygons)
            return false;

        return true;
    }

    const ::basegfx::B2DPolygon& getB2DPolygon(sal_uInt32 nIndex) const
    {
        return maPolygons[nIndex];
    }

    void setB2DPolygon(sal_uInt32 nIndex, const ::basegfx::B2DPolygon& rPolygon)
    {
        maPolygons[nIndex] = rPolygon;
    }

    void insert(sal_uInt32 nIndex, const ::basegfx::B2DPolygon& rPolygon, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // add nCount copies of rPolygon
            PolygonVector::iterator aIndex(maPolygons.begin());
            aIndex += nIndex;
            maPolygons.insert(aIndex, nCount, rPolygon);
        }
    }

    void insert(sal_uInt32 nIndex, const ::basegfx::B2DPolyPolygon& rPolyPolygon)
    {
        const sal_uInt32 nCount = rPolyPolygon.count();

        if(nCount)
        {
            // add nCount polygons from rPolyPolygon
            maPolygons.reserve(maPolygons.size() + nCount);
            PolygonVector::iterator aIndex(maPolygons.begin());
            aIndex += nIndex;

            for(sal_uInt32 a(0L); a < nCount; a++)
            {
                maPolygons.insert(aIndex, rPolyPolygon.getB2DPolygon(a));
                aIndex++;
            }
        }
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

    void setClosed(bool bNew)
    {
        for(sal_uInt32 a(0L); a < maPolygons.size(); a++)
        {
            maPolygons[a].setClosed(bNew);
        }
    }

    void flip()
    {
        std::for_each( maPolygons.begin(),
                       maPolygons.end(),
                       std::mem_fun_ref( &::basegfx::B2DPolygon::flip ));
    }

    void removeDoublePoints()
    {
        std::for_each( maPolygons.begin(),
                       maPolygons.end(),
                       std::mem_fun_ref( &::basegfx::B2DPolygon::removeDoublePoints ));
    }

    void transform(const ::basegfx::B2DHomMatrix& rMatrix)
    {
        for(sal_uInt32 a(0L); a < maPolygons.size(); a++)
        {
            maPolygons[a].transform(rMatrix);
        }
    }

    void makeUnique()
    {
        std::for_each( maPolygons.begin(),
                       maPolygons.end(),
                       std::mem_fun_ref( &::basegfx::B2DPolygon::makeUnique ));
    }
};

//////////////////////////////////////////////////////////////////////////////

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

        return mpPolyPolygon->isEqual(*(rPolyPolygon.mpPolyPolygon));
    }

    bool B2DPolyPolygon::operator!=(const B2DPolyPolygon& rPolyPolygon) const
    {
        return !(*this == rPolyPolygon);
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

    bool B2DPolyPolygon::areControlVectorsUsed() const
    {
        for(sal_uInt32 a(0L); a < mpPolyPolygon->count(); a++)
        {
            const ::basegfx::B2DPolygon& rPolygon = mpPolyPolygon->getB2DPolygon(a);

            if(rPolygon.areControlVectorsUsed())
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

        // PolyPOlygon is closed when all contained Polygons are closed or
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

    void B2DPolyPolygon::setClosed(bool bNew)
    {
        if(bNew != isClosed())
            mpPolyPolygon->setClosed(bNew);
    }

    void B2DPolyPolygon::flip()
    {
        mpPolyPolygon->flip();
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

    void B2DPolyPolygon::transform(const ::basegfx::B2DHomMatrix& rMatrix)
    {
        if(mpPolyPolygon->count() && !rMatrix.isIdentity())
        {
            mpPolyPolygon->transform(rMatrix);
        }
    }
} // end of namespace basegfx

// eof
