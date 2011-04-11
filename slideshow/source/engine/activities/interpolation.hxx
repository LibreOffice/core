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

#ifndef INCLUDED_SLIDESHOW_INTERPOLATION_HXX
#define INCLUDED_SLIDESHOW_INTERPOLATION_HXX

#include <basegfx/tools/lerp.hxx>

namespace basegfx
{
    namespace tools
    {
        // Interpolator specializations
        // ============================

        // NOTE: generic lerp is included from lerp.hxx. Following
        // are some specializations for various
        // not-straight-forward-interpolatable types

        /// Specialization for RGBColor, to employ color-specific interpolator
        template<> ::slideshow::internal::RGBColor lerp< ::slideshow::internal::RGBColor >(
            const ::slideshow::internal::RGBColor& rFrom,
            const ::slideshow::internal::RGBColor& rTo,
            double                                 t     )
        {
            return interpolate( rFrom, rTo, t );
        }

        /// Specialization also for sal_Int16, although this code should not be called
        template<> sal_Int16 lerp< sal_Int16 >( const sal_Int16&,
                                                const sal_Int16&    rTo,
                                                double                     )
        {
            OSL_FAIL( "lerp<sal_Int16> called" );
            return rTo;
        }

        /// Specialization also for string, although this code should not be called
        template<> ::rtl::OUString lerp< ::rtl::OUString >( const ::rtl::OUString&,
                                                            const ::rtl::OUString&  rTo,
                                                            double                       )
        {
            OSL_FAIL( "lerp<::rtl::OUString> called" );
            return rTo;
        }

        /// Specialization also for bool, although this code should not be called
        template<> bool lerp< bool >( const bool&,
                                      const bool&   rTo,
                                      double             )
        {
            OSL_FAIL( "lerp<bool> called" );
            return rTo;
        }
    }
}

namespace slideshow
{
    namespace internal
    {
        template< typename ValueType > struct Interpolator
        {
            ValueType operator()( const ValueType&  rFrom,
                                  const ValueType&  rTo,
                                  double            t ) const
            {
                return basegfx::tools::lerp( rFrom, rTo, t );
            }
        };

        /// Specialization for HSLColor, to employ color-specific interpolator
        template<> struct Interpolator< HSLColor >
        {
            Interpolator( bool bCCW ) :
                mbCCW( bCCW )
            {
            }

            HSLColor operator()( const HSLColor&    rFrom,
                                 const HSLColor&    rTo,
                                 double             t ) const
            {
                return interpolate( rFrom, rTo, t, mbCCW );
            }

        private:
            /// When true: interpolate counter-clockwise
            const bool mbCCW;
        };


        /** Generic linear interpolator

            @tpl ValueType
            Must have operator+ and operator* defined, and should
            have value semantics.

            @param rInterpolator
            Interpolator to use for lerp

            @param nFrame
            Must be in the [0,nTotalFrames) range

            @param nTotalFrames
            Total number of frames. Should be greater than zero.
        */
        template< typename ValueType > ValueType lerp( const Interpolator< ValueType >& rInterpolator,
                                                       const ValueType&                 rFrom,
                                                       const ValueType&                 rTo,
                                                       sal_uInt32                       nFrame,
                                                       ::std::size_t                    nTotalFrames )
        {
            // TODO(P1): There's a nice HAKMEM trick for that
            // nTotalFrames > 1 condition below

            // for 1 and 0 frame animations, always take end value
            const double nFraction( nTotalFrames > 1 ? double(nFrame)/(nTotalFrames-1) : 1.0 );

            return rInterpolator( rFrom, rTo, nFraction );
        }

        /// Specialization for non-interpolatable constants/enums
        template<> sal_Int16 lerp< sal_Int16 >( const Interpolator< sal_Int16 >&    /*rInterpolator*/,
                                                const sal_Int16&                    rFrom,
                                                const sal_Int16&                    rTo,
                                                sal_uInt32                          nFrame,
                                                ::std::size_t                       nTotalFrames )
        {
            // until one half of the total frames are over, take from value.
            // after that, take to value.
            // For nFrames not divisable by 2, we prefer to over from, which
            // also neatly yields to for 1 frame activities
            return nFrame < nTotalFrames/2 ? rFrom : rTo;
        }

        /// Specialization for non-interpolatable strings
        template<> ::rtl::OUString lerp< ::rtl::OUString >( const Interpolator< ::rtl::OUString >&  /*rInterpolator*/,
                                                            const ::rtl::OUString&                  rFrom,
                                                            const ::rtl::OUString&                  rTo,
                                                            sal_uInt32                              nFrame,
                                                            ::std::size_t                           nTotalFrames )
        {
            // until one half of the total frames are over, take from value.
            // after that, take to value.
            // For nFrames not divisable by 2, we prefer to over from, which
            // also neatly yields to for 1 frame activities
            return nFrame < nTotalFrames/2 ? rFrom : rTo;
        }

        /// Specialization for non-interpolatable bools
        template<> bool lerp< bool >( const Interpolator< bool >&   /*rInterpolator*/,
                                      const bool&                   bFrom,
                                      const bool&                   bTo,
                                      sal_uInt32                    nFrame,
                                      ::std::size_t                 nTotalFrames )
        {
            // until one half of the total frames are over, take from value.
            // after that, take to value.
            // For nFrames not divisable by 2, we prefer to over from, which
            // also neatly yields to for 1 frame activities
            return nFrame < nTotalFrames/2 ? bFrom : bTo;
        }
    }
}

#endif /* INCLUDED_SLIDESHOW_INTERPOLATION_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
