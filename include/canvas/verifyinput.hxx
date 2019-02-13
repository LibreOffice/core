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

#ifndef INCLUDED_CANVAS_VERIFYINPUT_HXX
#define INCLUDED_CANVAS_VERIFYINPUT_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

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
} } } }

namespace com::sun::star::uno { template <class E> class Sequence; }

namespace canvas
{
    namespace tools
    {

        // Input checking facilities


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
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::geometry::AffineMatrix2D&   rMatrix,
                          const char*                                       pStr,
                          const css::uno::Reference< css::uno::XInterface >&  xIf,
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
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::geometry::Matrix2D& rMatrix,
                          const char*                                   pStr,
                          const css::uno::Reference< css::uno::XInterface >&  xIf,
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
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::geometry::RealPoint2D&  rPoint,
                          const char*                                       pStr,
                          const css::uno::Reference< css::uno::XInterface >&   xIf,
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
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::geometry::RealBezierSegment2D&  rSegment,
                          const char*                                               pStr,
                          const css::uno::Reference< css::uno::XInterface >&        xIf,
                          ::sal_Int16                                               nArgPos );

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
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::geometry::RealRectangle2D&  rRect,
                          const char*                                           pStr,
                          const css::uno::Reference< css::uno::XInterface >&    xIf,
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
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::rendering::ViewState&   viewState,
                          const char*                                   pStr,
                          const css::uno::Reference< css::uno::XInterface >& xIf,
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
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::rendering::RenderState& renderState,
                          const char*                                       pStr,
                          const css::uno::Reference< css::uno::XInterface >&  xIf,
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
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::rendering::StrokeAttributes&    strokeAttributes,
                          const char*                                           pStr,
                          const css::uno::Reference< css::uno::XInterface >&    xIf,
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
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::rendering::Texture&     texture,
                          const char*                                   pStr,
                          const css::uno::Reference< css::uno::XInterface >& xIf,
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
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::rendering::IntegerBitmapLayout&     bitmapLayout,
                          const char*                                               pStr,
                          const css::uno::Reference< css::uno::XInterface >&        xIf,
                          ::sal_Int16                                               nArgPos );

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
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::rendering::FontRequest& fontRequest,
                          const char*                                       pStr,
                          const css::uno::Reference< css::uno::XInterface >&  xIf,
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
            const css::uno::Reference< Interface >&                 rRef,
            const char*                                             pStr,
            const css::uno::Reference< css::uno::XInterface >&      xIf,
            ::sal_Int16                                             nArgPos )
        {
            if( !rRef.is() )
            {
#if OSL_DEBUG_LEVEL > 0
                throw css::lang::IllegalArgumentException(
                    OUString::createFromAscii(pStr) + ": reference is NULL",
                    xIf,
                    nArgPos );
#else
                (void)pStr; (void)xIf; (void)nArgPos;
                throw css::lang::IllegalArgumentException();
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
            const css::uno::Sequence< SequenceContent >&                rSequence,
            const char*                                                 pStr,
            const css::uno::Reference< css::uno::XInterface >&          xIf,
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
                                                 const css::uno::Reference< css::uno::XInterface >& /*xIf*/,
                                                 ::sal_Int16                                /*nArgPos*/ )
        {
        }

        // TODO(Q2): Employ some template arglist magic here, to avoid
        // this duplication of code...

        template< typename Arg0 > void verifyArgs( const Arg0&                                  rArg0,
                                                   const char*                                  pStr,
                                                   const css::uno::Reference< css::uno::XInterface >& xIf )
        {
            verifyInput( rArg0, pStr, xIf, 0 );
        }

        template< typename Arg0,
                  typename Arg1 > void verifyArgs( const Arg0&                                  rArg0,
                                                   const Arg1&                                  rArg1,
                                                   const char*                                  pStr,
                                                   const css::uno::Reference< css::uno::XInterface >& xIf )
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
                                                   const css::uno::Reference< css::uno::XInterface >& xIf )
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
                                                   const css::uno::Reference< css::uno::XInterface >& xIf )
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
                                                   const css::uno::Reference< css::uno::XInterface >& xIf )
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
                                                   const css::uno::Reference< css::uno::XInterface >& xIf )
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
                                                   const css::uno::Reference< css::uno::XInterface >& xIf )
        {
            verifyInput( rArg0, pStr, xIf, 0 );
            verifyInput( rArg1, pStr, xIf, 1 );
            verifyInput( rArg2, pStr, xIf, 2 );
            verifyInput( rArg3, pStr, xIf, 3 );
            verifyInput( rArg4, pStr, xIf, 4 );
            verifyInput( rArg5, pStr, xIf, 5 );
            verifyInput( rArg6, pStr, xIf, 6 );
        }


        /** Range checker, which throws css::lang::IllegalArgument exception, when
            range is violated
        */
        template< typename NumType > inline void verifyRange( NumType arg, NumType lowerBound, NumType upperBound )
        {
            if( arg < lowerBound ||
                arg > upperBound )
            {
                throw css::lang::IllegalArgumentException();
            }
        }

        /** Range checker, which throws css::lang::IllegalArgument exception, when
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
                throw css::lang::IllegalArgumentException();
            }
        }

        /** Range checker, which throws css::lang::IndexOutOfBounds exception, when
            index range is violated

            @param rect
            Rect to verify

            @param size
            Given rectangle must be within ((0,0), (size.Width, size.Height))
         */
        CANVASTOOLS_DLLPUBLIC void verifyIndexRange( const css::geometry::IntegerRectangle2D& rect,
                               const css::geometry::IntegerSize2D&      size );

        /** Range checker, which throws css::lang::IndexOutOfBounds exception, when
            index range is violated

            @param pos
            Position to verify

            @param size
            Given position must be within ((0,0), (size.Width, size.Height))
         */
        CANVASTOOLS_DLLPUBLIC void verifyIndexRange( const css::geometry::IntegerPoint2D& pos,
                               const css::geometry::IntegerSize2D&  size );

        /** Range checker, which throws css::lang::IndexOutOfBounds exception, when
            the size is negative or null

            @param size
            Size to verify
         */
        CANVASTOOLS_DLLPUBLIC void verifyBitmapSize( const css::geometry::IntegerSize2D& size,
                               const char*                                      pStr,
                               const css::uno::Reference< css::uno::XInterface >&     xIf );

        /** Range checker, which throws css::lang::IndexOutOfBounds exception, when
            the size is negative or null

            @param size
            Size to verify
         */
        CANVASTOOLS_DLLPUBLIC void verifySpriteSize( const css::geometry::RealSize2D& size,
                               const char*                                   pStr,
                               const css::uno::Reference< css::uno::XInterface >&  xIf );
    }
}

#endif /* INCLUDED_CANVAS_VERIFYINPUT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
