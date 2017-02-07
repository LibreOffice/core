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



#if !defined INCLUDED_SAL_MATHCONF_H
#define INCLUDED_SAL_MATHCONF_H

#include "osl/endian.h"

#include <float.h>
#include <math.h>

#if defined SOLARIS
#include <ieeefp.h>
#endif /* SOLARIS */

#if defined __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Generally, the C standard guarantees that at program startup, "trapping or
   stopping (if supported) is disabled on all [floating-point] exceptions"
   (F.7.3/1 of the August 3, 1998 draft of C99), and that during program
   execution, "a programmer can safely assume default modes (or be unaware of
   them)" (7.6/2, footnote 161 of the August 3, 1998 draft of C99).  Reportedly,
   on Windows there are printer drivers that switch on exceptions.  To avoid
   problems, the SAL_MATH_FPEXCEPTIONS_OFF macro can be used to explicitly
   switch off exceptions (on Windows).
 */
#if defined WNT
#define SAL_MATH_FPEXCEPTIONS_OFF() _control87( _MCW_EM, _MCW_EM )
#else /* WNT */
#define SAL_MATH_FPEXCEPTIONS_OFF()
#endif /* WNT */


/* SAL_MATH_FINITE(d): test double d on INFINITY, NaN et al. */
#if defined(__GNUC__) && !defined(__clang__) // workaround gcc bug 14608
    #if (__GNUC_MINOR__ >= 3) // gcc>=4.3 has a builtin
        #define SAL_MATH_FINITE(d) __builtin_isfinite(d)
    #else
        #define SAL_MATH_FINITE(d) finite(d) // fall back to pre-C99 name
    #endif
#elif defined(__STDC__) && !(defined(LINUX) && defined(__clang__))
    // isfinite() should be available in math.h according to C99,C++99,SUSv3,etc.
    // unless GCC bug 14608 hits us where cmath undefines isfinite() as macro.
    // Clang on Linux runs into that bug too.
    #define SAL_MATH_FINITE(d) isfinite(d)
#elif defined( WNT)
#define SAL_MATH_FINITE(d) _finite(d)
#elif defined OS2
#define SAL_MATH_FINITE(d) finite(d)
#elif defined LINUX || defined UNX
#define SAL_MATH_FINITE(d) finite(d)
#else /* WNT, LINUX, UNX */
#error "SAL_MATH_FINITE not defined"
#endif /* WNT, LINUX, UNX */


/* This needs to be fixed for non--IEEE-754 platforms: */
#if 1 /* IEEE 754 supported */
#if defined OSL_BIGENDIAN

/* IEEE 754 double structures for BigEndian */
union sal_math_Double
{
    struct
    {
        unsigned sign         : 1;
        unsigned exponent     :11;
        unsigned fraction_hi  :20;
        unsigned fraction_lo  :32;
    } inf_parts;
    struct
    {
        unsigned sign         : 1;
        unsigned exponent     :11;
        unsigned qnan_bit     : 1;
        unsigned bits         :19;
        unsigned fraction_lo  :32;
    } nan_parts;
    struct
    {
        unsigned msw          :32;
        unsigned lsw          :32;
    } w32_parts;
    double value;
};

#elif defined OSL_LITENDIAN

/* IEEE 754 double structures for LittleEndian */
union sal_math_Double
{
    struct {
        unsigned fraction_lo  :32;
        unsigned fraction_hi  :20;
        unsigned exponent     :11;
        unsigned sign         : 1;
    } inf_parts;
    struct {
        unsigned fraction_lo  :32;
        unsigned bits         :19;
        unsigned qnan_bit     : 1;
        unsigned exponent     :11;
        unsigned sign         : 1;
    } nan_parts;
    struct
    {
        unsigned lsw          :32;
        unsigned msw          :32;
    } w32_parts;
    double value;
};

#else /* OSL_BIGENDIAN, OSL_LITENDIAN */

#error "neither OSL_BIGENDIAN nor OSL_LITENDIAN"

#endif /* OSL_BIGENDIAN, OSL_LITENDIAN */
#else /* IEEE 754 supported */

#error "don't know how to handle IEEE 754"

#endif /* IEEE 754 supported */


#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* INCLUDED_SAL_MATHCONF_H */
