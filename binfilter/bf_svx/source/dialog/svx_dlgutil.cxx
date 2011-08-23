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

// include ---------------------------------------------------------------

#include <limits.h>

#include "dialogs.hrc"

#include "dlgutil.hxx"
namespace binfilter {

/*N*/ FieldUnit GetModuleFieldUnit( const SfxItemSet* pSet )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	FieldUnit eUnit = FUNIT_INCH;
/*N*/ 	return eUnit;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ long CalcToUnit( float nIn, SfxMapUnit eUnit )
/*N*/ {
/*N*/ 	// nIn ist in Points
/*N*/ 
/*N*/ 	DBG_ASSERT( eUnit == SFX_MAPUNIT_TWIP 		||
/*N*/ 				eUnit == SFX_MAPUNIT_100TH_MM 	||
/*N*/ 				eUnit == SFX_MAPUNIT_10TH_MM 	||
/*N*/ 				eUnit == SFX_MAPUNIT_MM 		||
/*N*/ 				eUnit == SFX_MAPUNIT_CM, "this unit is not implemented" );
/*N*/ 
/*N*/ 	float nTmp = nIn;
/*N*/ 
/*N*/ 	if ( SFX_MAPUNIT_TWIP != eUnit )
/*N*/ 		nTmp = nIn * 10 / 567;
/*N*/ 
/*N*/ 	switch ( eUnit )
/*N*/ 	{
/*N*/ 		case SFX_MAPUNIT_100TH_MM:	nTmp *= 100; break;
/*?*/ 		case SFX_MAPUNIT_10TH_MM:	nTmp *= 10;	 break;
/*?*/ 		case SFX_MAPUNIT_MM:					 break;
/*?*/ 		case SFX_MAPUNIT_CM:		nTmp /= 10;	 break;
/*N*/ 	}
/*N*/ 
/*N*/ 	nTmp *= 20;
/*N*/ 	long nRet = (long)nTmp;
/*N*/ 	return nRet;
/*N*/ //!	return (long)(nTmp * 20);
/*N*/ }

// -----------------------------------------------------------------------

/*?*/ long ItemToControl( long nIn, SfxMapUnit eItem, SfxFieldUnit eCtrl )
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 //STRIP001 	long nOut = 0;
/*?*/ }

// -----------------------------------------------------------------------

/*N*/ long CalcToPoint( long nIn, SfxMapUnit eUnit, USHORT nFaktor )
/*N*/ {
/*N*/ 	DBG_ASSERT( eUnit == SFX_MAPUNIT_TWIP 		||
/*N*/ 				eUnit == SFX_MAPUNIT_100TH_MM 	||
/*N*/ 				eUnit == SFX_MAPUNIT_10TH_MM 	||
/*N*/ 				eUnit == SFX_MAPUNIT_MM 		||
/*N*/ 				eUnit == SFX_MAPUNIT_CM, "this unit is not implemented" );
/*N*/ 
/*N*/ 	long nRet = 0;
/*N*/ 
/*N*/ 	if ( SFX_MAPUNIT_TWIP == eUnit )
/*N*/ 		nRet = nIn;
/*N*/ 	else
/*N*/ 		nRet = nIn * 567;
/*N*/ 
/*N*/ 	switch ( eUnit )
/*N*/ 	{
/*N*/ 		case SFX_MAPUNIT_100TH_MM:	nRet /= 100; break;
/*?*/ 		case SFX_MAPUNIT_10TH_MM:	nRet /= 10;	 break;
/*?*/ 		case SFX_MAPUNIT_MM:					 break;
/*?*/ 		case SFX_MAPUNIT_CM:		nRet *= 10;	 break;
/*N*/ 	}
/*N*/ 
/*N*/ 	// ggf. aufrunden
/*N*/ 	if ( SFX_MAPUNIT_TWIP != eUnit )
/*N*/ 	{
/*N*/ 		long nMod = 10;
/*N*/ 		long nTmp = nRet % nMod;
/*N*/ 
/*N*/ 		if ( nTmp >= 4 )
/*N*/ 			nRet += 10 - nTmp;
/*N*/ 		nRet /= 10;
/*N*/ 	}
/*N*/ 	return nRet * nFaktor / 20;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
