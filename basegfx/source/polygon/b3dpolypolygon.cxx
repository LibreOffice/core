/*************************************************************************
 *
 *  $RCSfile: b3dpolypolygon.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hjs $ $Date: 2004-06-25 17:18:23 $
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

#ifndef _BGFX_POLYGON_B3DPOLYPOLYGON_HXX
#include <basegfx/polygon/b3dpolypolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B3DPOLYGON_HXX
#include <basegfx/polygon/b3dpolygon.hxx>
#endif

#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

#include <vector>

//////////////////////////////////////////////////////////////////////////////

class ImplB3DPolyPolygon
{
    typedef ::std::vector< ::basegfx::B3DPolygon >  PolygonVector;

    PolygonVector                                   maPolygons;
    sal_uInt32                                      mnRefCount;

public:
    // This constructor is only used from the static identity polygon, thus
    // the RefCount is set to 1 to never 'delete' this static incarnation.
    ImplB3DPolyPolygon()
    :   mnRefCount(1)
    {
        // complete initialization with defaults
    }

    ImplB3DPolyPolygon(const ImplB3DPolyPolygon& rToBeCopied)
    :   mnRefCount(0)
    {
        // complete initialization using copy
        maPolygons = rToBeCopied.maPolygons;
    }

    ~ImplB3DPolyPolygon()
    {
    }

    const sal_uInt32 getRefCount() const { return mnRefCount; }
    void incRefCount() { mnRefCount++; }
    void decRefCount() { mnRefCount--; }

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
        for(sal_uInt32 a(0L); a < maPolygons.size(); a++)
        {
            maPolygons[a].flip();
        }
    }

    void removeDoublePoints()
    {
        for(sal_uInt32 a(0L); a < maPolygons.size(); a++)
        {
            maPolygons[a].removeDoublePoints();
        }
    }

    void transform(const ::basegfx::B3DHomMatrix& rMatrix)
    {
        for(sal_uInt32 a(0L); a < maPolygons.size(); a++)
        {
            maPolygons[a].transform(rMatrix);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

namespace { struct DefaultPolyPolygon : public rtl::Static<ImplB3DPolyPolygon, DefaultPolyPolygon> {}; }

namespace basegfx
{
    void B3DPolyPolygon::implForceUniqueCopy()
    {
        if(mpPolyPolygon->getRefCount())
        {
            mpPolyPolygon->decRefCount();
            mpPolyPolygon = new ImplB3DPolyPolygon(*mpPolyPolygon);
        }
    }

    B3DPolyPolygon::B3DPolyPolygon()
    :   mpPolyPolygon(&DefaultPolyPolygon::get())
    {
        mpPolyPolygon->incRefCount();
    }

    B3DPolyPolygon::B3DPolyPolygon(const B3DPolyPolygon& rPolyPolygon)
    :   mpPolyPolygon(rPolyPolygon.mpPolyPolygon)
    {
        mpPolyPolygon->incRefCount();
    }

    B3DPolyPolygon::~B3DPolyPolygon()
    {
        if(mpPolyPolygon->getRefCount())
        {
            mpPolyPolygon->decRefCount();
        }
        else
        {
            delete mpPolyPolygon;
        }
    }

    B3DPolyPolygon& B3DPolyPolygon::operator=(const B3DPolyPolygon& rPolyPolygon)
    {
        if(mpPolyPolygon->getRefCount())
        {
            mpPolyPolygon->decRefCount();
        }
        else
        {
            delete mpPolyPolygon;
        }

        mpPolyPolygon = rPolyPolygon.mpPolyPolygon;
        mpPolyPolygon->incRefCount();

        return *this;
    }

    bool B3DPolyPolygon::operator==(const B3DPolyPolygon& rPolyPolygon) const
    {
        if(mpPolyPolygon == rPolyPolygon.mpPolyPolygon)
        {
            return true;
        }

        return mpPolyPolygon->isEqual(*(rPolyPolygon.mpPolyPolygon));
    }

    bool B3DPolyPolygon::operator!=(const B3DPolyPolygon& rPolyPolygon) const
    {
        if(mpPolyPolygon == rPolyPolygon.mpPolyPolygon)
        {
            return false;
        }

        return !mpPolyPolygon->isEqual(*(rPolyPolygon.mpPolyPolygon));
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

        if(mpPolyPolygon->getB3DPolygon(nIndex) != rPolygon)
        {
            implForceUniqueCopy();
            mpPolyPolygon->setB3DPolygon(nIndex, rPolygon);
        }
    }

    void B3DPolyPolygon::insert(sal_uInt32 nIndex, const B3DPolygon& rPolygon, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex <= mpPolyPolygon->count(), "B3DPolyPolygon Insert outside range (!)");

        if(nCount)
        {
            implForceUniqueCopy();
            mpPolyPolygon->insert(nIndex, rPolygon, nCount);
        }
    }

    void B3DPolyPolygon::append(const B3DPolygon& rPolygon, sal_uInt32 nCount)
    {
        if(nCount)
        {
            implForceUniqueCopy();
            mpPolyPolygon->insert(mpPolyPolygon->count(), rPolygon, nCount);
        }
    }

    void B3DPolyPolygon::insert(sal_uInt32 nIndex, const B3DPolyPolygon& rPolyPolygon)
    {
        OSL_ENSURE(nIndex <= mpPolyPolygon->count(), "B3DPolyPolygon Insert outside range (!)");

        if(rPolyPolygon.count())
        {
            implForceUniqueCopy();
            mpPolyPolygon->insert(nIndex, rPolyPolygon);
        }
    }

    void B3DPolyPolygon::append(const B3DPolyPolygon& rPolyPolygon)
    {
        if(rPolyPolygon.count())
        {
            implForceUniqueCopy();
            mpPolyPolygon->insert(mpPolyPolygon->count(), rPolyPolygon);
        }
    }

    void B3DPolyPolygon::remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        OSL_ENSURE(nIndex + nCount <= mpPolyPolygon->count(), "B3DPolyPolygon Remove outside range (!)");

        if(nCount)
        {
            implForceUniqueCopy();
            mpPolyPolygon->remove(nIndex, nCount);
        }
    }

    void B3DPolyPolygon::clear()
    {
        if(mpPolyPolygon->getRefCount())
        {
            mpPolyPolygon->decRefCount();
        }
        else
        {
            delete mpPolyPolygon;
        }

        mpPolyPolygon = &DefaultPolyPolygon::get();
        mpPolyPolygon->incRefCount();
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
        {
            implForceUniqueCopy();
            mpPolyPolygon->setClosed(bNew);
        }
    }

    void B3DPolyPolygon::flip()
    {
        implForceUniqueCopy();
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
        {
            implForceUniqueCopy();
            mpPolyPolygon->removeDoublePoints();
        }
    }

    void B3DPolyPolygon::transform(const ::basegfx::B3DHomMatrix& rMatrix)
    {
        implForceUniqueCopy();
        mpPolyPolygon->transform(rMatrix);
    }
} // end of namespace basegfx

// eof
