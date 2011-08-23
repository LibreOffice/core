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


#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
namespace binfilter {

/*	Immer:
    -	Zuruecksetzen des Cursorstacks
    -	Timer nachtriggern
    -	gfs. GCAttr

    bei Selektion
    -	SttSelect()

    sonst
    -	EndSelect()
 */

const long nReadOnlyScrollOfst = 10;

/*------------------------------------------------------------------------
 Beschreibung:	Spaltenweises Springen
 Parameter: 	mit oder ohne SSelection
 Return:		Erfolg oder Misserfolg
------------------------------------------------------------------------*/


/*N*/ BOOL SwWrtShell::PopCrsr(BOOL bUpdate, BOOL bSelect)
/*N*/ {
/*N*/ 	if( 0 == pCrsrStack)
/*N*/ 		return FALSE;

/*?*/ 	const BOOL bValidPos = pCrsrStack->bValidCurPos;
/*?*/ 	if( bUpdate && bValidPos )
/*?*/ 	{
/*?*/ 			// falls ein Vorgaenger auf dem Stack steht, dessen Flag fuer eine
/*?*/ 			// gueltige Position verwenden.
/*?*/ 		SwRect aTmpArea(VisArea());
/*?*/ 		aTmpArea.Pos().Y() -= pCrsrStack->lOffset;
/*?*/ 		if( aTmpArea.IsInside( pCrsrStack->aDocPos ) )
/*?*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( bSelect )
/*?*/ 		}
/*?*/ 			// Falls eine Verschiebung zwischen dem sichtbaren Bereich
/*?*/ 			// und der gemerkten Cursorpositionen auftritt, werden
/*?*/ 			// alle gemerkten Positionen weggeschmissen
/*?*/ 		else
/*?*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 _ResetCursorStack();
/*?*/ 			return FALSE;
/*?*/ 		}
/*?*/ 	}
/*?*/ 	CrsrStack *pTmp = pCrsrStack;
/*?*/ 	pCrsrStack = pCrsrStack->pNext;
/*?*/ 	delete pTmp;
/*?*/ 	if( 0 == pCrsrStack )
/*?*/ 	{
/*?*/ 		ePageMove = MV_NO;
/*?*/ 		bDestOnStack = FALSE;
/*?*/ 	}
/*?*/ 	return bValidPos;
/*N*/ }

}
