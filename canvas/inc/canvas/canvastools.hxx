/*************************************************************************
 *
 *  $RCSfile: canvastools.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2005-06-15 10:25:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CANVAS_CANVASTOOLS_HXX
#define _CANVAS_CANVASTOOLS_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif

#include <limits>
#include <algorithm>


namespace basegfx
{
    class B2DHomMatrix;
    class B2DRange;
}

namespace com { namespace sun { namespace star { namespace geometry
{
    struct RealSize2D;
    struct AffineMatrix2D;
    struct Matrix2D;
} } } }

namespace com { namespace sun { namespace star { namespace rendering
{
    struct RenderState;
    struct ViewState;
    class  XCanvas;
} } } }


namespace canvas
{
    namespace tools
    {
        /** Compute the next highest power of 2 of a 32-bit value

            Code devised by Sean Anderson, in good ole HAKMEM
            tradition.

            @return 1 << (lg(x - 1) + 1)
        */
        inline sal_uInt32 nextPow2( sal_uInt32 x )
        {
            --x;
            x |= x >> 1;
            x |= x >> 2;
            x |= x >> 4;
            x |= x >> 8;
            x |= x >> 16;

            return ++x;
        }

        /**
         *
         * Count the number of 1-bits of an n-bit value
         *
         */

        // mickey's math tricks...
        inline unsigned int pow2( unsigned int c ) { return 0x1 << c; }
        inline unsigned int mask( unsigned int c ) { return ((unsigned int)(-1)) / (pow2(pow2(c)) + 1); }
        inline unsigned int count( unsigned int x, unsigned int c ) { return ((x) & mask(c)) + (((x) >> (pow2(c))) & mask(c)); }
        template<typename T>
        inline unsigned int bitcount( T c ) {
            unsigned int nByteIndex = 0;
            unsigned int nNumBytes = sizeof(T)<<2;
            do {
                c=count(c,nByteIndex++);
                nNumBytes >>= 1;
            } while(nNumBytes);
            return c;
        }
        inline unsigned int bitcount32( unsigned int c ) {
            c=count(c,0);
            c=count(c,1);
            c=count(c,2);
            c=count(c,3);
            c=count(c,4);
            return c;
        }

        /** Create a RealSize2D with both coordinate values set to +infinity
         */
        ::com::sun::star::geometry::RealSize2D createInfiniteSize2D();


        // View- and RenderState utilities
        // ===================================================================

        ::com::sun::star::rendering::RenderState&
            initRenderState( ::com::sun::star::rendering::RenderState&                      renderState );

        ::com::sun::star::rendering::ViewState&
            initViewState( ::com::sun::star::rendering::ViewState&                          viewState );

        ::basegfx::B2DHomMatrix&
            getViewStateTransform( ::basegfx::B2DHomMatrix&                                 transform,
                                   const ::com::sun::star::rendering::ViewState&            viewState );

        ::com::sun::star::rendering::ViewState&
            setViewStateTransform( ::com::sun::star::rendering::ViewState&                  viewState,
                                   const ::basegfx::B2DHomMatrix&                           transform );

        ::basegfx::B2DHomMatrix&
            getRenderStateTransform( ::basegfx::B2DHomMatrix&                               transform,
                                     const ::com::sun::star::rendering::RenderState&        renderState );

        ::com::sun::star::rendering::RenderState&
            setRenderStateTransform( ::com::sun::star::rendering::RenderState&              renderState,
                                     const ::basegfx::B2DHomMatrix&                         transform );

        ::com::sun::star::rendering::ViewState&
            appendToViewState( ::com::sun::star::rendering::ViewState&                      viewState,
                               const ::basegfx::B2DHomMatrix&                               transform );

        ::com::sun::star::rendering::RenderState&
            appendToRenderState( ::com::sun::star::rendering::RenderState&                  renderState,
                                 const ::basegfx::B2DHomMatrix&                             transform );

        ::com::sun::star::rendering::ViewState&
            prependToViewState( ::com::sun::star::rendering::ViewState&                     viewState,
                                const ::basegfx::B2DHomMatrix&                              transform );

        ::com::sun::star::rendering::RenderState&
            prependToRenderState( ::com::sun::star::rendering::RenderState&                 renderState,
                                  const ::basegfx::B2DHomMatrix&                            transform );

        ::basegfx::B2DHomMatrix&
            mergeViewAndRenderTransform( ::basegfx::B2DHomMatrix&                           transform,
                                         const ::com::sun::star::rendering::ViewState&      viewState,
                                         const ::com::sun::star::rendering::RenderState&    renderState );

        ::com::sun::star::rendering::ViewState&
            mergeViewAndRenderState( ::com::sun::star::rendering::ViewState&                resultViewState,
                                     const ::com::sun::star::rendering::ViewState&          viewState,
                                     const ::com::sun::star::rendering::RenderState&        renderState,
                                     const ::com::sun::star::uno::Reference<
                                         ::com::sun::star::rendering::XCanvas >                 xCanvas );

        void setDeviceColor( ::com::sun::star::rendering::RenderState&  o_renderState,
                             const double&                              rColor0,
                             const double&                              rColor1,
                             const double&                              rColor2,
                             const double&                              rColor3 );

        void getDeviceColor( double&                                            o_rColor0,
                             double&                                            o_rColor1,
                             double&                                            o_rColor2,
                             double&                                            o_rColor3,
                             const ::com::sun::star::rendering::RenderState&    rRenderState );

        bool operator==( const ::com::sun::star::rendering::RenderState&    rLHS,
                         const ::com::sun::star::rendering::RenderState&    rRHS );

        bool operator==( const ::com::sun::star::rendering::ViewState&      rLHS,
                         const ::com::sun::star::rendering::ViewState&      rRHS );


        // Matrix utilities
        // ===================================================================

        ::com::sun::star::geometry::AffineMatrix2D&
            setIdentityAffineMatrix2D( ::com::sun::star::geometry::AffineMatrix2D&  matrix );

        ::com::sun::star::geometry::Matrix2D&
            setIdentityMatrix2D( ::com::sun::star::geometry::Matrix2D&              matrix );


        // Special utilities
        // ===================================================================

        /** Calc the bounding rectangle of a transformed rectangle.

            The method applies the given transformation to the
            specified input rectangle, and returns the bounding box of
            the resulting output area.

            @param o_Rect
            Output rectangle

            @param i_Rect
            Input rectangle

            @param i_Transformation
            Transformation to apply to the input rectangle

            @see calcRectToRectTransform()

            @return a reference to the resulting rectangle
         */
        ::basegfx::B2DRange& calcTransformedRectBounds( ::basegfx::B2DRange&            o_Rect,
                                                        const ::basegfx::B2DRange&      i_Rect,
                                                        const ::basegfx::B2DHomMatrix&  i_Transformation );

        /** Calc a transform that maps one rectangle on top of
            another.

            The method is a kissing cousin to
            calcTransformedRectBounds(). It can be used to modify the
            given transformation matrix, such that it transforms the
            given input rectangle to the given output rectangle,
            changing only translation and scale (if necessary). Thus,
            if you've calculated an output rectangle via
            calcTransformedRectBounds(), you can move and scale that
            rectangle as you like, and have this method calculate the
            required total transformation for it.

            @param o_transform
            Output parameter, to receive the resulting transformation
            matrix.

            @param i_destRect
            Input parameter, specifies the requested destination
            rectangle. The resulting transformation will exactly map
            the source rectangle to the destination rectangle.

            @param i_srcRect
            Input parameter, specifies the original source
            rectangle. The resulting transformation will exactly map
            the source rectangle to the destination rectangle.

            @param i_transformation
            The original transformation matrix. This is changed with
            translations and scalings (if necessary), to exactly map
            the source rectangle to the destination rectangle.

            @return a reference to the resulting transformation matrix

            @see calcTransformedRectBounds()
        */
        ::basegfx::B2DHomMatrix& calcRectToRectTransform( ::basegfx::B2DHomMatrix&          o_transform,
                                                          const ::basegfx::B2DRange&        i_destRect,
                                                          const ::basegfx::B2DRange&        i_srcRect,
                                                          const ::basegfx::B2DHomMatrix&    i_transformation );

        /** Calc a transform that maps the upper, left corner of a
             rectangle to the origin.

            The method is a specialized version of
            calcRectToRectTransform(), mapping the input rectangle's
            the upper, left corner to the origin, and leaving the size
            untouched.

            @param o_transform
            Output parameter, to receive the resulting transformation
            matrix.

            @param i_srcRect
            Input parameter, specifies the original source
            rectangle. The resulting transformation will exactly map
            the source rectangle's upper, left corner to the origin.

            @param i_transformation
            The original transformation matrix. This is changed with
            translations (if necessary), to exactly map the source
            rectangle to the origin.

            @return a reference to the resulting transformation matrix

            @see calcRectToRectTransform()
            @see calcTransformedRectBounds()
        */
        ::basegfx::B2DHomMatrix& calcRectToOriginTransform( ::basegfx::B2DHomMatrix&        o_transform,
                                                            const ::basegfx::B2DRange&      i_srcRect,
                                                            const ::basegfx::B2DHomMatrix&  i_transformation );

        /** Range checker, which throws ::com::sun::star::lang::IllegalArgument exception, when
            range is violated
        */
        template< typename NumType > inline void checkRange( NumType arg, NumType lowerBound, NumType upperBound )
        {
            if( arg < lowerBound ||
                arg > upperBound )
            {
                throw ::com::sun::star::lang::IllegalArgumentException();
            }
        }

        /** Range checker, which throws ::com::sun::star::lang::IndexOutOfBounds exception, when
            index range is violated
        */
        template< typename NumType > inline void checkIndexRange( NumType arg, NumType lowerBound, NumType upperBound )
        {
            if( arg < lowerBound ||
                arg > upperBound )
            {
                throw ::com::sun::star::lang::IndexOutOfBoundsException();
            }
        }

        // BEWARE(E2): don't currently use with float or double, Solaris
        // STLport's numeric_limits bark on that (unresolved
        // externals)

        // Modeled closely after boost::numeric_cast, only that we
        // issue some trace output here and throw a RuntimeException
        template< typename Target, typename Source > inline Target numeric_cast( Source arg )
        {
            // typedefs abbreviating respective trait classes
            typedef ::std::numeric_limits< Source > SourceLimits;
            typedef ::std::numeric_limits< Target > TargetLimits;

            if( ( arg<0 && !TargetLimits::is_signed) ||                     // loosing the sign here
                ( SourceLimits::is_signed && arg<TargetLimits::min()) ||    // underflow will happen
                ( arg>TargetLimits::max() ) )                               // overflow will happen
            {
#if defined(VERBOSE) && defined(DBG_UTIL)
                OSL_TRACE("numeric_cast detected data loss");
#endif
                throw ::com::sun::star::uno::RuntimeException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "numeric_cast detected data loss" )),
                    NULL );
            }

            return static_cast<Target>(arg);
        }

        /** Retrieve various internal properties of the actual canvas implementation.

            This method retrieves a bunch of internal, implementation-
            and platform-dependent values from the canvas
            implementation. Among them are for example operating
            system window handles. The actual layout and content of
            the returned sequence is dependent on the component
            implementation, undocumented and subject to change.

            @param i_rxCanvas
            Input parameter, the canvas representation for which the device information
            is to be retrieveds

            @param o_rxParams
            Output parameter, the sequence of Anys that hold the device parameters. Layout is as described above

            @return A reference to the resulting sequence of parameters
        */
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& getDeviceInfo(
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >& i_rxCanvas,
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& o_rxParams );
    }
}

#endif /* _CANVAS_CANVASTOOLS_HXX */
// eof
