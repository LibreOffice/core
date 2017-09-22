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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_INTERPOLATION_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_INTERPOLATION_HXX

#include <basegfx/utils/lerp.hxx>

namespace basegfx
{
    namespace utils
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
        template<> OUString lerp< OUString >( const OUString&,
                                                            const OUString&  rTo,
                                                            double                       )
        {
            OSL_FAIL( "lerp<OUString> called" );
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
                return basegfx::utils::lerp( rFrom, rTo, t );
            }
        };

        /// Specialization for HSLColor, to employ color-specific interpolator
        template<> struct Interpolator< HSLColor >
        {
            explicit Interpolator( bool bCCW ) :
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
        template<> OUString lerp< OUString >( const Interpolator< OUString >&  /*rInterpolator*/,
                                                            const OUString&                  rFrom,
                                                            const OUString&                  rTo,
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

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_INTERPOLATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
