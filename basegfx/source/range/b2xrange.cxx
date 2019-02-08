/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>

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
            std::vector< RangeType >& o_rRanges,
            const RangeType&            a,
            const RangeType&            b )
        {
            o_rRanges.clear();

            // special-casing the empty rect case (this will fail most
            // of the times below, because of the DBL_MIN/MAX special
            // values denoting emptiness in the rectangle.
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

    std::vector< B2IRange >& computeSetDifference( std::vector< B2IRange >& o_rResult,
                                                     const B2IRange&            rFirst,
                                                     const B2IRange&            rSecond )
    {
        doComputeSetDifference( o_rResult, rFirst, rSecond );

        return o_rResult;
    }

    std::vector< B2DRange >& computeSetDifference( std::vector< B2DRange >& o_rResult,
                                                     const B2DRange&            rFirst,
                                                     const B2DRange&            rSecond )
    {
        doComputeSetDifference( o_rResult, rFirst, rSecond );

        return o_rResult;
    }

} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
