/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bessel.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:21:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SCA_BESSEL_HXX
#define SCA_BESSEL_HXX

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

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

