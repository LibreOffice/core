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
#include <basegfx/range/b2drange.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2dmultirange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>
#include <algorithm>
#include <vector>


namespace basegfx
{
    class ImplB2DMultiRange
    {
    public:
        ImplB2DMultiRange() :
            maBounds(),
            maRanges()
        {
        }

        explicit ImplB2DMultiRange( const B2DRange& rRange ) :
            maBounds(),
            maRanges( 1, rRange )
        {
        }

        bool isEmpty() const
        {
            // no ranges at all, or all ranges empty
            return maRanges.empty() ||
                ::std::count_if( maRanges.begin(),
                                 maRanges.end(),
                                 ::boost::mem_fn( &B2DRange::isEmpty ) )
                == static_cast<VectorOfRanges::difference_type>(maRanges.size());
        }

        void reset()
        {
            // swap in empty vector
            VectorOfRanges aTmp;
            maRanges.swap( aTmp );

            maBounds.reset();
        }

        template< typename ValueType > bool isInside( const ValueType& rValue ) const
        {
            if( !maBounds.isInside( rValue ) )
                return false;

            // cannot use ::boost::bind here, since isInside is overloaded.
            // It is currently not possible to resolve the overload
            // by considering one of the other template arguments.
            VectorOfRanges::const_iterator       aCurr( maRanges.begin() );
            const VectorOfRanges::const_iterator aEnd ( maRanges.end() );
            while( aCurr != aEnd )
                if( aCurr->isInside( rValue ) )
                    return true;

            return false;
        }

        bool overlaps( const B2DRange& rRange ) const
        {
            if( !maBounds.overlaps( rRange ) )
                return false;

            const VectorOfRanges::const_iterator aEnd( maRanges.end() );
            return ::std::find_if( maRanges.begin(),
                                   aEnd,
                                   ::boost::bind<bool>( ::boost::mem_fn( &B2DRange::overlaps ),
                                                        _1,
                                                        rRange ) ) != aEnd;
        }

        void addRange( const B2DRange& rRange )
        {
            maRanges.push_back( rRange );
            maBounds.expand( rRange );
        }

        B2DRange getBounds() const
        {
            return maBounds;
        }

        B2DPolyPolygon getPolyPolygon() const
        {
            B2DPolyPolygon aRes;

            // Make range vector unique ( have to avoid duplicate
            // rectangles. The polygon clipper will return an empty
            // result in this case).
            VectorOfRanges  aUniqueRanges;
            aUniqueRanges.reserve( maRanges.size() );

            VectorOfRanges::const_iterator       aCurr( maRanges.begin() );
            const VectorOfRanges::const_iterator aEnd ( maRanges.end() );
            while( aCurr != aEnd )
            {
                // TODO(F3): It's plain wasted resources to apply a
                // general clipping algorithm to the problem at
                // hand. Go for a dedicated, scan-line-based approach.
                VectorOfRanges::const_iterator aCurrScan( aCurr+1 );
                VectorOfRanges::const_iterator aFound( aEnd );
                while( aCurrScan != aEnd )
                {
                    if( aCurrScan->equal( *aCurr ) ||
                        aCurrScan->isInside( *aCurr ) )
                    {
                        // current probe is equal to aCurr, or
                        // completely contains aCurr. Thus, stop
                        // searching, because aCurr is definitely not
                        // a member of the unique rect list
                        aFound = aCurrScan;
                        break;
                    }

                    ++aCurrScan;
                }

                if( aFound == aEnd )
                {
                    // check whether aCurr is fully contained in one
                    // of the already added rects. If yes, we can skip
                    // it.
                    bool bUnique( true );
                    VectorOfRanges::const_iterator aCurrUnique( aUniqueRanges.begin() );
                    VectorOfRanges::const_iterator aEndUnique ( aUniqueRanges.end() );
                    while( aCurrUnique != aEndUnique )
                    {
                        if( aCurrUnique->isInside( *aCurr ) )
                        {
                            // fully contained, no need to add
                            bUnique = false;
                            break;
                        }

                        ++aCurrUnique;
                    }

                    if( bUnique )
                        aUniqueRanges.push_back( *aCurr );
                }

                ++aCurr;
            }

            VectorOfRanges::const_iterator       aCurrUnique( aUniqueRanges.begin() );
            const VectorOfRanges::const_iterator aEndUnique ( aUniqueRanges.end() );
            while( aCurrUnique != aEndUnique )
            {
                // simply merge all unique parts (OR)
                aRes.append( tools::createPolygonFromRect( *aCurrUnique++ ) );
            }

            // remove redundant intersections. Note: since all added
            // rectangles are positively oriented, this method cannot
            // generate any holes.
            aRes = basegfx::tools::solveCrossovers(aRes);
            aRes = basegfx::tools::stripNeutralPolygons(aRes);
            aRes = basegfx::tools::stripDispensablePolygons(aRes, false);

            return aRes;
        }

    private:
        typedef ::std::vector< B2DRange >   VectorOfRanges;

        B2DRange        maBounds;
        VectorOfRanges  maRanges;
    };


    // ====================================================================


    B2DMultiRange::B2DMultiRange() :
        mpImpl()
    {
    }

    B2DMultiRange::B2DMultiRange( const B2DRange& rRange ) :
        mpImpl( ImplB2DMultiRange( rRange ) )
    {
    }

    B2DMultiRange::~B2DMultiRange()
    {
        // otherwise, ImplB2DMultiRange would be an incomplete type
    }

    B2DMultiRange::B2DMultiRange( const B2DMultiRange& rSrc ) :
        mpImpl( rSrc.mpImpl )
    {
    }

    B2DMultiRange& B2DMultiRange::operator=( const B2DMultiRange& rSrc )
    {
        mpImpl = rSrc.mpImpl;
        return *this;
    }

    bool B2DMultiRange::isEmpty() const
    {
        return mpImpl->isEmpty();
    }

    void B2DMultiRange::reset()
    {
        mpImpl->reset();
    }

    bool B2DMultiRange::isInside( const B2DTuple& rTuple ) const
    {
        return mpImpl->isInside( rTuple );
    }

    bool B2DMultiRange::isInside( const B2DRange& rRange ) const
    {
        return mpImpl->isInside( rRange );
    }

    bool B2DMultiRange::overlaps( const B2DRange& rRange ) const
    {
        return mpImpl->overlaps( rRange );
    }

    void B2DMultiRange::addRange( const B2DRange& rRange )
    {
        mpImpl->addRange( rRange );
    }

    B2DRange B2DMultiRange::getBounds() const
    {
        return mpImpl->getBounds();
    }

    B2DPolyPolygon B2DMultiRange::getPolyPolygon() const
    {
        return mpImpl->getPolyPolygon();
    }

} // end of namespace basegfx

// eof
