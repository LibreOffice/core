/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: verifyinput.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_canvas.hxx"

#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/geometry/Matrix2D.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/geometry/IntegerPoint2D.hpp>
#include <com/sun/star/geometry/IntegerSize2D.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <com/sun/star/geometry/RealBezierSegment2D.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/ViewState.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/Endianness.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/IntegerBitmapFormat.hpp>
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <com/sun/star/rendering/FloatingPointBitmapFormat.hpp>
#include <com/sun/star/rendering/FloatingPointBitmapLayout.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <canvas/verifyinput.hxx>
#include <canvas/canvastools.hxx>


using namespace ::com::sun::star;

namespace canvas
{
    namespace tools
    {
        void verifyInput( const geometry::RealPoint2D&              rPoint,
                          const char*                               pStr,
                          const uno::Reference< uno::XInterface >&  xIf,
                          ::sal_Int16                               nArgPos )
        {
            (void)pStr; (void)xIf; (void)nArgPos;

#if OSL_DEBUG_LEVEL > 0
            if( !::rtl::math::isFinite( rPoint.X ) )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): point X value contains infinite or NAN"),
                    xIf,
                    nArgPos );
            }

            if( !::rtl::math::isFinite( rPoint.Y ) )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): point X value contains infinite or NAN"),
                    xIf,
                    nArgPos );
            }
#else
            if( !::rtl::math::isFinite( rPoint.X ) ||
                !::rtl::math::isFinite( rPoint.Y ) )
            {
                throw lang::IllegalArgumentException();
            }
#endif
        }

        void verifyInput( const geometry::RealSize2D&               rSize,
                          const char*                               pStr,
                          const uno::Reference< uno::XInterface >&  xIf,
                          ::sal_Int16                               nArgPos )
        {
            (void)pStr; (void)xIf; (void)nArgPos;

#if OSL_DEBUG_LEVEL > 0
            if( !::rtl::math::isFinite( rSize.Width ) )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): size.Width value contains infinite or NAN"),
                    xIf,
                    nArgPos );
            }

            if( !::rtl::math::isFinite( rSize.Height ) )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): size.Height value contains infinite or NAN"),
                    xIf,
                    nArgPos );
            }
#else
            if( !::rtl::math::isFinite( rSize.Width ) ||
                !::rtl::math::isFinite( rSize.Height ) )
            {
                throw lang::IllegalArgumentException();
            }
#endif
        }

        void verifyInput( const geometry::RealBezierSegment2D&      rSegment,
                          const char*                               pStr,
                          const uno::Reference< uno::XInterface >&  xIf,
                          ::sal_Int16                               nArgPos )
        {
            (void)pStr; (void)xIf; (void)nArgPos;

#if OSL_DEBUG_LEVEL > 0
            if( !::rtl::math::isFinite( rSegment.Px ) )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bezier segment's Px value contains infinite or NAN"),
                    xIf,
                    nArgPos );
            }

            if( !::rtl::math::isFinite( rSegment.Py ) )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bezier segment's Py value contains infinite or NAN"),
                    xIf,
                    nArgPos );
            }

            if( !::rtl::math::isFinite( rSegment.C1x ) )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bezier segment's C1x value contains infinite or NAN"),
                    xIf,
                    nArgPos );
            }

            if( !::rtl::math::isFinite( rSegment.C1y ) )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bezier segment's C1y value contains infinite or NAN"),
                    xIf,
                    nArgPos );
            }

            if( !::rtl::math::isFinite( rSegment.C2x ) )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bezier segment's C2x value contains infinite or NAN"),
                    xIf,
                    nArgPos );
            }

            if( !::rtl::math::isFinite( rSegment.C2y ) )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bezier segment's C2y value contains infinite or NAN"),
                    xIf,
                    nArgPos );
            }
#else
            if( !::rtl::math::isFinite( rSegment.Px ) ||
                !::rtl::math::isFinite( rSegment.Py ) ||
                !::rtl::math::isFinite( rSegment.C1x ) ||
                !::rtl::math::isFinite( rSegment.C1y ) ||
                !::rtl::math::isFinite( rSegment.C2x ) ||
                !::rtl::math::isFinite( rSegment.C2y ) )
            {
                throw lang::IllegalArgumentException();
            }
#endif
        }

        void verifyInput( const geometry::RealRectangle2D&          rRect,
                          const char*                               pStr,
                          const uno::Reference< uno::XInterface >&  xIf,
                          ::sal_Int16                               nArgPos )
        {
            (void)pStr; (void)xIf; (void)nArgPos;

#if OSL_DEBUG_LEVEL > 0
            if( !::rtl::math::isFinite( rRect.X1 ) )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): rectangle point X1 contains infinite or NAN"),
                    xIf,
                    nArgPos );
            }

            if( !::rtl::math::isFinite( rRect.Y1 ) )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): rectangle point Y1 contains infinite or NAN"),
                    xIf,
                    nArgPos );
            }

            if( !::rtl::math::isFinite( rRect.X2 ) )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): rectangle point X2 contains infinite or NAN"),
                    xIf,
                    nArgPos );
            }

            if( !::rtl::math::isFinite( rRect.Y2 ) )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): rectangle point Y2 contains infinite or NAN"),
                    xIf,
                    nArgPos );
            }
#else
            if( !::rtl::math::isFinite( rRect.X1 ) ||
                !::rtl::math::isFinite( rRect.Y1 ) ||
                !::rtl::math::isFinite( rRect.X2 ) ||
                !::rtl::math::isFinite( rRect.Y2 ) )
            {
                throw lang::IllegalArgumentException();
            }
#endif
        }

        void verifyInput( const geometry::AffineMatrix2D&           matrix,
                          const char*                               pStr,
                          const uno::Reference< uno::XInterface >&  xIf,
                          ::sal_Int16                               nArgPos )
        {
            (void)pStr; (void)xIf; (void)nArgPos;

#if OSL_DEBUG_LEVEL > 0
            const sal_Int32 nBinaryState(
                100000 * !::rtl::math::isFinite( matrix.m00 ) +
                 10000 * !::rtl::math::isFinite( matrix.m01 ) +
                  1000 * !::rtl::math::isFinite( matrix.m02 ) +
                   100 * !::rtl::math::isFinite( matrix.m10 ) +
                    10 * !::rtl::math::isFinite( matrix.m11 ) +
                     1 * !::rtl::math::isFinite( matrix.m12 ) );

            if( nBinaryState )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): AffineMatrix2D contains infinite or NAN value(s) at the following positions (m00-m12): ") +
                    ::rtl::OUString::valueOf(nBinaryState),
                    xIf,
                    nArgPos );
            }
#else
            if( !::rtl::math::isFinite( matrix.m00 ) ||
                !::rtl::math::isFinite( matrix.m01 ) ||
                !::rtl::math::isFinite( matrix.m02 ) ||
                !::rtl::math::isFinite( matrix.m10 ) ||
                !::rtl::math::isFinite( matrix.m11 ) ||
                !::rtl::math::isFinite( matrix.m12 ) )
            {
                throw lang::IllegalArgumentException();
            }
#endif
        }

        void verifyInput( const geometry::Matrix2D&                 matrix,
                          const char*                               pStr,
                          const uno::Reference< uno::XInterface >&  xIf,
                          ::sal_Int16                               nArgPos )
        {
            (void)pStr; (void)xIf; (void)nArgPos;

#if OSL_DEBUG_LEVEL > 0
            const sal_Int32 nBinaryState(
                1000 * !::rtl::math::isFinite( matrix.m00 ) +
                 100 * !::rtl::math::isFinite( matrix.m01 ) +
                  10 * !::rtl::math::isFinite( matrix.m10 ) +
                   1 * !::rtl::math::isFinite( matrix.m11 ) );

            if( nBinaryState )
            {
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): Matrix2D contains infinite or NAN value(s) at the following positions (m00-m11): ") +
                    ::rtl::OUString::valueOf(nBinaryState),
                    xIf,
                    nArgPos );
            }
#else
            if( !::rtl::math::isFinite( matrix.m00 ) ||
                !::rtl::math::isFinite( matrix.m01 ) ||
                !::rtl::math::isFinite( matrix.m10 ) ||
                !::rtl::math::isFinite( matrix.m11 ) )
            {
                throw lang::IllegalArgumentException();
            }
#endif
        }

        void verifyInput( const rendering::ViewState&               viewState,
                          const char*                               pStr,
                          const uno::Reference< uno::XInterface >&  xIf,
                          ::sal_Int16                               nArgPos )
        {
            verifyInput( viewState.AffineTransform,
                         pStr, xIf, nArgPos );
        }

        void verifyInput( const rendering::RenderState&             renderState,
                          const char*                               pStr,
                          const uno::Reference< uno::XInterface >&  xIf,
                          ::sal_Int16                               nArgPos,
                          sal_Int32                                 nMinColorComponents )
        {
            verifyInput( renderState.AffineTransform,
                         pStr, xIf, nArgPos );

            if( renderState.DeviceColor.getLength() < nMinColorComponents )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): render state's device color has too few components (") +
                    ::rtl::OUString::valueOf(nMinColorComponents) +
                    ::rtl::OUString::createFromAscii(" expected, ") +
                    ::rtl::OUString::valueOf(renderState.DeviceColor.getLength()) +
                    ::rtl::OUString::createFromAscii(" provided)"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( renderState.CompositeOperation < rendering::CompositeOperation::CLEAR ||
                renderState.CompositeOperation > rendering::CompositeOperation::SATURATE )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): render state's CompositeOperation value out of range (") +
                    ::rtl::OUString::valueOf(sal::static_int_cast<sal_Int32>(renderState.CompositeOperation)) +
                    ::rtl::OUString::createFromAscii(" not known)"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }
        }

        void verifyInput( const rendering::Texture&                 texture,
                          const char*                               pStr,
                          const uno::Reference< uno::XInterface >&  xIf,
                          ::sal_Int16                               nArgPos )
        {
            verifyInput( texture.AffineTransform,
                         pStr, xIf, nArgPos );

            if( !::rtl::math::isFinite( texture.Alpha ) ||
                texture.Alpha < 0.0 ||
                texture.Alpha > 1.0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): textures' alpha value out of range (is ") +
                    ::rtl::OUString::valueOf(texture.Alpha) +
                    ::rtl::OUString::createFromAscii(")"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( texture.NumberOfHatchPolygons < 0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): textures' NumberOfHatchPolygons is negative"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( texture.RepeatModeX < rendering::TexturingMode::CLAMP ||
                texture.RepeatModeX > rendering::TexturingMode::REPEAT )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): textures' RepeatModeX value is out of range (") +
                    ::rtl::OUString::valueOf(sal::static_int_cast<sal_Int32>(texture.RepeatModeX)) +
                    ::rtl::OUString::createFromAscii(" not known)"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( texture.RepeatModeY < rendering::TexturingMode::CLAMP ||
                texture.RepeatModeY > rendering::TexturingMode::REPEAT )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): textures' RepeatModeY value is out of range (") +
                    ::rtl::OUString::valueOf(sal::static_int_cast<sal_Int32>(texture.RepeatModeY)) +
                    ::rtl::OUString::createFromAscii(" not known)"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }
        }

        namespace
        {
            struct VerifyDashValue
            {
                VerifyDashValue( const char*                                pStr,
                                 const uno::Reference< uno::XInterface >&   xIf,
                                 ::sal_Int16                                nArgPos ) :
                    mpStr( pStr ),
                    mrIf( xIf ),
                    mnArgPos( nArgPos )
                {
                }

                void operator()( const double& rVal )
                {
                    if( !::rtl::math::isFinite( rVal ) || rVal < 0.0 )
                    {
#if OSL_DEBUG_LEVEL > 0
                        throw lang::IllegalArgumentException(
                            ::rtl::OUString::createFromAscii(mpStr) +
                            ::rtl::OUString::createFromAscii(": verifyInput(): one of stroke attributes' DashArray value out of range (is ") +
                            ::rtl::OUString::valueOf(rVal) +
                            ::rtl::OUString::createFromAscii(")"),
                            mrIf,
                            mnArgPos );
#else
                        throw lang::IllegalArgumentException();
#endif
                    }
                }

                const char*                                 mpStr;
                const uno::Reference< uno::XInterface >&    mrIf;
                sal_Int16                                   mnArgPos;
            };
        }

        void verifyInput( const rendering::StrokeAttributes&        strokeAttributes,
                          const char*                               pStr,
                          const uno::Reference< uno::XInterface >&  xIf,
                          ::sal_Int16                               nArgPos )
        {
            if( !::rtl::math::isFinite( strokeAttributes.StrokeWidth ) ||
                strokeAttributes.StrokeWidth < 0.0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): stroke attributes' StrokeWidth value out of range (is ") +
                    ::rtl::OUString::valueOf(strokeAttributes.StrokeWidth) +
                    ::rtl::OUString::createFromAscii(")"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( !::rtl::math::isFinite( strokeAttributes.MiterLimit ) ||
                strokeAttributes.MiterLimit < 0.0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): stroke attributes' MiterLimit value out of range (is ") +
                    ::rtl::OUString::valueOf(strokeAttributes.MiterLimit) +
                    ::rtl::OUString::createFromAscii(")"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            ::std::for_each( strokeAttributes.DashArray.getConstArray(),
                             strokeAttributes.DashArray.getConstArray() + strokeAttributes.DashArray.getLength(),
                             VerifyDashValue( pStr, xIf, nArgPos ) );

            ::std::for_each( strokeAttributes.LineArray.getConstArray(),
                             strokeAttributes.LineArray.getConstArray() + strokeAttributes.LineArray.getLength(),
                             VerifyDashValue( pStr, xIf, nArgPos ) );

            if( strokeAttributes.StartCapType < rendering::PathCapType::BUTT ||
                strokeAttributes.StartCapType > rendering::PathCapType::SQUARE )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): stroke attributes' StartCapType value is out of range (") +
                    ::rtl::OUString::valueOf(sal::static_int_cast<sal_Int32>(strokeAttributes.StartCapType)) +
                    ::rtl::OUString::createFromAscii(" not known)"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( strokeAttributes.EndCapType < rendering::PathCapType::BUTT ||
                strokeAttributes.EndCapType > rendering::PathCapType::SQUARE )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): stroke attributes' StartCapType value is out of range (") +
                    ::rtl::OUString::valueOf(sal::static_int_cast<sal_Int32>(strokeAttributes.EndCapType)) +
                    ::rtl::OUString::createFromAscii(" not known)"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( strokeAttributes.JoinType < rendering::PathJoinType::NONE ||
                strokeAttributes.JoinType > rendering::PathJoinType::BEVEL )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): stroke attributes' JoinType value is out of range (") +
                    ::rtl::OUString::valueOf(sal::static_int_cast<sal_Int32>(strokeAttributes.JoinType)) +
                    ::rtl::OUString::createFromAscii(" not known)"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }
        }

        void verifyInput( const rendering::IntegerBitmapLayout&     bitmapLayout,
                          const char*                               pStr,
                          const uno::Reference< uno::XInterface >&  xIf,
                          ::sal_Int16                               nArgPos )
        {
            (void)pStr; (void)xIf; (void)nArgPos;

            if( bitmapLayout.ScanLines < 0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bitmap layout's ScanLines is negative"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( bitmapLayout.ScanLineBytes < 0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bitmap layout's ScanLineBytes is negative"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( !bitmapLayout.ColorSpace.is() )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bitmap layout's ColorSpace is invalid"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( bitmapLayout.NumComponents < 0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bitmap layout's NumComponents is negative"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( bitmapLayout.ComponentMasks.getLength() != bitmapLayout.NumComponents )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bitmap layout's number of ComponentMasks elements mismatch NumComponents (is: ") +
                    ::rtl::OUString::valueOf(bitmapLayout.ComponentMasks.getLength()) +
                    ::rtl::OUString::createFromAscii(" expected: ") +
                    ::rtl::OUString::valueOf(bitmapLayout.NumComponents) +
                    ::rtl::OUString::createFromAscii(")"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( bitmapLayout.Endianness < rendering::Endianness::LITTLE ||
                bitmapLayout.Endianness > rendering::Endianness::BIG )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bitmap layout's Endianness value is out of range (") +
                    ::rtl::OUString::valueOf(sal::static_int_cast<sal_Int32>(bitmapLayout.Endianness)) +
                    ::rtl::OUString::createFromAscii(" not known)"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( bitmapLayout.Format < rendering::IntegerBitmapFormat::CHUNKY_1BIT ||
                bitmapLayout.Format > rendering::IntegerBitmapFormat::PLANES_64BIT )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bitmap layout's Format value is out of range (") +
                    ::rtl::OUString::valueOf(sal::static_int_cast<sal_Int32>(bitmapLayout.Format)) +
                    ::rtl::OUString::createFromAscii(" not known)"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }
        }

        void verifyInput( const rendering::FloatingPointBitmapLayout&   bitmapLayout,
                          const char*                                   pStr,
                          const uno::Reference< uno::XInterface >&      xIf,
                          ::sal_Int16                                   nArgPos )
        {
            (void)pStr; (void)xIf; (void)nArgPos;

            if( bitmapLayout.ScanLines < 0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bitmap layout's ScanLines is negative"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( bitmapLayout.ScanLineBytes < 0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bitmap layout's ScanLineBytes is negative"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( !bitmapLayout.ColorSpace.is() )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bitmap layout's ColorSpace is invalid"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( bitmapLayout.NumComponents < 0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bitmap layout's NumComponents is negative"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( bitmapLayout.Endianness < rendering::Endianness::LITTLE ||
                bitmapLayout.Endianness > rendering::Endianness::BIG )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bitmap layout's Endianness value is out of range (") +
                    ::rtl::OUString::valueOf(sal::static_int_cast<sal_Int32>(bitmapLayout.Endianness)) +
                    ::rtl::OUString::createFromAscii(" not known)"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( bitmapLayout.Format < rendering::IntegerBitmapFormat::CHUNKY_1BIT ||
                bitmapLayout.Format > rendering::IntegerBitmapFormat::PLANES_64BIT )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): bitmap layout's Format value is out of range (") +
                    ::rtl::OUString::valueOf(sal::static_int_cast<sal_Int32>(bitmapLayout.Format)) +
                    ::rtl::OUString::createFromAscii(" not known)"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }
        }

        void verifyInput( const rendering::FontInfo&                /*fontInfo*/,
                          const char*                               /*pStr*/,
                          const uno::Reference< uno::XInterface >&  /*xIf*/,
                          ::sal_Int16                               /*nArgPos*/ )
        {
            // TODO(E3): Implement FontDescription checks, once the
            // Panose stuff is ready.
        }

        void verifyInput( const rendering::FontRequest&             fontRequest,
                          const char*                               pStr,
                          const uno::Reference< uno::XInterface >&  xIf,
                          ::sal_Int16                               nArgPos )
        {
            verifyInput( fontRequest.FontDescription,
                         pStr, xIf, nArgPos );

            if( !::rtl::math::isFinite( fontRequest.CellSize ) )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): font request's CellSize value contains infinite or NAN"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( !::rtl::math::isFinite( fontRequest.ReferenceAdvancement ) )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): font request's ReferenceAdvancement value contains infinite or NAN"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( fontRequest.CellSize != 0.0 &&
                fontRequest.ReferenceAdvancement != 0.0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyInput(): font request's CellSize and ReferenceAdvancement are mutually exclusive, one of them must be 0.0"),
                    xIf,
                    nArgPos );
#else
                throw lang::IllegalArgumentException();
#endif
            }
        }

        void verifyIndexRange( const geometry::IntegerRectangle2D&  rect,
                               const geometry::IntegerSize2D&       size )
        {
            const ::basegfx::B2IRange aRect(
                ::basegfx::unotools::b2IRectangleFromIntegerRectangle2D(
                    rect ) );

            if( aRect.getMinX() < 0 ||
                aRect.getMaxX() > size.Width ||
                aRect.getMinY() < 0 ||
                aRect.getMaxY() > size.Height )
            {
                throw ::com::sun::star::lang::IndexOutOfBoundsException();
            }
        }

        void verifyIndexRange( const geometry::IntegerPoint2D& pos,
                               const geometry::IntegerSize2D&  size )
        {
            if( pos.X < 0 ||
                pos.X > size.Width ||
                pos.Y < 0 ||
                pos.Y > size.Height )
            {
                throw ::com::sun::star::lang::IndexOutOfBoundsException();
            }
        }

        void verifyBitmapSize( const geometry::IntegerSize2D&           size,
                               const char*                              pStr,
                               const uno::Reference< uno::XInterface >& xIf )
        {
            (void)pStr; (void)xIf;

            if( size.Width <= 0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyBitmapSize(): size has 0 or negative width (value: ") +
                    ::rtl::OUString::valueOf(size.Width) +
                    ::rtl::OUString::createFromAscii(")"),
                    xIf,
                    0 );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( size.Height <= 0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifyBitmapSize(): size has 0 or negative height (value: ") +
                    ::rtl::OUString::valueOf(size.Height) +
                    ::rtl::OUString::createFromAscii(")"),
                    xIf,
                    0 );
#else
                throw lang::IllegalArgumentException();
#endif
            }
        }

        void verifySpriteSize( const geometry::RealSize2D&              size,
                               const char*                              pStr,
                               const uno::Reference< uno::XInterface >& xIf )
        {
            (void)pStr; (void)xIf;

            if( size.Width <= 0.0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifySpriteSize(): size has 0 or negative width (value: ") +
                    ::rtl::OUString::valueOf(size.Width) +
                    ::rtl::OUString::createFromAscii(")"),
                    xIf,
                    0 );
#else
                throw lang::IllegalArgumentException();
#endif
            }

            if( size.Height <= 0.0 )
            {
#if OSL_DEBUG_LEVEL > 0
                throw lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString::createFromAscii(": verifySpriteSize(): size has 0 or negative height (value: ") +
                    ::rtl::OUString::valueOf(size.Height) +
                    ::rtl::OUString::createFromAscii(")"),
                    xIf,
                    0 );
#else
                throw lang::IllegalArgumentException();
#endif
            }
        }


    } // namespace tools

} // namespace canvas
