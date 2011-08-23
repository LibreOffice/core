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

#include <sal/macros.h>
#include "dialogs.hrc"
#include "paperinf.hxx"
namespace binfilter {

#define SVX_PAPER_OFFSET	3	// Anfang: enum Paper A3 - SvxPaper A0; Diff=3

// STATIC DATA -----------------------------------------------------------

static Size __FAR_DATA aDinTab[] =
{
    Size(lA0Width,lA0Height),			// A0
    Size(lA1Width,lA1Height),			// A1
    Size(lA2Width,lA2Height),			// A2
    Size(lA3Width,lA3Height),			// A3
    Size(lA4Width,lA4Height),			// A4
    Size(lA5Width,lA5Height),			// A5
    Size(lB4Width, lB4Height),			// B4
    Size(lB5Width,lB5Height),			// B5
    Size(lLetterWidth,lLetterHeight),	// LETTER
    Size(lLegalWidth,lLegalHeight), 	// LEGAL
    Size(lTabloidWidth,lTabloidHeight), // TABLOID
    Size(0, 0), 						// USER
    Size(lB6Width, lB6Height),			// B6
    Size(lC4Width, lC4Height),			// C4
    Size(lC5Width, lC5Height),			// C5
    Size(lC6Width, lC6Height),			// C6
    Size(lC65Width, lC65Height),		// C65
    Size(lDLWidth, lDLHeight),			// DL
    Size(lDiaWidth,lDiaHeight ),		// DIA
    Size(lScreenWidth, lScreenHeight),	// SCREEN
    Size(lAWidth, lAHeight),			// A
    Size(lBWidth, lBHeight),			// B
    Size(lCWidth, lCHeight),			// C
    Size(lDWidth, lDHeight),			// D
    Size(lEWidth, lEHeight), 			// E
    Size(lExeWidth, lExeHeight),		// Executive
    Size(lLegal2Width, lLegal2Height),	// Legal2
    Size(lMonarchWidth, lMonarchHeight),// Monarch
    Size(lCom675Width, lCom675Height),	// COM-6 3/4
    Size(lCom9Width, lCom9Height),		// COM-9
    Size(lCom10Width, lCom10Height),	// COM-10
    Size(lCom11Width, lCom11Height),	// COM-11
    Size(lCom12Width, lCom12Height),	// COM-12
    Size(lKai16Width, lKai16Height),	// 16 kai
    Size(lKai32Width, lKai32Height),	// 32 kai
    Size(lKai32BigWidth, lKai32BigHeight), // 32 kai gross
    Size(lJISB4Width, lJISB4Height),       // B4 (JIS)
    Size(lJISB5Width, lJISB5Height),       // B5 (JIS)
    Size(lJISB6Width, lJISB6Height)        // B6 (JIS)
};

static const int nTabSize = SAL_N_ELEMENTS(aDinTab);

// -----------------------------------------------------------------------

/*N*/ long TwipsTo100thMM( long nIn )
/*N*/ {
/*N*/ 	long nRet = OutputDevice::LogicToLogic( nIn, MAP_TWIP, MAP_100TH_MM );
/*N*/ 	long nTmp = nRet % 10;
/*N*/ 
/*N*/ 	if ( nTmp )
/*N*/ 		nRet += 10 - nTmp;
/*N*/ 	return nRet;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ Size ConvertTo100thMM( Size& rSize )
/*N*/ {
/*N*/ 	// Convert form TWIPS to 100TH_MM
/*N*/ 	long nW = TwipsTo100thMM( rSize.Width() );
/*N*/ 	long nH = TwipsTo100thMM( rSize.Height() );
/*N*/ 
/*N*/ 	rSize.Width() = nW;
/*N*/ 	rSize.Height() = nH;
/*N*/ 	return rSize;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:	Ist der Printer gueltig
 --------------------------------------------------------------------*/

/*?*/ inline BOOL IsValidPrinter( const Printer* pPtr )
/*?*/ {
/*?*/ 	return pPtr->GetName().Len() ? TRUE : FALSE;
/*?*/ }

/*------------------------------------------------------------------------
 Beschreibung:	Konvertierung eines SV-Defines fuer Papiergroesse in
                Twips.
                Funktioniert logischerweise nicht fuer User Groessen
                (ASSERT).
------------------------------------------------------------------------*/

/*N*/ Size SvxPaperInfo::GetPaperSize( SvxPaper ePaper, MapUnit eUnit )
/*N*/ {
/*N*/ 	DBG_ASSERT( ePaper < nTabSize, "Tabelle der Papiergroessen ueberindiziert" );
/*N*/ 	DBG_ASSERT( eUnit == MAP_TWIP || eUnit == MAP_100TH_MM, "this MapUnit not supported" );
/*N*/ 	Size aSize = aDinTab[ePaper];	// in Twips
/*N*/ 
/*N*/ 	if ( eUnit == MAP_100TH_MM )
/*N*/ 		ConvertTo100thMM( aSize );
/*N*/ 	return aSize;
/*N*/ }

/*------------------------------------------------------------------------
 Beschreibung:	Papiergroesse der Druckers liefern, aligned auf
                die eigenen Groessen.
                Falls kein Printer im System eingestellt ist,
                wird DIN A4 Portrait als Defaultpapiergroesse geliefert.
------------------------------------------------------------------------*/

/*N*/ Size SvxPaperInfo::GetPaperSize( const Printer* pPrinter )
/*N*/ {
/*N*/ 	if ( !IsValidPrinter(pPrinter) )
/*?*/ 		return GetPaperSize( SVX_PAPER_A4 );
/*N*/ 	const SvxPaper ePaper = (SvxPaper)(pPrinter->GetPaper() + SVX_PAPER_OFFSET);
/*N*/ 
/*N*/ 	if ( ePaper == SVX_PAPER_USER )
/*N*/ 	{
/*N*/ 			// Orientation nicht beruecksichtigen, da durch SV bereits
/*N*/ 			// die richtigen Masze eingestellt worden sind.
/*N*/ 		Size aPaperSize = pPrinter->GetPaperSize();
/*N*/ 		const Size aInvalidSize;
/*N*/ 
/*N*/ 		if ( aPaperSize == aInvalidSize )
/*?*/ 			return GetPaperSize(SVX_PAPER_A4);
/*N*/ 		MapMode aMap1 = pPrinter->GetMapMode();
/*N*/ 		MapMode aMap2;
/*N*/ 
/*N*/ 		if ( aMap1 == aMap2 )
/*N*/ 			aPaperSize =
/*N*/ 				pPrinter->PixelToLogic( aPaperSize, MapMode( MAP_TWIP ) );
/*N*/ 		return aPaperSize;
/*N*/ 	}
/*N*/ 
/*N*/ 	const Orientation eOrient = pPrinter->GetOrientation();
/*N*/ 	Size aSize( GetPaperSize( ePaper ) );
/*N*/ 		// bei Landscape die Seiten tauschen, ist bei SV schon geschehen
/*N*/ 	if ( eOrient == ORIENTATION_LANDSCAPE )
/*?*/ 		Swap( aSize );
/*N*/ 	return aSize;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
