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

#ifdef RS6000

#include <fptrap.h>
#include <fpxcp.h>

#elif defined ( MAC )

#include <MAC_START.h>
#include <fp.h>
#include <MAC_END.h>

#endif


#include <float.h>
#ifdef SOLARIS
#include <ieeefp.h>
#endif
#include <signal.h>

#include "subtotal.hxx"
#include "interpre.hxx"
namespace binfilter {

// STATIC DATA -----------------------------------------------------------

jmp_buf SubTotal::aGlobalJumpBuf;

// -----------------------------------------------------------------------

/*N*/ SubTotal::SubTotal()
/*N*/ {
/*N*/	nIndex = 0;				//! test fuer Pivot
/*N*/ 
/*N*/ 	nCount	   = 0;
/*N*/ 	nCount2	   = 0;
/*N*/ 	nSum	   = 0.0;
/*N*/ 	nSumSqr	   = 0.0;
/*N*/ 	nMax	   = -MAXDOUBLE;
/*N*/ 	nMin	   = MAXDOUBLE;
/*N*/ 	nProduct   = 1.0;
/*N*/ 	bSumOk     = TRUE;
/*N*/ 	bSumSqrOk  = TRUE;
/*N*/ 	bProductOk = TRUE;
/*N*/ }


/*N*/ SubTotal::~SubTotal()
/*N*/ {
/*N*/ }




/*N*/ void SubTotal::Update( double nVal )
/*N*/ {
/*N*/ 	SAL_MATH_FPEXCEPTIONS_OFF();
/*N*/     nCount++;
/*N*/     nCount2++;
/*N*/     if (nVal > nMax) nMax = nVal;
/*N*/     if (nVal < nMin) nMin = nVal;
/*N*/     nProgress = 0;
/*N*/     if (bSumOk) nSum += nVal;
/*N*/     nProgress = 1;
/*N*/     if (bProductOk) nProduct *= nVal;
/*N*/     nProgress = 2;
/*N*/     if (bSumSqrOk) nSumSqr += nVal*nVal;
/*N*/ 	if (!::rtl::math::isFinite(nSum))
/*N*/ 		bSumOk = FALSE;
/*N*/ 	if (!::rtl::math::isFinite(nProduct))
/*N*/ 		bProductOk = FALSE;
/*N*/ 	if (!::rtl::math::isFinite(nSumSqr))
/*N*/ 		bSumSqrOk = FALSE;
/*N*/ }








/*N*/ BOOL SubTotal::SafePlus(double& fVal1, double fVal2)
/*N*/ {
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	SAL_MATH_FPEXCEPTIONS_OFF();
/*N*/     fVal1 += fVal2;
/*N*/ 	if (!::rtl::math::isFinite(fVal1))
/*N*/ 	{
/*N*/ 		bOk = FALSE;
/*N*/ 		if (fVal2 > 0.0)
/*N*/ 			fVal1 = DBL_MAX;
/*N*/ 		else
/*N*/ 			fVal1 = -DBL_MAX;
/*N*/ 	}
/*N*/ 	return bOk;
/*N*/ }


/*N*/ BOOL SubTotal::SafeMult(double& fVal1, double fVal2)
/*N*/ {
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	SAL_MATH_FPEXCEPTIONS_OFF();
/*N*/     fVal1 *= fVal2;
/*N*/ 	if (!::rtl::math::isFinite(fVal1))
/*N*/ 	{
/*N*/ 		bOk = FALSE;
/*N*/ 		fVal1 = DBL_MAX;
/*N*/ 	}
/*N*/ 	return bOk;
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
