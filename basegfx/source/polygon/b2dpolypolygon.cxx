/*************************************************************************
 *
 *  $RCSfile: b2dpolypolygon.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: aw $ $Date: 2003-11-10 11:45:50 $
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

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#include <vector>

//////////////////////////////////////////////////////////////////////////////

class ImplB2DPolyPolygon
{
    typedef ::std::vector< ::basegfx::polygon::B2DPolygon > PolygonVector;

    PolygonVector                                   maPolygons;
    sal_uInt32                                      mnRefCount;

public:
    // This constructor is only used from the static identity polygon, thus
    // the RefCount is set to 1 to never 'delete' this static incarnation.
    ImplB2DPolyPolygon()
    :   mnRefCount(1)
    {
        // complete initialization with defaults
    }

    ImplB2DPolyPolygon(const ImplB2DPolyPolygon& rToBeCopied)
    :   mnRefCount(0)
    {
        // complete initialization using copy
        maPolygons = rToBeCopied.maPolygons;
    }

    ~ImplB2DPolyPolygon()
    {
    }

    const sal_uInt32 getRefCount() const { return mnRefCount; }
    void incRefCount() { mnRefCount++; }
    void decRefCount() { mnRefCount--; }

    sal_Bool isEqual(const ImplB2DPolyPolygon& rPolygonList) const
    {
        // same polygon count?
        if(maPolygons.size() != rPolygonList.maPolygons.size())
            return sal_False;

        // if zero polygons the polys are equal
        if(!maPolygons.size())
            return sal_True;

        // compare polygon content
        if(maPolygons != rPolygonList.maPolygons)
            return sal_False;

        return sal_True;
    }

    const ::basegfx::polygon::B2DPolygon& getPolygon(sal_uInt32 nIndex) const
    {
        return maPolygons[nIndex];
    }

    void setPolygon(sal_uInt32 nIndex, const ::basegfx::polygon::B2DPolygon& rPolygon)
    {
        maPolygons[nIndex] = rPolygon;
    }

    void insert(sal_uInt32 nIndex, const ::basegfx::polygon::B2DPolygon& rPolygon, sal_uInt32 nCount)
    {
        if(nCount)
        {
            // add nCount copies of rPolygon
            PolygonVector::iterator aIndex(maPolygons.begin());
            aIndex += nIndex;
            maPolygons.insert(aIndex, nCount, rPolygon);
        }
    }

    void insert(sal_uInt32 nIndex, const ::basegfx::polygon::B2DPolyPolygon& rPolyPolygon)
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
                maPolygons.insert(aIndex, rPolyPolygon.getPolygon(a));
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

    void setClosed(sal_Bool bNew)
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
};

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    namespace polygon
    {
        // init static default Polygon
        static ImplB2DPolyPolygon maStaticDefaultPolyPolygon;

        void B2DPolyPolygon::implForceUniqueCopy()
        {
            if(mpPolyPolygon->getRefCount())
            {
                mpPolyPolygon->decRefCount();
                mpPolyPolygon = new ImplB2DPolyPolygon(*mpPolyPolygon);
            }
        }

        B2DPolyPolygon::B2DPolyPolygon()
        :   mpPolyPolygon(&maStaticDefaultPolyPolygon)
        {
            mpPolyPolygon->incRefCount();
        }

        B2DPolyPolygon::B2DPolyPolygon(const B2DPolyPolygon& rPolyPolygon)
        :   mpPolyPolygon(rPolyPolygon.mpPolyPolygon)
        {
            mpPolyPolygon->incRefCount();
        }

        B2DPolyPolygon::~B2DPolyPolygon()
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

        B2DPolyPolygon& B2DPolyPolygon::operator=(const B2DPolyPolygon& rPolyPolygon)
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

        sal_Bool B2DPolyPolygon::operator==(const B2DPolyPolygon& rPolyPolygon) const
        {
            if(mpPolyPolygon == rPolyPolygon.mpPolyPolygon)
            {
                return sal_True;
            }

            return mpPolyPolygon->isEqual(*(rPolyPolygon.mpPolyPolygon));
        }

        sal_Bool B2DPolyPolygon::operator!=(const B2DPolyPolygon& rPolyPolygon) const
        {
            if(mpPolyPolygon == rPolyPolygon.mpPolyPolygon)
            {
                return sal_False;
            }

            return !mpPolyPolygon->isEqual(*(rPolyPolygon.mpPolyPolygon));
        }

        sal_uInt32 B2DPolyPolygon::count() const
        {
            return mpPolyPolygon->count();
        }

        B2DPolygon B2DPolyPolygon::getPolygon(sal_uInt32 nIndex) const
        {
            DBG_ASSERT(nIndex < mpPolyPolygon->count(), "B2DPolyPolygon access outside range (!)");

            return mpPolyPolygon->getPolygon(nIndex);
        }

        void B2DPolyPolygon::setPolygon(sal_uInt32 nIndex, const B2DPolygon& rPolygon)
        {
            DBG_ASSERT(nIndex < mpPolyPolygon->count(), "B2DPolyPolygon access outside range (!)");

            if(mpPolyPolygon->getPolygon(nIndex) != rPolygon)
            {
                implForceUniqueCopy();
                mpPolyPolygon->setPolygon(nIndex, rPolygon);
            }
        }

        sal_Bool B2DPolyPolygon::areControlPointsUsed() const
        {
            for(sal_uInt32 a(0L); a < mpPolyPolygon->count(); a++)
            {
                const ::basegfx::polygon::B2DPolygon& rPolygon = mpPolyPolygon->getPolygon(a);

                if(rPolygon.areControlPointsUsed())
                {
                    return sal_True;
                }
            }

            return sal_False;
        }

        void B2DPolyPolygon::insert(sal_uInt32 nIndex, const B2DPolygon& rPolygon, sal_uInt32 nCount)
        {
            DBG_ASSERT(nIndex <= mpPolyPolygon->count(), "B2DPolyPolygon Insert outside range (!)");

            if(nCount)
            {
                implForceUniqueCopy();
                mpPolyPolygon->insert(nIndex, rPolygon, nCount);
            }
        }

        void B2DPolyPolygon::append(const B2DPolygon& rPolygon, sal_uInt32 nCount)
        {
            if(nCount)
            {
                implForceUniqueCopy();
                mpPolyPolygon->insert(mpPolyPolygon->count(), rPolygon, nCount);
            }
        }

        void B2DPolyPolygon::insert(sal_uInt32 nIndex, const B2DPolyPolygon& rPolyPolygon)
        {
            DBG_ASSERT(nIndex <= mpPolyPolygon->count(), "B2DPolyPolygon Insert outside range (!)");

            if(rPolyPolygon.count())
            {
                implForceUniqueCopy();
                mpPolyPolygon->insert(nIndex, rPolyPolygon);
            }
        }

        void B2DPolyPolygon::append(const B2DPolyPolygon& rPolyPolygon)
        {
            if(rPolyPolygon.count())
            {
                implForceUniqueCopy();
                mpPolyPolygon->insert(mpPolyPolygon->count(), rPolyPolygon);
            }
        }

        void B2DPolyPolygon::remove(sal_uInt32 nIndex, sal_uInt32 nCount)
        {
            DBG_ASSERT(nIndex + nCount <= mpPolyPolygon->count(), "B2DPolyPolygon Remove outside range (!)");

            if(nCount)
            {
                implForceUniqueCopy();
                mpPolyPolygon->remove(nIndex, nCount);
            }
        }

        void B2DPolyPolygon::clear()
        {
            if(mpPolyPolygon->getRefCount())
            {
                mpPolyPolygon->decRefCount();
            }
            else
            {
                delete mpPolyPolygon;
            }

            mpPolyPolygon = &maStaticDefaultPolyPolygon;
            mpPolyPolygon->incRefCount();
        }

        sal_Bool B2DPolyPolygon::isClosed() const
        {
            sal_Bool bRetval(sal_True);

            // PolyPOlygon is closed when all contained Polygons are closed or
            // no Polygon exists.
            for(sal_uInt32 a(0L); bRetval && a < mpPolyPolygon->count(); a++)
            {
                if(!(mpPolyPolygon->getPolygon(a)).isClosed())
                {
                    bRetval = sal_False;
                }
            }

            return bRetval;
        }

        void B2DPolyPolygon::setClosed(sal_Bool bNew)
        {
            if(bNew != isClosed())
            {
                implForceUniqueCopy();
                mpPolyPolygon->setClosed(bNew);
            }
        }

        void B2DPolyPolygon::flip()
        {
            implForceUniqueCopy();
            mpPolyPolygon->flip();
        }

        sal_Bool B2DPolyPolygon::hasDoublePoints() const
        {
            sal_Bool bRetval(sal_False);

            for(sal_uInt32 a(0L); !bRetval && a < mpPolyPolygon->count(); a++)
            {
                if((mpPolyPolygon->getPolygon(a)).hasDoublePoints())
                {
                    bRetval = sal_True;
                }
            }

            return bRetval;
        }

        void B2DPolyPolygon::removeDoublePoints()
        {
            if(hasDoublePoints())
            {
                implForceUniqueCopy();
                mpPolyPolygon->removeDoublePoints();
            }
        }
    } // end of namespace polygon
} // end of namespace basegfx

// eof
