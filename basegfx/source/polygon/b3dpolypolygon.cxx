/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: b3dpolypolygon.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:09:19 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYPOLYGON_HXX
#include <basegfx/polygon/b3dpolypolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

#include <functional>
#include <vector>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////

class ImplB3DPolyPolygon
{
    typedef ::std::vector< ::basegfx::B3DPolygon >  PolygonVector;

    PolygonVector                                   maPolygons;

public:
    ImplB3DPolyPolygon() : maPolygons()
    {
    }

    ImplB3DPolyPolygon(const ::basegfx::B3DPolygon& rToBeCopied) :
        maPolygons(1,rToBeCopied)
    {
    }

    bool isEqual(const ImplB3DPolyPolygon& rPolygonList) const
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
            aIndex += nIndex;
            maPolygons.insert(aIndex, nCount, rPolygon);
        }
    }

    void insert(sal_uInt32 nIndex, const ::basegfx::B3DPolyPolygon& rPolyPolygon)
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
                maPolygons.insert(aIndex, rPolyPolygon.getB3DPolygon(a));
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
                       std::mem_fun_ref( &::basegfx::B3DPolygon::flip ));
    }

    void removeDoublePoints()
    {
        std::for_each( maPolygons.begin(),
                       maPolygons.end(),
                       std::mem_fun_ref( &::basegfx::B3DPolygon::removeDoublePoints ));
    }

    void transform(const ::basegfx::B3DHomMatrix& rMatrix)
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
                       std::mem_fun_ref( &::basegfx::B3DPolygon::makeUnique ));
    }
};

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace { struct DefaultPolyPolygon : public rtl::Static<B3DPolyPolygon::ImplType,
                                                               DefaultPolyPolygon> {}; }

    B3DPolyPolygon::B3DPolyPolygon() :
        mpPolyPolygon(DefaultPolyPolygon::get())
    {
    }

    B3DPolyPolygon::B3DPolyPolygon(const B3DPolyPolygon& rPolyPolygon) :
        mpPolyPolygon(rPolyPolygon.mpPolyPolygon)
    {
    }

    B3DPolyPolygon::B3DPolyPolygon(const B3DPolygon& rPolygon) :
        mpPolyPolygon( ImplB3DPolyPolygon(rPolygon) )
    {
    }

    B3DPolyPolygon::~B3DPolyPolygon()
    {
    }

    B3DPolyPolygon& B3DPolyPolygon::operator=(const B3DPolyPolygon& rPolyPolygon)
    {
        mpPolyPolygon = rPolyPolygon.mpPolyPolygon;
        return *this;
    }

    void B3DPolyPolygon::makeUnique()
    {
        mpPolyPolygon.make_unique();
        mpPolyPolygon->makeUnique();
    }

    bool B3DPolyPolygon::operator==(const B3DPolyPolygon& rPolyPolygon) const
    {
        if(mpPolyPolygon.same_object(rPolyPolygon.mpPolyPolygon))
            return true;

        return mpPolyPolygon->isEqual(*(rPolyPolygon.mpPolyPolygon));
    }

    bool B3DPolyPolygon::operator!=(const B3DPolyPolygon& rPolyPolygon) const
    {
        return !(*this == rPolyPolygon);
    }

    sal_uInt32 B3DPolyPolygon::count() const
    {
        return mpPolyPolygon->count();
    }

    B3DPolygon B3DPolyPolygon::getB3DPolygon(sal_uInt32 nIndex) const
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

    void B3DPolyPolygon::insert(sal_uInt32 nIndex, const B3DPolygon& rPolygon, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex <= mpPolyPolygon->count(), "B3DPolyPolygon Insert outside range (!)");

        if(nCount)
            mpPolyPolygon->insert(nIndex, rPolygon, nCount);
    }

    void B3DPolyPolygon::append(const B3DPolygon& rPolygon, sal_uInt32 nCount)
    {
        if(nCount)
            mpPolyPolygon->insert(mpPolyPolygon->count(), rPolygon, nCount);
    }

    void B3DPolyPolygon::insert(sal_uInt32 nIndex, const B3DPolyPolygon& rPolyPolygon)
    {
        OSL_ENSURE(nIndex <= mpPolyPolygon->count(), "B3DPolyPolygon Insert outside range (!)");

        if(rPolyPolygon.count())
            mpPolyPolygon->insert(nIndex, rPolyPolygon);
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
        mpPolyPolygon = DefaultPolyPolygon::get();
    }

    bool B3DPolyPolygon::isClosed() const
    {
        bool bRetval(true);

        // PolyPOlygon is closed when all contained Polygons are closed or
        // no Polygon exists.
        for(sal_uInt32 a(0L); bRetval && a < mpPolyPolygon->count(); a++)
        {
            if(!(mpPolyPolygon->getB3DPolygon(a)).isClosed())
            {
                bRetval = false;
            }
        }

        return bRetval;
    }

    void B3DPolyPolygon::setClosed(bool bNew)
    {
        if(bNew != isClosed())
            mpPolyPolygon->setClosed(bNew);
    }

    void B3DPolyPolygon::flip()
    {
        mpPolyPolygon->flip();
    }

    bool B3DPolyPolygon::hasDoublePoints() const
    {
        bool bRetval(false);

        for(sal_uInt32 a(0L); !bRetval && a < mpPolyPolygon->count(); a++)
        {
            if((mpPolyPolygon->getB3DPolygon(a)).hasDoublePoints())
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

    void B3DPolyPolygon::transform(const ::basegfx::B3DHomMatrix& rMatrix)
    {
        if(mpPolyPolygon->count() && !rMatrix.isIdentity())
        {
            mpPolyPolygon->transform(rMatrix);
        }
    }
} // end of namespace basegfx

// eof
