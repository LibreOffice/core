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

#include "ring.hxx"
namespace binfilter {


/*************************************************************************
|*
|*    Ring::Ring()
|*
|*    Ersterstellung    VB	02.07.91
|*    Letzte Aenderung  JP 10.10.97
|*
*************************************************************************/

/*N*/ Ring::Ring( Ring *pObj )
/*N*/ {
/*N*/ 	if( !pObj )
/*N*/ 		pNext = this, pPrev = this;
/*N*/ 	else
/*N*/ 	{
/*?*/ 		pNext = pObj;
/*?*/ 		pPrev = pObj->pPrev;
/*?*/ 		pObj->pPrev = this;
/*?*/ 		pPrev->pNext = this;
/*N*/ 	}
}

/*************************************************************************
|*
|*    Ring::~Ring()
|*
|*    Ersterstellung    VB	02.07.91
|*    Letzte Aenderung  JP 10.10.97
|*
*************************************************************************/

/*N*/ Ring::~Ring()
/*N*/ {
/*N*/ 	pNext->pPrev = pPrev;
/*N*/ 	pPrev->pNext = pNext;
/*N*/ }

/*************************************************************************
|*
|*    Ring::MoveTo
|*
|*    Ersterstellung    VB 4.3.91
|*    Letzte Aenderung  JP 10.10.97
|*
*************************************************************************/

/*N*/ void Ring::MoveTo(Ring *pDestRing)
/*N*/ {
/*N*/ 	// loeschen aus dem alten
/*N*/ 	pNext->pPrev = pPrev;
/*N*/ 	pPrev->pNext = pNext;
/*N*/ 
/*N*/ 	// im neuen einfuegen
/*N*/ 	if( pDestRing )
/*N*/ 	{
/*N*/ 		pNext = pDestRing;
/*N*/ 		pPrev = pDestRing->pPrev;
/*N*/ 		pDestRing->pPrev = this;
/*N*/ 		pPrev->pNext = this;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pNext = pPrev = this;
/*N*/ 
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
