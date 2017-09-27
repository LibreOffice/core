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

#ifndef INCLUDED_BASEGFX_NUMERIC_FTOOLS_HXX
#define INCLUDED_BASEGFX_NUMERIC_FTOOLS_HXX

#include <rtl/math.hxx>
#include <basegfx/basegfxdllapi.h>


// standard PI defines from solar.h, but we do not want to link against tools

#ifndef F_PI
#define F_PI        M_PI
#endif
#ifndef F_PI2
#define F_PI2       M_PI_2
#endif
#ifndef F_PI4
#define F_PI4       M_PI_4
#endif
#ifndef F_PI180
#define F_PI180     (M_PI/180.0)
#endif
#ifndef F_PI1800
#define F_PI1800    (M_PI/1800.0)
#endif
#ifndef F_PI18000
#define F_PI18000   (M_PI/18000.0)
#endif
#ifndef F_2PI
#define F_2PI       (2.0*M_PI)
#endif


// fTools defines

namespace basegfx
{
    /** Round double to nearest integer

        @return the nearest integer
    */
    inline sal_Int32 fround( double fVal )
    {
        return fVal > 0.0 ? static_cast<sal_Int32>( fVal + .5 ) : -static_cast<sal_Int32>( -fVal + .5 );
    }

    /** Round double to nearest integer

        @return the nearest 64 bit integer
    */
    inline sal_Int64 fround64( double fVal )
    {
        return fVal > 0.0 ? static_cast<sal_Int64>( fVal + .5 ) : -static_cast<sal_Int64>( -fVal + .5 );
    }

    /** Prune a small epsilon range around zero.

        Use this method e.g. for calculating scale values. There, it
        is usually advisable not to set a scaling to 0.0, because that
        yields singular transformation matrices.

        @param fVal
        An arbitrary, but finite and valid number

        @return either fVal, or a small value slightly above (when
        fVal>0) or below (when fVal<0) zero.
     */
    inline double pruneScaleValue( double fVal )
    {
        // old version used ::std::min/max, but this collides if min is defined as preprocessor
        // macro which is the case e.g with windows.h headers. The simplest way to avoid this is to
        // just use the full comparison. I keep the original here, maybe there will be a better
        // solution some day.

        //return fVal < 0.0 ?
        //    (::std::min(fVal,-0.00001)) :
        //    (::std::max(fVal,0.00001));

        if(fVal < 0.0)
            return (fVal < -0.00001 ? fVal : -0.00001);
        else
            return (fVal > 0.00001 ? fVal : 0.00001);
    }

    /** clamp given value against given minimum and maximum values
    */
    template <class T> inline const T& clamp(const T& value, const T& minimum, const T& maximum)
    {
        if(value < minimum)
        {
            return minimum;
        }
        else if(value > maximum)
        {
            return maximum;
        }
        else
        {
            return value;
        }
    }

    /** Convert value from degrees to radians
     */
    inline double deg2rad( double v )
    {
        // divide first, to get exact values for v being a multiple of
        // 90 degrees
        return v / 90.0 * M_PI_2;
    }

    /** Convert value radians to degrees
     */
    inline double rad2deg( double v )
    {
        // divide first, to get exact values for v being a multiple of
        // pi/2
        return v / M_PI_2 * 90.0;
    }

    /** Snap v to nearest multiple of fStep, from negative and
        positive side.

        Examples:

        snapToNearestMultiple(-0.1, 0.5) = 0.0
        snapToNearestMultiple(0.1, 0.5) = 0.0
        snapToNearestMultiple(0.25, 0.5) = 0.0
        snapToNearestMultiple(0.26, 0.5) = 0.5
     */
    BASEGFX_DLLPUBLIC double snapToNearestMultiple(double v, const double fStep);

    class fTools
    {
    public:
        /// Get threshold value for equalZero and friends
        static double getSmallValue() { return 0.000000001f; }

        /// Compare against small value
        static bool equalZero(const double& rfVal)
        {
            return (fabs(rfVal) <= getSmallValue());
        }

        /// Compare against given small value
        static bool equalZero(const double& rfVal, const double& rfSmallValue)
        {
            return (fabs(rfVal) <= rfSmallValue);
        }

        static bool equal(const double& rfValA, const double& rfValB)
        {
            // changed to approxEqual usage for better numerical correctness
            return rtl::math::approxEqual(rfValA, rfValB);
        }

        static bool equal(const double& rfValA, const double& rfValB, const double& rfSmallValue)
        {
            return (fabs(rfValA - rfValB) <= rfSmallValue);
        }

        static bool less(const double& rfValA, const double& rfValB)
        {
            return (rfValA < rfValB && !equal(rfValA, rfValB));
        }

        static bool lessOrEqual(const double& rfValA, const double& rfValB)
        {
            return (rfValA < rfValB || equal(rfValA, rfValB));
        }

        static bool more(const double& rfValA, const double& rfValB)
        {
            return (rfValA > rfValB && !equal(rfValA, rfValB));
        }

        static bool moreOrEqual(const double& rfValA, const double& rfValB)
        {
            return (rfValA > rfValB || equal(rfValA, rfValB));
        }

        static bool betweenOrEqualEither(const double& rfValA, const double& rfValB, const double& rfValC)
        {
            return (rfValA > rfValB && rfValA < rfValC) || equal(rfValA, rfValB) || equal(rfValA, rfValC);
        }

    };
} // end of namespace basegfx

#endif // INCLUDED_BASEGFX_NUMERIC_FTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
