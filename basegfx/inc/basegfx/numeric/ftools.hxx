/*************************************************************************
 *
 *  $RCSfile: ftools.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 18:32:42 $
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

#ifndef _BGFX_NUMERIC_FTOOLS_HXX
#define _BGFX_NUMERIC_FTOOLS_HXX

#ifndef  _USE_MATH_DEFINES
#define  _USE_MATH_DEFINES  // needed by Visual C++ for math constants
#endif
#include <math.h>           // M_PI definition

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#include <algorithm> // for min/max


//////////////////////////////////////////////////////////////////////////////
// standard PI defines from solar.h, but we do not want to link against tools

#ifndef F_PI
#define F_PI        M_PI
#endif
#ifndef F_PI2
#define F_PI2       (M_PI/2.0)
#endif
#ifndef F_PI4
#define F_PI4       (M_PI/4.0)
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

//////////////////////////////////////////////////////////////////////////////
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
        return fVal < 0.0 ?
            (::std::min(fVal,-0.00001)) :
            (::std::max(fVal,0.00001));
    }

    class fTools
    {
        /// Threshold value for equalZero()
        static double                                   mfSmallValue;

    public:
        /// Get threshold value for equalZero and friends
        static double getSmallValue() { return mfSmallValue; }
        /// Set threshold value for equalZero and friends
        static void setSmallValue(const double& rfNew) { mfSmallValue = rfNew; }

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
            return (fabs(rfValB - rfValA) <= getSmallValue());
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


        static bool equal(const double& rfValA, const double& rfValB, const double& rfSmallValue)
        {
            return (fabs(rfValB - rfValA) <= rfSmallValue);
        }

        static bool less(const double& rfValA, const double& rfValB, const double& rfSmallValue)
        {
            return (rfValA < rfValB && !equal(rfValA, rfValB, rfSmallValue));
        }

        static bool lessOrEqual(const double& rfValA, const double& rfValB, const double& rfSmallValue)
        {
            return (rfValA < rfValB || equal(rfValA, rfValB, rfSmallValue));
        }

        static bool more(const double& rfValA, const double& rfValB, const double& rfSmallValue)
        {
            return (rfValA > rfValB && !equal(rfValA, rfValB, rfSmallValue));
        }

        static bool moreOrEqual(const double& rfValA, const double& rfValB, const double& rfSmallValue)
        {
            return (rfValA > rfValB || equal(rfValA, rfValB, rfSmallValue));
        }
    };
} // end of namespace basegfx

#endif /* _BGFX_NUMERIC_FTOOLS_HXX */
