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

#ifndef INCLUDED_SAL_MATHCONF_H
#define INCLUDED_SAL_MATHCONF_H

#include "osl/endian.h"

#include <float.h>

#if defined SOLARIS
#include <ieeefp.h>
#endif /* SOLARIS */

#if defined(__cplusplus) && ( defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L )
#include <cmath>
#endif

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
#if !defined SOLARIS && !defined ANDROID \
                     && defined(__cplusplus) \
                     && ( defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L )
#define SAL_MATH_FINITE(d) std::isfinite(d)
#elif defined( WNT)
#define SAL_MATH_FINITE(d) _finite(d)
#elif defined IOS
/* C++ is so nice. This is the only way I could come up with making
 * this actually work in all cases (?), even when <cmath> has been
 * included which #undefs isfinite: copy the definition of isfinite()
 * from <architecture/arm/math.h>
 */
#define SAL_MATH_FINITE(d) \
  ( sizeof (d) == sizeof(float ) ?  __inline_isfinitef((float)(d)) \
  : sizeof (d) == sizeof(double) ?  __inline_isfinited((double)(d)) \
                                 :  __inline_isfinite ((long double)(d)))
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
