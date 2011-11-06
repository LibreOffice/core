/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SCA_BESSEL_HXX
#define SCA_BESSEL_HXX

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/sheet/NoConvergenceException.hpp>
#include <sal/types.h>

namespace sca {
namespace analysis {

// ============================================================================
//     BESSEL functions
// ============================================================================

/** Returns the result for the unmodified BESSEL function of first kind (J), n-th order, at point x. */
double BesselJ( double x, sal_Int32 n ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::sheet::NoConvergenceException );

/** Returns the result for the modified BESSEL function of first kind (I), n-th order, at point x. */
double BesselI( double x, sal_Int32 n ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::sheet::NoConvergenceException );

/** Returns the result for the unmodified BESSEL function of second kind (Y), n-th order, at point x. */
double BesselY( double x, sal_Int32 n ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::sheet::NoConvergenceException );

/** Returns the result for the modified BESSEL function of second kind (K), n-th order, at point x. */
double BesselK( double x, sal_Int32 n ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::sheet::NoConvergenceException );

// ============================================================================

} // namespace analysis
} // namespace sca

#endif

