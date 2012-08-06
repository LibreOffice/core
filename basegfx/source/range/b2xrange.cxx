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

#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/range/b2ibox.hxx>


namespace basegfx
{
    namespace
    {
        /** Generic implementation of the difference set computation

            @tpl RangeType
            Type to operate on. Must provide ValueType and TraitsType
            nested types.
         */
        template< class RangeType > void doComputeSetDifference(
            ::std::vector< RangeType >& o_rRanges,
            const RangeType&            a,
            const RangeType&            b )
        {
            o_rRanges.clear();

            // special-casing the empty rect case (this will fail most
            // of the times below, because of the DBL_MIN/MAX special
            // values denoting emptyness in the rectangle.
            if( a.isEmpty() )
            {
                o_rRanges.push_back( b );
                return;
            }
            if( b.isEmpty() )
            {
                o_rRanges.push_back( a );
                return;
            }

            const typename RangeType::ValueType                     ax(a.getMinX());
            const typename RangeType::ValueType                     ay(a.getMinY());
            const typename RangeType::TraitsType::DifferenceType    aw(a.getWidth());
            const typename RangeType::TraitsType::DifferenceType    ah(a.getHeight());
            const typename RangeType::ValueType                     bx(b.getMinX());
            const typename RangeType::ValueType                     by(b.getMinY());
            const typename RangeType::TraitsType::DifferenceType    bw(b.getWidth());
            const typename RangeType::TraitsType::DifferenceType    bh(b.getHeight());

            const typename RangeType::TraitsType::DifferenceType    h0( (by > ay) ? by - ay : 0 );
            const typename RangeType::TraitsType::DifferenceType    h3( (by + bh < ay + ah) ? ay + ah - by - bh : 0 );
            const typename RangeType::TraitsType::DifferenceType    w1( (bx > ax) ? bx - ax : 0 );
            const typename RangeType::TraitsType::DifferenceType    w2( (ax + aw > bx + bw) ? ax + aw - bx - bw : 0 );
            const typename RangeType::TraitsType::DifferenceType    h12( (h0 + h3 < ah) ? ah - h0 - h3 : 0 );

            // TODO(E2): Use numeric_cast instead of static_cast here,
            // need range checks!
            if (h0 > 0)
                o_rRanges.push_back(
                    RangeType(ax,ay,
                              static_cast<typename RangeType::ValueType>(ax+aw),
                              static_cast<typename RangeType::ValueType>(ay+h0)) );

            if (w1 > 0 && h12 > 0)
                o_rRanges.push_back(
                    RangeType(ax,
                              static_cast<typename RangeType::ValueType>(ay+h0),
                              static_cast<typename RangeType::ValueType>(ax+w1),
                              static_cast<typename RangeType::ValueType>(ay+h0+h12)) );

            if (w2 > 0 && h12 > 0)
                o_rRanges.push_back(
                    RangeType(static_cast<typename RangeType::ValueType>(bx+bw),
                              static_cast<typename RangeType::ValueType>(ay+h0),
                              static_cast<typename RangeType::ValueType>(bx+bw+w2),
                              static_cast<typename RangeType::ValueType>(ay+h0+h12)) );

            if (h3 > 0)
                o_rRanges.push_back(
                    RangeType(ax,
                              static_cast<typename RangeType::ValueType>(ay+h0+h12),
                              static_cast<typename RangeType::ValueType>(ax+aw),
                              static_cast<typename RangeType::ValueType>(ay+h0+h12+h3)) );
        }
    }

    ::std::vector< B2IRange >& computeSetDifference( ::std::vector< B2IRange >& o_rResult,
                                                     const B2IRange&            rFirst,
                                                     const B2IRange&            rSecond )
    {
        doComputeSetDifference( o_rResult, rFirst, rSecond );

        return o_rResult;
    }

    ::std::vector< B2DRange >& computeSetDifference( ::std::vector< B2DRange >& o_rResult,
                                                     const B2DRange&            rFirst,
                                                     const B2DRange&            rSecond )
    {
        doComputeSetDifference( o_rResult, rFirst, rSecond );

        return o_rResult;
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
