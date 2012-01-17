/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <basegfx/range/b2dpolyrange.hxx>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2drangeclipper.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <algorithm>
#include <vector>

namespace basegfx
{
    class ImplB2DPolyRange
    {
        void updateBounds()
        {
            maBounds.reset();
            std::for_each(maRanges.begin(),
                          maRanges.end(),
                          boost::bind(
                              (void (B2DRange::*)(const B2DRange&))(
                 &B2DRange::expand),
                              boost::ref(maBounds),
                              _1));
        }

    public:
        ImplB2DPolyRange() :
            maBounds(),
            maRanges(),
            maOrient()
        {}

        explicit ImplB2DPolyRange( const B2DRange& rRange, B2VectorOrientation eOrient ) :
            maBounds( rRange ),
            maRanges( 1, rRange ),
            maOrient( 1, eOrient )
        {}

        bool operator==(const ImplB2DPolyRange& rRHS) const
        {
            return maRanges == rRHS.maRanges && maOrient == rRHS.maOrient;
        }

        sal_uInt32 count() const
        {
            return maRanges.size();
        }

        B2DPolyRange::ElementType getElement(sal_uInt32 nIndex) const
        {
            return boost::make_tuple(maRanges[nIndex],
                                     maOrient[nIndex]);
        }

        void appendElement(const B2DRange& rRange, B2VectorOrientation eOrient, sal_uInt32 nCount)
        {
            maRanges.insert(maRanges.end(), nCount, rRange);
            maOrient.insert(maOrient.end(), nCount, eOrient);
            maBounds.expand(rRange);
        }

        void clear()
        {
            std::vector<B2DRange> aTmpRanges;
            std::vector<B2VectorOrientation> aTmpOrient;

            maRanges.swap(aTmpRanges);
            maOrient.swap(aTmpOrient);

            maBounds.reset();
        }

        bool overlaps( const B2DRange& rRange ) const
        {
            if( !maBounds.overlaps( rRange ) )
                return false;

            const std::vector<B2DRange>::const_iterator aEnd( maRanges.end() );
            return std::find_if( maRanges.begin(),
                                 aEnd,
                                 boost::bind<bool>( boost::mem_fn( &B2DRange::overlaps ),
                                                    _1,
                                                    boost::cref(rRange) ) ) != aEnd;
        }

        B2DPolyPolygon solveCrossovers() const
        {
            return tools::solveCrossovers(maRanges,maOrient);
        }

    private:
        B2DRange                         maBounds;
        std::vector<B2DRange>            maRanges;
        std::vector<B2VectorOrientation> maOrient;
    };

    B2DPolyRange::B2DPolyRange() :
        mpImpl()
    {}

    B2DPolyRange::~B2DPolyRange()
    {}

    B2DPolyRange::B2DPolyRange( const B2DPolyRange& rRange ) :
        mpImpl( rRange.mpImpl )
    {}

    B2DPolyRange& B2DPolyRange::operator=( const B2DPolyRange& rRange )
    {
        mpImpl = rRange.mpImpl;
        return *this;
    }

    bool B2DPolyRange::operator==(const B2DPolyRange& rRange) const
    {
        if(mpImpl.same_object(rRange.mpImpl))
            return true;

        return ((*mpImpl) == (*rRange.mpImpl));
    }

    bool B2DPolyRange::operator!=(const B2DPolyRange& rRange) const
    {
        return !(*this == rRange);
    }

    sal_uInt32 B2DPolyRange::count() const
    {
        return mpImpl->count();
    }

    B2DPolyRange::ElementType B2DPolyRange::getElement(sal_uInt32 nIndex) const
    {
        return mpImpl->getElement(nIndex);
    }

    void B2DPolyRange::appendElement(const B2DRange& rRange, B2VectorOrientation eOrient, sal_uInt32 nCount)
    {
        mpImpl->appendElement(rRange, eOrient, nCount );
    }

    void B2DPolyRange::clear()
    {
        mpImpl->clear();
    }

    bool B2DPolyRange::overlaps( const B2DRange& rRange ) const
    {
        return mpImpl->overlaps(rRange);
    }

    B2DPolyPolygon B2DPolyRange::solveCrossovers() const
    {
        return mpImpl->solveCrossovers();
    }
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
