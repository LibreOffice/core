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

#pragma once

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <canvas/canvastoolsdllapi.h>

namespace com::sun::star::geometry
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
}

namespace com::sun::star::rendering
{
    struct StrokeAttributes;
    struct FontRequest;
}

namespace cpo::uno { template <class E> class Sequence; }

namespace vclcanvas
{
    class Canvas;
    struct RenderState;
    struct Texture;
    struct ViewState;
}

namespace canvastools
{

        // Input checking facilities


        // This header provides methods to check all common
        // css::rendering::* method input parameters against
        // compliance to the API specification.

        /** Verify that the given transformation contains valid floating point
            values.

            @param rMatrix
            Matrix to check

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws a lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::geometry::AffineMatrix2D&   rMatrix,
                          const char*                                       pStr,
                          ::sal_Int16                                       nArgPos );

        /** Verify that the given transformation contains valid floating point
            values.

            @param rMatrix
            Matrix to check

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws a lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::geometry::Matrix2D& rMatrix,
                          const char*                                   pStr,
                          ::sal_Int16                                   nArgPos );

        /** Verify that the given point contains valid floating point
            values.

            @param rPoint
            Point to check

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws a lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::geometry::RealPoint2D&  rPoint,
                          const char*                                       pStr,
                          ::sal_Int16                                       nArgPos );

        /** Basic check for view state validity.

            @param viewState
            Viewstate to check

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws a lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::vclcanvas::ViewState&   viewState,
                          const char*                                   pStr,
                          ::sal_Int16                                   nArgPos );

        /** Basic check for render state validity.

            @param renderState
            Renderstate to check

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @param nMinColorComponents
            Minimal number of color components available in
            RenderState::DeviceColor

            @throws a lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const ::vclcanvas::RenderState& renderState,
                          const char*                                       pStr,
                          ::sal_Int16                                       nArgPos,
                          sal_Int32                                         nMinColorComponents=0 );

        /** Basic check for stroke attributes validity.

            @param strokeAttributes
            Attributes to check

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws a lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::rendering::StrokeAttributes&    strokeAttributes,
                          const char*                                           pStr,
                          ::sal_Int16                                           nArgPos );

        /** Basic check for texture validity.

            @param texture
            Texture to check

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws a lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const vclcanvas::Texture&     texture,
                          const char*                                   pStr,
                          ::sal_Int16                                   nArgPos );

        /** Basic check for font request validity.

            @param fontRequest
            Font request to check

            @param nArgPos
            Argument position on the call site (i.e. the position of
            the argument, checked here, on the UNO interface
            method. Counting starts at 0).

            @throws a lang::IllegalArgumentException, if anything is wrong
         */
        CANVASTOOLS_DLLPUBLIC void verifyInput( const css::rendering::FontRequest& fontRequest,
                          const char*                                       pStr,
                          ::sal_Int16                                       nArgPos );

        template< typename SequenceContent > void verifyInput(
            const std::vector< SequenceContent >&                rSequence,
            const char*                                                 pStr,
            ::sal_Int16                                                 nArgPos )
        {
            for (auto& element : rSequence)
                verifyInput(element, pStr, nArgPos);
        }

        /// Catch-all, to handle cases that DON'T need input checking (i.e. the Integer geometry ones)
        template< typename T > void verifyInput( const T&                                   /*rDummy*/,
                                                 const char*                                /*pStr*/,
                                                 ::sal_Int16                                /*nArgPos*/ )
        {
        }

        // TODO(Q2): Employ some template arglist magic here, to avoid
        // this duplication of code...

        template< typename Arg0,
                  typename Arg1,
                  typename Arg2 > void verifyArgs( const Arg0&                                  rArg0,
                                                   const Arg1&                                  rArg1,
                                                   const Arg2&                                  rArg2,
                                                   const char*                                  pStr )
        {
            verifyInput( rArg0, pStr, 0 );
            verifyInput( rArg1, pStr, 1 );
            verifyInput( rArg2, pStr, 2 );
        }

        template< typename Arg0,
                  typename Arg1,
                  typename Arg2,
                  typename Arg3 > void verifyArgs( const Arg0&                                  rArg0,
                                                   const Arg1&                                  rArg1,
                                                   const Arg2&                                  rArg2,
                                                   const Arg3&                                  rArg3,
                                                   const char*                                  pStr )
        {
            verifyInput( rArg0, pStr, 0 );
            verifyInput( rArg1, pStr, 1 );
            verifyInput( rArg2, pStr, 2 );
            verifyInput( rArg3, pStr, 3 );
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
