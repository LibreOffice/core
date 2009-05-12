/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bessel.hxx,v $
 * $Revision: 1.4 $
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

#ifndef SCA_BESSEL_HXX
#define SCA_BESSEL_HXX

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <sal/types.h>

namespace sca {
namespace analysis {

// ============================================================================
//     BESSEL functions
// ============================================================================

/** Returns the result for the unmodified BESSEL function of first kind (J), n-th order, at point x. */
double BesselJ( double x, sal_Int32 n ) throw( ::com::sun::star::lang::IllegalArgumentException );

/** Returns the result for the modified BESSEL function of first kind (I), n-th order, at point x. */
double BesselI( double x, sal_Int32 n ) throw( ::com::sun::star::lang::IllegalArgumentException );

/** Returns the result for the unmodified BESSEL function of second kind (Y), n-th order, at point x. */
double BesselY( double x, sal_Int32 n ) throw( ::com::sun::star::lang::IllegalArgumentException );

/** Returns the result for the modified BESSEL function of second kind (K), n-th order, at point x. */
double BesselK( double x, sal_Int32 n ) throw( ::com::sun::star::lang::IllegalArgumentException );

// ============================================================================

} // namespace analysis
} // namespace sca

#endif

