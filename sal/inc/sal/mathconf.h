/*************************************************************************
 *
 *  $RCSfile: mathconf.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2002-11-06 15:48:48 $
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
#include <limits.h>

#if defined SOLARIS
#include <ieeefp.h>
#endif /* SOLARIS */

#if defined __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @descr
    Handle FP errors using exceptions and setjmp/longjmp? DON'T in C++!
    Switch off with SAL_MATH_FPCONTROL() and just calculate, test results with
    SAL_MATH_FINITE(d), 0==error (INFinitiy, NaN), else ok.
 */
#if 0
#define SAL_MATH_FPSIGNAL_JUMP 1
#else /* 0 */
#define SAL_MATH_FPSIGNAL_JUMP 0
#endif /* 0 */


/* signal() ID for FP exceptions */
#if defined RS6000
#define SAL_MATH_SIGFPE SIGTRAP
#else /* RS6000 */
#define SAL_MATH_SIGFPE SIGFPE
#endif /* RS6000 */


/* SAL_MATH_FPCONTROL(): switch FP exceptions on/off, depending on
   SAL_MATH_FPSIGNAL_JUMP
   SAL_MATH_FPRESET(): reinitialize math package
*/
#if defined WNT || defined WIN

#define SAL_MATH_FPEXCEPTIONS_ON()  _control87( _MCW_EM, 0 )
#define SAL_MATH_FPEXCEPTIONS_OFF() _control87( _MCW_EM, _MCW_EM )
#define SAL_MATH_FPRESET()          _fpreset()

#elif defined OS2

#define SAL_MATH_FPEXCEPTIONS_ON() _control87( MCW_EM, 0 )
#define SAL_MATH_FPEXCEPTIONS_OFF() _control87( MCW_EM, MCW_EM )
#define SAL_MATH_FPRESET()          _fpreset()

#elif defined RS6000

#define SAL_MATH_FPEXCEPTIONS_ON()  fp_enable_all()
#define SAL_MATH_FPEXCEPTIONS_OFF() fp_disable_all()
#define SAL_MATH_FPRESET()

#elif defined LINUX

#include <fpu_control.h>

void sal_math_setfpucw( fpu_control_t set );

#if defined POWERPC
/* set bit to 1 to enable that exception */
/* _FPU_MASK_ZM | _FPU_MASK_OM | _FPU_MASK_UM | _FPU_MASK_IM */
#define SAL_MATH_FPEXCEPTIONS_ON() sal_math_setfpucw(_FPU_DEFAULT | 0x000000F0)
#define SAL_MATH_FPEXCEPTIONS_OFF() sal_math_setfpucw(_FPU_DEFAULT)
#else /* POWERPC */
#define SAL_MATH_FPEXCEPTIONS_ON()  sal_math_setfpucw(_FPU_DEFAULT & ~0x001F)
#define SAL_MATH_FPEXCEPTIONS_OFF() sal_math_setfpucw(_FPU_IEEE)
#endif /* POWERPC */
#define SAL_MATH_FPRESET()

#else /* WNT, WIN, OS2, RS6000, LINUX */

#define SAL_MATH_FPEXCEPTIONS_ON()
#define SAL_MATH_FPEXCEPTIONS_OFF()
#define SAL_MATH_FPRESET()

#endif /* WNT, WIN, OS2, RS6000, LINUX */


#if SAL_MATH_FPSIGNAL_JUMP
#define SAL_MATH_FPCONTROL() SAL_MATH_FPEXCEPTIONS_ON()
#else /* SAL_MATH_FPSIGNAL_JUMP */
#define SAL_MATH_FPCONTROL() SAL_MATH_FPEXCEPTIONS_OFF()
#endif /* SAL_MATH_FPSIGNAL_JUMP */


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
