/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rangeexpander.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:34:56 $
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

#ifndef _BGFX_RANGE_RANGEEXPANDER_HXX
#define _BGFX_RANGE_RANGEEXPANDER_HXX

#ifndef _BGFX_RANGE_B1DRANGE_HXX
#include <basegfx/range/b1drange.hxx>
#endif
#ifndef _BGFX_RANGE_B1IRANGE_HXX
#include <basegfx/range/b1irange.hxx>
#endif
#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif
#ifndef _BGFX_RANGE_B2IRANGE_HXX
#include <basegfx/range/b2irange.hxx>
#endif
#ifndef _BGFX_RANGE_B3DRANGE_HXX
#include <basegfx/range/b3drange.hxx>
#endif
#ifndef _BGFX_RANGE_B3IRANGE_HXX
#include <basegfx/range/b3irange.hxx>
#endif

namespace basegfx
{
    /** Generic functor for expanding a range with a number of other
        ranges.

        Since *Range::expand() is overloaded, straight-forward
        application of ::boost::bind and friends fails (because of
        ambiguities). Thus, this functor template can be used, to
        expand the given range with a number of other ranges, passed
        in at the function operator.

        @tpl RangeType
        Range type to operate with. Preferrably, one of B1*Range,
        B2*Range, or B3*Range.
    */
    template< typename RangeType > class RangeExpander
    {
    public:
        typedef RangeType   ValueType;
        typedef void        result_type;

        explicit RangeExpander( ValueType& rBounds ) :
            mrBounds( rBounds )
        {
        }

        void operator()( const ValueType& rBounds )
        {
            mrBounds.expand( rBounds );
        }

    private:
        ValueType& mrBounds;
    };

    typedef RangeExpander< B1DRange > B1DRangeExpander;
    typedef RangeExpander< B1IRange > B1IRangeExpander;
    typedef RangeExpander< B2DRange > B2DRangeExpander;
    typedef RangeExpander< B2IRange > B2IRangeExpander;
    typedef RangeExpander< B3DRange > B3DRangeExpander;
    typedef RangeExpander< B3IRange > B3IRangeExpander;

} // end of namespace basegfx


#endif /* _BGFX_RANGE_RANGEEXPANDER_HXX */
