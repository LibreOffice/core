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

#ifndef INCLUDED_CANVAS_VERIFYINPUT_HXX
#define INCLUDED_CANVAS_VERIFYINPUT_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/current_function.hpp>

#include <canvas/canvastoolsdllapi.h>

namespace com { namespace sun { namespace star { namespace geometry
{
    struct RealPoint2D;
    struct RealSize2D;
    struct RealBezierSegment2D;
    struct RealRectangle2D;
    struct AffineMatrix2D;
    struct Matrix2D;
    struct IntegerPoint2D;
    struct IntegerSize2D;
    struct IntegerRectangle2D;
} } } }

namespace com { namespace sun { namespace star { namespace rendering
{
    struct RenderState;
    struct StrokeAttributes;
    struct Texture;
    struct ViewState;
    struct IntegerBitmapLayout;
    struct FontRequest;
    struct FontInfo;
    class  XCanvas;
} } } }


namespace canvas
{
    namespace tools
    {

        // Input checking facilities
        // ===================================================================

        // This header provides methods to check all common
        // css::rendering::* method input parameters against
        // compliance to the API specification.

        /** Verify that the given transformation contains valid floating point
            values.

            @param rMatrix
            Matrix to check

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::com::sun::star::geometry::AffineMatrix2D&   rMatrix,
                          const char*                                       pStr,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::uno::XInterface >&          xIf,
                          ::sal_Int16                                       nArgPos );

        /** Verify that the given transformation contains valid floating point
            values.

            @param rMatrix
            Matrix to check

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::com::sun::star::geometry::Matrix2D& rMatrix,
                          const char*                                   pStr,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::uno::XInterface >&      xIf,
                          ::sal_Int16                                   nArgPos );

        /** Verify that the given point contains valid floating point
            values.

            @param rPoint
            Point to check

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::com::sun::star::geometry::RealPoint2D&  rPoint,
                          const char*                                       pStr,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::uno::XInterface >&          xIf,
                          ::sal_Int16                                       nArgPos );

        /** Verify that the given bezier segment contains valid
            floating point values.

            @param rSegment
            Segment to check

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::com::sun::star::geometry::RealBezierSegment2D&  rSegment,
                          const char*                                               pStr,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::uno::XInterface >&                  xIf,
                          ::sal_Int16                                               nArgPos );

        /** Verify that the given point contains valid floating point
            values.

            @param rPoint
            Point to check

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::com::sun::star::geometry::RealPoint2D&  rPoint,
                          const char*                                       pStr,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::uno::XInterface >&          xIf,
                          ::sal_Int16                                       nArgPos );

        /** Verify that the given rectangle contains valid floating
            point values.

            @param rRect
            Rect to check

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::com::sun::star::geometry::RealRectangle2D&  rRect,
                          const char*                                           pStr,
                          const ::com::sun::star::uno::Reference<
                              ::com::sun::star::uno::XInterface >&              xIf,
                          ::sal_Int16                                           nArgPos );

        /** Basic check for view state validity.

            @param viewState
            Viewstate to check

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::com::sun::star::rendering::ViewState&   viewState,
                          const char*                                   pStr,
                          const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::uno::XInterface >& xIf,
                          ::sal_Int16                                   nArgPos );

        /** Basic check for render state validity.

            @param renderState
            Renderstate to check

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @param nMinColorComponents
            Minimal number of color components available in
            RenderState::DeviceColor

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::com::sun::star::rendering::RenderState& renderState,
                          const char*                                       pStr,
                          const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::uno::XInterface >&     xIf,
                          ::sal_Int16                                       nArgPos,
                          sal_Int32                                         nMinColorComponents=0 );

        /** Basic check for stroke attributes validity.

            @param strokeAttributes
            Attributes to check

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::com::sun::star::rendering::StrokeAttributes&    strokeAttributes,
                          const char*                                           pStr,
                          const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::uno::XInterface >&         xIf,
                          ::sal_Int16                                           nArgPos );

        /** Basic check for texture validity.

            @param texture
            Texture to check

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::com::sun::star::rendering::Texture&     texture,
                          const char*                                   pStr,
                          const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::uno::XInterface >& xIf,
                          ::sal_Int16                                   nArgPos );

        /** Basic check for bitmap layout validity.

            @param bitmapLayout
            Bitmap layout to check

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::com::sun::star::rendering::IntegerBitmapLayout&     bitmapLayout,
                          const char*                                               pStr,
                          const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::uno::XInterface >&             xIf,
                          ::sal_Int16                                               nArgPos );

        /** Basic check for font info validity.

            @param fontInfo
            Font info to check

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::com::sun::star::rendering::FontInfo&    fontInfo,
                          const char*                                   pStr,
                          const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::uno::XInterface >& xIf,
                          ::sal_Int16                                   nArgPos );

        /** Basic check for font request validity.

            @param fontRequest
            Font request to check

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::com::sun::star::rendering::FontRequest& fontRequest,
                          const char*                                       pStr,
                          const ::com::sun::star::uno::Reference<
                                   ::com::sun::star::uno::XInterface >&     xIf,
                          ::sal_Int16                                       nArgPos );

        /** Templatized check for uno::Reference validity.

            @param rRef
            Reference to check against non-NILness

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        template< class Interface > void verifyInput(
            const ::com::sun::star::uno::Reference< Interface >&    rRef,
            const char*                                             pStr,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface >&                xIf,
            ::sal_Int16                                             nArgPos )
        {
            (void)pStr; (void)xIf; (void)nArgPos;

            if( !rRef.is() )
            {
#if OSL_DEBUG_LEVEL > 0
                throw ::com::sun::star::lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii(pStr) +
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": reference is NULL" )),
                    xIf,
                    nArgPos );
#else
                throw ::com::sun::star::lang::IllegalArgumentException();
#endif
            }
        }

        /** Templatized check for content-of-sequence validity.

            @param rSequence
            Sequence of things to check

            @param xIf
            The interface that should be reported as the one
            generating the exception.

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws an lang::IllegalArgumentException, if anything is wrong
         */
        template< typename SequenceContent > void verifyInput(
            const ::com::sun::star::uno::Sequence< SequenceContent >&   rSequence,
            const char*                                                 pStr,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface >&                    xIf,
            ::sal_Int16                                                 nArgPos )
        {
            const SequenceContent* pCurr = rSequence.getConstArray();
            const SequenceContent* pEnd  = pCurr + rSequence.getLength();
            while( pCurr != pEnd )
                verifyInput( *pCurr++, pStr, xIf, nArgPos );
        }

        /// Catch-all, to handle cases that DON'T need input checking (i.e. the Integer geometry ones)
        template< typename T > void verifyInput( const T&                                   /*rDummy*/,
                                                 const char*                                /*pStr*/,
                                                 const ::com::sun::star::uno::Reference<
                                                       ::com::sun::star::uno::XInterface >& /*xIf*/,
                                                 ::sal_Int16                                /*nArgPos*/ )
        {
        }

        // TODO(Q2): Employ some template arglist magic here, to avoid
        // this duplication of code...

        template< typename Arg0 > void verifyArgs( const Arg0&                                  rArg0,
                                                   const char*                                  pStr,
                                                   const ::com::sun::star::uno::Reference<
                                                           ::com::sun::star::uno::XInterface >& xIf )
        {
            verifyInput( rArg0, pStr, xIf, 0 );
        }

        template< typename Arg0,
                  typename Arg1 > void verifyArgs( const Arg0&                                  rArg0,
                                                   const Arg1&                                  rArg1,
                                                   const char*                                  pStr,
                                                   const ::com::sun::star::uno::Reference<
                                                           ::com::sun::star::uno::XInterface >& xIf )
        {
            verifyInput( rArg0, pStr, xIf, 0 );
            verifyInput( rArg1, pStr, xIf, 1 );
        }

        template< typename Arg0,
                  typename Arg1,
                  typename Arg2 > void verifyArgs( const Arg0&                                  rArg0,
                                                   const Arg1&                                  rArg1,
                                                   const Arg2&                                  rArg2,
                                                   const char*                                  pStr,
                                                   const ::com::sun::star::uno::Reference<
                                                           ::com::sun::star::uno::XInterface >& xIf )
        {
            verifyInput( rArg0, pStr, xIf, 0 );
            verifyInput( rArg1, pStr, xIf, 1 );
            verifyInput( rArg2, pStr, xIf, 2 );
        }

        template< typename Arg0,
                  typename Arg1,
                  typename Arg2,
                  typename Arg3 > void verifyArgs( const Arg0&                                  rArg0,
                                                   const Arg1&                                  rArg1,
                                                   const Arg2&                                  rArg2,
                                                   const Arg3&                                  rArg3,
                                                   const char*                                  pStr,
                                                   const ::com::sun::star::uno::Reference<
                                                           ::com::sun::star::uno::XInterface >& xIf )
        {
            verifyInput( rArg0, pStr, xIf, 0 );
            verifyInput( rArg1, pStr, xIf, 1 );
            verifyInput( rArg2, pStr, xIf, 2 );
            verifyInput( rArg3, pStr, xIf, 3 );
        }

        template< typename Arg0,
                  typename Arg1,
                  typename Arg2,
                  typename Arg3,
                  typename Arg4 > void verifyArgs( const Arg0&                                  rArg0,
                                                   const Arg1&                                  rArg1,
                                                   const Arg2&                                  rArg2,
                                                   const Arg3&                                  rArg3,
                                                   const Arg4&                                  rArg4,
                                                   const char*                                  pStr,
                                                   const ::com::sun::star::uno::Reference<
                                                           ::com::sun::star::uno::XInterface >& xIf )
        {
            verifyInput( rArg0, pStr, xIf, 0 );
            verifyInput( rArg1, pStr, xIf, 1 );
            verifyInput( rArg2, pStr, xIf, 2 );
            verifyInput( rArg3, pStr, xIf, 3 );
            verifyInput( rArg4, pStr, xIf, 4 );
        }

        template< typename Arg0,
                  typename Arg1,
                  typename Arg2,
                  typename Arg3,
                  typename Arg4,
                  typename Arg5 > void verifyArgs( const Arg0&                                  rArg0,
                                                   const Arg1&                                  rArg1,
                                                   const Arg2&                                  rArg2,
                                                   const Arg3&                                  rArg3,
                                                   const Arg4&                                  rArg4,
                                                   const Arg5&                                  rArg5,
                                                   const char*                                  pStr,
                                                   const ::com::sun::star::uno::Reference<
                                                           ::com::sun::star::uno::XInterface >& xIf )
        {
            verifyInput( rArg0, pStr, xIf, 0 );
            verifyInput( rArg1, pStr, xIf, 1 );
            verifyInput( rArg2, pStr, xIf, 2 );
            verifyInput( rArg3, pStr, xIf, 3 );
            verifyInput( rArg4, pStr, xIf, 4 );
            verifyInput( rArg5, pStr, xIf, 5 );
        }

        template< typename Arg0,
                  typename Arg1,
                  typename Arg2,
                  typename Arg3,
                  typename Arg4,
                  typename Arg5,
                  typename Arg6 > void verifyArgs( const Arg0&                                  rArg0,
                                                   const Arg1&                                  rArg1,
                                                   const Arg2&                                  rArg2,
                                                   const Arg3&                                  rArg3,
                                                   const Arg4&                                  rArg4,
                                                   const Arg5&                                  rArg5,
                                                   const Arg6&                                  rArg6,
                                                   const char*                                  pStr,
                                                   const ::com::sun::star::uno::Reference<
                                                           ::com::sun::star::uno::XInterface >& xIf )
        {
            verifyInput( rArg0, pStr, xIf, 0 );
            verifyInput( rArg1, pStr, xIf, 1 );
            verifyInput( rArg2, pStr, xIf, 2 );
            verifyInput( rArg3, pStr, xIf, 3 );
            verifyInput( rArg4, pStr, xIf, 4 );
            verifyInput( rArg5, pStr, xIf, 5 );
            verifyInput( rArg6, pStr, xIf, 6 );
        }


        /** Range checker, which throws ::com::sun::star::lang::IllegalArgument exception, when
            range is violated
        */
        template< typename NumType > inline void verifyRange( NumType arg, NumType lowerBound, NumType upperBound )
        {
            if( arg < lowerBound ||
                arg > upperBound )
            {
                throw ::com::sun::star::lang::IllegalArgumentException();
            }
        }

        /** Range checker, which throws ::com::sun::star::lang::IllegalArgument exception, when
            range is violated

            The checked range is half open, i.e. only bound by the specified value.

            @param arg
            Arg to check

            @param bound
            Bound to check against

            @param bLowerBound
            When true, given bound is the lower bound. When false,
            given bound is the upper bound.
        */
        template< typename NumType > inline void verifyRange( NumType arg, NumType bound, bool bLowerBound=true )
        {
            if( (bLowerBound && arg < bound) ||
                (!bLowerBound && arg > bound) )
            {
                throw ::com::sun::star::lang::IllegalArgumentException();
            }
        }

        /** Range checker, which throws ::com::sun::star::lang::IndexOutOfBounds exception, when
            index range is violated
        */
        template< typename NumType > inline void verifyIndexRange( NumType arg, NumType lowerBound, NumType upperBound )
        {
            if( arg < lowerBound ||
                arg > upperBound )
            {
                throw ::com::sun::star::lang::IndexOutOfBoundsException();
            }
        }

        /** Range checker, which throws ::com::sun::star::lang::IndexOutOfBounds exception, when
            index range is violated

            @param rect
            Rect to verify

            @param size
            Given rectangle must be within ((0,0), (size.Width, size.Height))
         */
        CANVASTOOLS_DLLPUBLIC void verifyIndexRange( const ::com::sun::star::geometry::IntegerRectangle2D& rect,
                               const ::com::sun::star::geometry::IntegerSize2D&      size );

        /** Range checker, which throws ::com::sun::star::lang::IndexOutOfBounds exception, when
            index range is violated

            @param pos
            Position to verify

            @param size
            Given position must be within ((0,0), (size.Width, size.Height))
         */
        CANVASTOOLS_DLLPUBLIC void verifyIndexRange( const ::com::sun::star::geometry::IntegerPoint2D& pos,
                               const ::com::sun::star::geometry::IntegerSize2D&  size );

        /** Range checker, which throws ::com::sun::star::lang::IndexOutOfBounds exception, when
            the size is negative or null

            @param size
            Size to verify
         */
        CANVASTOOLS_DLLPUBLIC void verifyBitmapSize( const ::com::sun::star::geometry::IntegerSize2D& size,
                               const char*                                      pStr,
                               const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::uno::XInterface >&     xIf );

        /** Range checker, which throws ::com::sun::star::lang::IndexOutOfBounds exception, when
            the size is negative or null

            @param size
            Size to verify
         */
        CANVASTOOLS_DLLPUBLIC void verifySpriteSize( const ::com::sun::star::geometry::RealSize2D& size,
                               const char*                                   pStr,
                               const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::uno::XInterface >&  xIf );
    }
}

#endif /* INCLUDED_CANVAS_VERIFYINPUT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
