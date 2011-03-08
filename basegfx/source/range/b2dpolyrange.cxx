/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: b2dmultirange.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"
#include <basegfx/range/b2dpolyrange.hxx>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2drangeclipper.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <algorithm>
#include <vector>

static basegfx::B2VectorOrientation flipOrientation(
    basegfx::B2VectorOrientation eOrient)
{
    return eOrient == basegfx::ORIENTATION_POSITIVE ?
        basegfx::ORIENTATION_NEGATIVE : basegfx::ORIENTATION_POSITIVE;
}

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

        explicit ImplB2DPolyRange( const B2DPolyRange::ElementType& rElem ) :
            maBounds( boost::get<0>(rElem) ),
            maRanges( 1, boost::get<0>(rElem) ),
            maOrient( 1, boost::get<1>(rElem) )
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

        void setElement(sal_uInt32 nIndex, const B2DPolyRange::ElementType& rElement )
        {
            maRanges[nIndex] = boost::get<0>(rElement);
            maOrient[nIndex] = boost::get<1>(rElement);
            updateBounds();
        }

        void setElement(sal_uInt32 nIndex, const B2DRange& rRange, B2VectorOrientation eOrient )
        {
            maRanges[nIndex] = rRange;
            maOrient[nIndex] = eOrient;
            updateBounds();
        }

        void insertElement(sal_uInt32 nIndex, const B2DPolyRange::ElementType& rElement, sal_uInt32 nCount)
        {
            maRanges.insert(maRanges.begin()+nIndex, nCount, boost::get<0>(rElement));
            maOrient.insert(maOrient.begin()+nIndex, nCount, boost::get<1>(rElement));
            maBounds.expand(boost::get<0>(rElement));
        }

        void insertElement(sal_uInt32 nIndex, const B2DRange& rRange, B2VectorOrientation eOrient, sal_uInt32 nCount)
        {
            maRanges.insert(maRanges.begin()+nIndex, nCount, rRange);
            maOrient.insert(maOrient.begin()+nIndex, nCount, eOrient);
            maBounds.expand(rRange);
        }

        void appendElement(const B2DPolyRange::ElementType& rElement, sal_uInt32 nCount)
        {
            maRanges.insert(maRanges.end(), nCount, boost::get<0>(rElement));
            maOrient.insert(maOrient.end(), nCount, boost::get<1>(rElement));
            maBounds.expand(boost::get<0>(rElement));
        }

        void appendElement(const B2DRange& rRange, B2VectorOrientation eOrient, sal_uInt32 nCount)
        {
            maRanges.insert(maRanges.end(), nCount, rRange);
            maOrient.insert(maOrient.end(), nCount, eOrient);
            maBounds.expand(rRange);
        }

        void insertPolyRange(sal_uInt32 nIndex, const ImplB2DPolyRange& rPolyRange)
        {
            maRanges.insert(maRanges.begin()+nIndex, rPolyRange.maRanges.begin(), rPolyRange.maRanges.end());
            maOrient.insert(maOrient.begin()+nIndex, rPolyRange.maOrient.begin(), rPolyRange.maOrient.end());
            updateBounds();
        }

        void appendPolyRange(const ImplB2DPolyRange& rPolyRange)
        {
            maRanges.insert(maRanges.end(),
                            rPolyRange.maRanges.begin(),
                            rPolyRange.maRanges.end());
            maOrient.insert(maOrient.end(),
                            rPolyRange.maOrient.begin(),
                            rPolyRange.maOrient.end());
            updateBounds();
        }

        void remove(sal_uInt32 nIndex, sal_uInt32 nCount)
        {
            maRanges.erase(maRanges.begin()+nIndex,maRanges.begin()+nIndex+nCount);
            maOrient.erase(maOrient.begin()+nIndex,maOrient.begin()+nIndex+nCount);
            updateBounds();
        }

        void clear()
        {
            std::vector<B2DRange> aTmpRanges;
            std::vector<B2VectorOrientation> aTmpOrient;

            maRanges.swap(aTmpRanges);
            maOrient.swap(aTmpOrient);

            maBounds.reset();
        }

        void flip()
        {
            std::for_each(maOrient.begin(),
                          maOrient.end(),
                          boost::bind(
                              &flipOrientation,
                              _1));
        }

        B2DRange getBounds() const
        {
            return maBounds;
        }

        template< typename ValueType > bool isInside( const ValueType& rValue ) const
        {
            if( !maBounds.isInside( rValue ) )
                return false;

            // cannot use boost::bind here, since isInside is overloaded.
            // It is currently not possible to resolve the overload
            // by considering one of the other template arguments.
            std::vector<B2DRange>::const_iterator       aCurr( maRanges.begin() );
            const std::vector<B2DRange>::const_iterator aEnd ( maRanges.end() );
            while( aCurr != aEnd )
                if( aCurr->isInside( rValue ) )
                    return true;

            return false;
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

        const B2DRange* begin() const
        {
            if(maRanges.empty())
                return 0;
            else
                return &maRanges.front();
        }

        const B2DRange* end() const
        {
            if(maRanges.empty())
                return 0;
            else
                return (&maRanges.back())+1;
        }

        B2DRange* begin()
        {
            if(maRanges.empty())
                return 0;
            else
                return &maRanges.front();
        }

        B2DRange* end()
        {
            if(maRanges.empty())
                return 0;
            else
                return (&maRanges.back())+1;
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

    B2DPolyRange::B2DPolyRange( const ElementType& rElem ) :
        mpImpl( ImplB2DPolyRange( rElem ) )
    {}

    B2DPolyRange::B2DPolyRange( const B2DRange& rRange, B2VectorOrientation eOrient ) :
        mpImpl( ImplB2DPolyRange( rRange, eOrient ) )
    {}

    B2DPolyRange::B2DPolyRange( const B2DPolyRange& rRange ) :
        mpImpl( rRange.mpImpl )
    {}

    B2DPolyRange& B2DPolyRange::operator=( const B2DPolyRange& rRange )
    {
        mpImpl = rRange.mpImpl;
        return *this;
    }

    void B2DPolyRange::makeUnique()
    {
        mpImpl.make_unique();
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

    void B2DPolyRange::setElement(sal_uInt32 nIndex, const ElementType& rElement )
    {
        mpImpl->setElement(nIndex, rElement);
    }

    void B2DPolyRange::setElement(sal_uInt32 nIndex, const B2DRange& rRange, B2VectorOrientation eOrient )
    {
        mpImpl->setElement(nIndex, rRange, eOrient );
    }

    void B2DPolyRange::insertElement(sal_uInt32 nIndex, const ElementType& rElement, sal_uInt32 nCount)
    {
        mpImpl->insertElement(nIndex, rElement, nCount );
    }

    void B2DPolyRange::insertElement(sal_uInt32 nIndex, const B2DRange& rRange, B2VectorOrientation eOrient, sal_uInt32 nCount)
    {
        mpImpl->insertElement(nIndex, rRange, eOrient, nCount );
    }

    void B2DPolyRange::appendElement(const ElementType& rElement, sal_uInt32 nCount)
    {
        mpImpl->appendElement(rElement, nCount);
    }

    void B2DPolyRange::appendElement(const B2DRange& rRange, B2VectorOrientation eOrient, sal_uInt32 nCount)
    {
        mpImpl->appendElement(rRange, eOrient, nCount );
    }

    void B2DPolyRange::insertPolyRange(sal_uInt32 nIndex, const B2DPolyRange& rRange)
    {
        mpImpl->insertPolyRange(nIndex, *rRange.mpImpl);
    }

    void B2DPolyRange::appendPolyRange(const B2DPolyRange& rRange)
    {
        mpImpl->appendPolyRange(*rRange.mpImpl);
    }

    void B2DPolyRange::remove(sal_uInt32 nIndex, sal_uInt32 nCount)
    {
        mpImpl->remove(nIndex, nCount);
    }

    void B2DPolyRange::clear()
    {
        mpImpl->clear();
    }

    void B2DPolyRange::flip()
    {
        mpImpl->flip();
    }

    B2DRange B2DPolyRange::getBounds() const
    {
        return mpImpl->getBounds();
    }

    bool B2DPolyRange::isInside( const B2DTuple& rTuple ) const
    {
        return mpImpl->isInside(rTuple);
    }

    bool B2DPolyRange::isInside( const B2DRange& rRange ) const
    {
        return mpImpl->isInside(rRange);
    }

    bool B2DPolyRange::overlaps( const B2DRange& rRange ) const
    {
        return mpImpl->overlaps(rRange);
    }

    B2DPolyPolygon B2DPolyRange::solveCrossovers() const
    {
        return mpImpl->solveCrossovers();
    }

    const B2DRange* B2DPolyRange::begin() const
    {
        return mpImpl->begin();
    }

    const B2DRange* B2DPolyRange::end() const
    {
        return mpImpl->end();
    }

    B2DRange* B2DPolyRange::begin()
    {
        return mpImpl->begin();
    }

    B2DRange* B2DPolyRange::end()
    {
        return mpImpl->end();
    }

} // end of namespace basegfx

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
