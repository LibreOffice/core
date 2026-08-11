/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/config.h>

#include <basegfx/range/b2irange.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/geometry/IntegerPoint2D.hpp>
#include <com/sun/star/geometry/IntegerSize2D.hpp>
#include <com/sun/star/geometry/Matrix2D.hpp>
#include <com/sun/star/geometry/RealBezierSegment2D.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/rendering/FontRequest.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <RenderState.hxx>
#include <com/sun/star/rendering/StrokeAttributes.hpp>
#include <Texture.hxx>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/util/Endianness.hpp>

#include <verifyinput.hxx>
#include <ViewState.hxx>
#include <canvas.hxx>


using namespace ::com::sun::star;

namespace canvastools
{
        void verifyInput( const geometry::RealPoint2D&              rPoint,
                          const char*                               pStr,
                          ::sal_Int16                               nArgPos )
        {
#if OSL_DEBUG_LEVEL > 0
            if( !std::isfinite( rPoint.X ) )
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii( pStr ) + ": verifyInput(): point X value contains infinite or NAN",
                    nullptr, nArgPos );
            }

            if( !std::isfinite( rPoint.Y ) )
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii( pStr ) + ": verifyInput(): point X value contains infinite or NAN",
                    nullptr, nArgPos );
            }
#else
            (void)pStr; (void)nArgPos;
            if( !std::isfinite( rPoint.X ) ||
                !std::isfinite( rPoint.Y ) )
            {
                throw lang::IllegalArgumentException();
            }
#endif
        }

        void verifyInput( const geometry::AffineMatrix2D&           matrix,
                          const char*                               pStr,
                          ::sal_Int16                               nArgPos )
        {
#if OSL_DEBUG_LEVEL > 0
            const sal_Int32 nBinaryState(
                100000 * int(!std::isfinite( matrix.m00 )) +
                 10000 * int(!std::isfinite( matrix.m01 )) +
                  1000 * int(!std::isfinite( matrix.m02 )) +
                   100 * int(!std::isfinite( matrix.m10 )) +
                    10 * int(!std::isfinite( matrix.m11 )) +
                     1 * int(!std::isfinite( matrix.m12 )) );

            if( nBinaryState )
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii(pStr) +
                    ": verifyInput(): AffineMatrix2D contains infinite or NAN value(s) at the following positions (m00-m12): " +
                    OUString::number(nBinaryState),
                    nullptr, nArgPos );
            }
#else
            (void)pStr; (void)nArgPos;
            if( !std::isfinite( matrix.m00 ) ||
                !std::isfinite( matrix.m01 ) ||
                !std::isfinite( matrix.m02 ) ||
                !std::isfinite( matrix.m10 ) ||
                !std::isfinite( matrix.m11 ) ||
                !std::isfinite( matrix.m12 ) )
            {
                throw lang::IllegalArgumentException();
            }
#endif
        }

        void verifyInput( const geometry::Matrix2D&                 matrix,
                          const char*                               pStr,
                          ::sal_Int16                               nArgPos )
        {
#if OSL_DEBUG_LEVEL > 0
            const sal_Int32 nBinaryState(
                1000 * int(!std::isfinite( matrix.m00 )) +
                 100 * int(!std::isfinite( matrix.m01 )) +
                  10 * int(!std::isfinite( matrix.m10 )) +
                   1 * int(!std::isfinite( matrix.m11 )) );

            if( nBinaryState )
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii(pStr) +
                    ": verifyInput(): Matrix2D contains infinite or NAN value(s) at the following positions (m00-m11): " +
                    OUString::number(nBinaryState),
                    nullptr, nArgPos );
            }
#else
            (void)pStr; (void)nArgPos;
            if( !std::isfinite( matrix.m00 ) ||
                !std::isfinite( matrix.m01 ) ||
                !std::isfinite( matrix.m10 ) ||
                !std::isfinite( matrix.m11 ) )
            {
                throw lang::IllegalArgumentException();
            }
#endif
        }

        void verifyInput( const vclcanvas::ViewState&               viewState,
                          const char*                               pStr,
                          ::sal_Int16                               nArgPos )
        {
            verifyInput( viewState.AffineTransform,
                         pStr, nArgPos );
        }

        void verifyInput( const vclcanvas::RenderState&             renderState,
                          const char*                               pStr,
                          ::sal_Int16                               nArgPos,
                          sal_Int32                                 nMinColorComponents )
        {
            verifyInput( renderState.AffineTransform,
                         pStr, nArgPos );
            if( !renderState.DeviceColor.has_value() && nMinColorComponents > 0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii(pStr) +
                    ": verifyInput(): expected color in render state, but no color",
                    nullptr, nArgPos );
#else
                (void)nMinColorComponents;
                throw lang::IllegalArgumentException();
#endif
            }
        }

        void verifyInput( const vclcanvas::Texture&                 texture,
                          const char*                               pStr,
                          ::sal_Int16                               nArgPos )
        {
            verifyInput( texture.AffineTransform,
                         pStr, nArgPos );

            if( !std::isfinite( texture.Alpha ) ||
                texture.Alpha < 0.0 ||
                texture.Alpha > 1.0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii(pStr) +
                    ": verifyInput(): textures' alpha value out of range (is " +
                    OUString::number(texture.Alpha) + ")",
                    nullptr, nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( texture.RepeatModeX < rendering::TexturingMode::NONE ||
                texture.RepeatModeX > rendering::TexturingMode::REPEAT )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii(pStr) +
                    ": verifyInput(): textures' RepeatModeX value is out of range (" +
                    OUString::number(sal::static_int_cast<sal_Int32>(texture.RepeatModeX)) +
                    " not known)",
                    nullptr, nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( texture.RepeatModeY >= rendering::TexturingMode::NONE &&
                texture.RepeatModeY <= rendering::TexturingMode::REPEAT )
                return;

#if OSL_DEBUG_LEVEL > 0
            throw lang::IllegalArgumentException(
                OUString::createFromAscii(pStr) +
                ": verifyInput(): textures' RepeatModeY value is out of range (" +
                OUString::number(sal::static_int_cast<sal_Int32>(texture.RepeatModeY)) +
                " not known)",
                nullptr, nArgPos );
#else
            throw lang::IllegalArgumentException();
#endif
        }

        namespace
        {
            struct VerifyDashValue
            {
                VerifyDashValue( const char*                                pStr,
                                 ::sal_Int16                                nArgPos ) :
                    mpStr( pStr ),
                    mnArgPos( nArgPos )
                {
                }

                void operator()( const double& rVal )
                {
                    if( !std::isfinite( rVal ) || rVal < 0.0 )
                    {
                        throw lang::IllegalArgumentException(
                            OUString::createFromAscii(mpStr) +
                            ": verifyInput(): one of stroke attributes' DashArray value out of range (is " +
                            OUString::number(rVal) + ")",
                            nullptr, mnArgPos );
                    }
                }

                const char*                                 mpStr;
                sal_Int16                                   mnArgPos;
            };
        }

        void verifyInput( const rendering::StrokeAttributes&        strokeAttributes,
                          const char*                               pStr,
                          ::sal_Int16                               nArgPos )
        {
            if( !std::isfinite( strokeAttributes.StrokeWidth ) ||
                strokeAttributes.StrokeWidth < 0.0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii(pStr) +
                    ": verifyInput(): stroke attributes' StrokeWidth value out of range (is " +
                    OUString::number(strokeAttributes.StrokeWidth) +
                    ")",
                    nullptr, nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( !std::isfinite( strokeAttributes.MiterLimit ) ||
                strokeAttributes.MiterLimit < 0.0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii(pStr) +
                    ": verifyInput(): stroke attributes' MiterLimit value out of range (is " +
                    OUString::number(strokeAttributes.MiterLimit) + ")",
                    nullptr, nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            VerifyDashValue aVerifyDashValue( pStr, nArgPos );
            for (auto const& aStrokeAttribute : strokeAttributes.DashArray)
                aVerifyDashValue( aStrokeAttribute );

            for (auto const& aStrokeAttribute : strokeAttributes.LineArray)
                aVerifyDashValue( aStrokeAttribute );

            if( strokeAttributes.StartCapType < rendering::PathCapType::BUTT ||
                strokeAttributes.StartCapType > rendering::PathCapType::SQUARE )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii(pStr) +
                    ": verifyInput(): stroke attributes' StartCapType value is out of range (" +
                    OUString::number(sal::static_int_cast<sal_Int32>(strokeAttributes.StartCapType)) +
                    " not known)",
                    nullptr, nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( strokeAttributes.EndCapType < rendering::PathCapType::BUTT ||
                strokeAttributes.EndCapType > rendering::PathCapType::SQUARE )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii(pStr) +
                    ": verifyInput(): stroke attributes' StartCapType value is out of range (" +
                    OUString::number(sal::static_int_cast<sal_Int32>(strokeAttributes.EndCapType)) +
                    " not known)",
                    nullptr, nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( strokeAttributes.JoinType >= rendering::PathJoinType::NONE &&
                strokeAttributes.JoinType <= rendering::PathJoinType::BEVEL )
                return;

#if OSL_DEBUG_LEVEL > 0
            throw lang::IllegalArgumentException(
                OUString::createFromAscii(pStr) +
                ": verifyInput(): stroke attributes' JoinType value is out of range (" +
                OUString::number(sal::static_int_cast<sal_Int32>(strokeAttributes.JoinType)) +
                " not known)",
                nullptr, nArgPos );
#else
            throw lang::IllegalArgumentException();
#endif
        }

        void verifyInput( const rendering::FontRequest&             fontRequest,
                          const char*                               pStr,
                          ::sal_Int16                               nArgPos )
        {
            verifyInput( fontRequest.FontDescription,
                         pStr, nArgPos );

            if( !std::isfinite( fontRequest.CellSize ) )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii(pStr) +
                    ": verifyInput(): font request's CellSize value contains infinite or NAN",
                    nullptr, nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( !std::isfinite( fontRequest.ReferenceAdvancement ) )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii(pStr) +
                    ": verifyInput(): font request's ReferenceAdvancement value contains infinite or NAN",
                    nullptr, nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( fontRequest.CellSize != 0.0 &&
                fontRequest.ReferenceAdvancement != 0.0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii(pStr) +
                    ": verifyInput(): font request's CellSize and ReferenceAdvancement are mutually exclusive, one of them must be 0.0",
                    nullptr, nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }
        }

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
