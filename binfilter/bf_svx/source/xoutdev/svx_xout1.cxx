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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "xoutx.hxx"

#ifdef JOEDEBUG
#endif
namespace binfilter {

#define GLOBALOVERFLOW

/*************************************************************************
|*
|*    XOutputDevice::SetOffset()
|*
|*    Beschreibung
|*    Ersterstellung    08.11.94
|*    Letzte Aenderung  08.11.94
|*
*************************************************************************/

/*N*/ void XOutputDevice::SetOffset( const Point& rOfs )
/*N*/ {
/*N*/ 	if (rOfs!=aOfs)
/*N*/ 	{
/*N*/ 		Point aDelta(rOfs);
/*N*/ 		Fraction aFact1(1,1);
/*N*/ 		aDelta-=aOfs;
/*N*/ #ifdef neeJOEDEBUG
/*N*/ 		DebWrite("XOutputDevice::SetOffset(");
/*N*/ 		DebOut(rOfs);
/*N*/ 		DebWrite("Alter Offset: ");
/*N*/ 		DebOut(aOfs);
/*N*/ 		DebWrite("Alter Origin: ");
/*N*/ 		DebOut(pOut->GetMapMode().GetOrigin());
/*N*/ 		DebWrite("Delta: ");
/*N*/ 		DebOut(aDelta);
/*N*/ #endif
//#ifdef JOEDEBUG
////        pOut->SetMapMode(MapMode(MAP_RELATIVE,Point(-aOfs.X(),-aOfs.Y()),aFact1,aFact1));
////        pOut->SetMapMode(MapMode(MAP_RELATIVE,rOfs,aFact1,aFact1));
/*N*/ 		pOut->SetMapMode(MapMode(MAP_RELATIVE,aDelta,aFact1,aFact1));
//#else
//        MapMode aMap(pOut->GetMapMode());
//        Point   aOrg(aMap.GetOrigin());
//        aOrg.X()+=aDelta.X();
//        aOrg.Y()+=aDelta.Y();
//        aMap.SetOrigin(aOrg);
//        pOut->SetMapMode(aMap);
//#endif
/*N*/ 		aOfs=rOfs;
/*N*/ #ifdef neeJOEDEBUG
/*N*/ 		DebWrite("Neuer Offset: ");
/*N*/ 		DebOut(aOfs);
/*N*/ 		DebWrite("Neuer Origin: ");
/*N*/ 		DebOut(pOut->GetMapMode().GetOrigin());
/*N*/ #endif
/*N*/ 	}
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
