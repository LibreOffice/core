/*************************************************************************
 *
 *  $RCSfile: mathconf.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 16:45:58 $
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#if !defined INCLUDED_SAL_MATHCONF_H
#define INCLUDED_SAL_MATHCONF_H

#include "osl/endian.h"

#include <float.h>

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
#if defined WNT
#define SAL_MATH_FINITE(d) _finite(d)
#elif defined MAC
#define SAL_MATH_FINITE(d) isfinite(d)
#elif defined LINUX || defined UNX
#define SAL_MATH_FINITE(d) finite(d)
#else /* WNT, MAC, LINUX, UNX */
#error "SAL_MATH_FINITE not defined"
#endif /* WNT, MAC, LINUX, UNX */


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
