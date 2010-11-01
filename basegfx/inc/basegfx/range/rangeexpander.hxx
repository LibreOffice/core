/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

#ifndef _BGFX_RANGE_RANGEEXPANDER_HXX
#define _BGFX_RANGE_RANGEEXPANDER_HXX

#include <basegfx/range/b1drange.hxx>
#include <basegfx/range/b1irange.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/range/b3drange.hxx>
#include <basegfx/range/b3irange.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
