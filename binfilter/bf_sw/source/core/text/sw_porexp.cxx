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

#include <inftxt.hxx>
#include <porexp.hxx>
namespace binfilter {

/*************************************************************************
 *						class SwExpandPortion
 *************************************************************************/


/*************************************************************************
 *				virtual SwExpandPortion::GetExpTxt()
 *************************************************************************/


/*************************************************************************
 *              virtual SwExpandPortion::HandlePortion()
 *************************************************************************/


/*************************************************************************
 *				virtual SwExpandPortion::GetTxtSize()
 *************************************************************************/

/*N*/ SwPosSize SwExpandPortion::GetTxtSize( const SwTxtSizeInfo &rInf ) const
/*N*/ {
/*N*/ 	SwTxtSlot aDiffTxt( &rInf, this );
/*N*/ 	return rInf.GetTxtSize();
/*N*/ }

/*************************************************************************
 *                 virtual SwExpandPortion::Format()
 *************************************************************************/

// 5010: Exp und Tabs

/*N*/ sal_Bool SwExpandPortion::Format( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	SwTxtSlotLen aDiffTxt( &rInf, this );
/*N*/ 	const xub_StrLen nFullLen = rInf.GetLen();
/*N*/ 
/*N*/ 	// So komisch es aussieht, die Abfrage auf GetLen() muss wegen der
/*N*/ 	// ExpandPortions _hinter_ aDiffTxt (vgl. SoftHyphs)
/*N*/ 	// sal_False returnen wegen SetFull ...
/*N*/ 	if( !nFullLen )
/*N*/ 	{
/*N*/ 		// nicht Init(), weil wir Hoehe und Ascent brauchen
/*?*/ 		Width(0);
/*?*/ 		return sal_False;
/*N*/ 	}
/*N*/ 	return SwTxtPortion::Format( rInf );
/*N*/ }

/*************************************************************************
 *				virtual SwExpandPortion::Paint()
 *************************************************************************/


/*************************************************************************
 *						class SwBlankPortion
 *************************************************************************/

/*N*/ SwLinePortion *SwBlankPortion::Compress() { return this; }

/*************************************************************************
 *                 SwBlankPortion::MayUnderFlow()
 *************************************************************************/

// 5497: Es gibt schon Gemeinheiten auf der Welt...
// Wenn eine Zeile voll mit HardBlanks ist und diese ueberlaeuft,
// dann duerfen keine Underflows generiert werden!
// Komplikationen bei Flys...

/*N*/ MSHORT SwBlankPortion::MayUnderFlow( const SwTxtFormatInfo &rInf,
/*N*/ 	xub_StrLen nIdx, sal_Bool bUnderFlow ) const
/*N*/ {
/*N*/ 	if( rInf.StopUnderFlow() )
/*N*/ 		return 0;
/*N*/ 	const SwLinePortion *pPos = rInf.GetRoot();
/*N*/ 	if( pPos->GetPortion() )
/*N*/ 		pPos = pPos->GetPortion();
/*N*/ 	while( pPos && pPos->IsBlankPortion() )
/*?*/ 		pPos = pPos->GetPortion();
/*N*/ 	if( !pPos || !rInf.GetIdx() || ( !pPos->GetLen() && pPos == rInf.GetRoot() ) )
/*N*/ 		return 0; // Nur noch BlankPortions unterwegs
/*N*/ 	// Wenn vor uns ein Blank ist, brauchen wir kein Underflow ausloesen,
/*N*/ 	// wenn hinter uns ein Blank ist, brauchen wir kein Underflow weiterreichen
/*N*/ 	if( bUnderFlow && CH_BLANK == rInf.GetTxt().GetChar( nIdx + 1) )
/*N*/ 		return 0;
/*N*/ 	if( nIdx && !((SwTxtFormatInfo&)rInf).GetFly() )
/*N*/ 	{
/*N*/ 		while( pPos && !pPos->IsFlyPortion() )
/*N*/ 			pPos = pPos->GetPortion();
/*N*/ 		if( !pPos )
/*N*/ 		{
/*N*/ 		//Hier wird ueberprueft, ob es in dieser Zeile noch sinnvolle Umbrueche
/*N*/ 		//gibt, Blanks oder Felder etc., wenn nicht, kein Underflow.
/*N*/ 		//Wenn Flys im Spiel sind, lassen wir das Underflow trotzdem zu.
/*N*/ 			xub_StrLen nBlank = nIdx;
/*N*/ 			while( --nBlank > rInf.GetLineStart() )
/*N*/ 			{
/*N*/ 				const xub_Unicode cCh = rInf.GetChar( nBlank );
/*N*/ 				if( CH_BLANK == cCh ||
/*N*/ 					(( CH_TXTATR_BREAKWORD == cCh || CH_TXTATR_INWORD == cCh )
/*N*/ 						&& rInf.HasHint( nBlank ) ) )
/*N*/ 					break;
/*N*/ 			}
/*N*/ 			if( nBlank <= rInf.GetLineStart() )
/*N*/ 				return 0;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	xub_Unicode cCh;
/*N*/ 	if( nIdx < 2 || CH_BLANK == (cCh = rInf.GetChar( nIdx - 1 )) )
/*N*/ 		return 1;
/*N*/ 	if( CH_BREAK == cCh )
/*N*/ 		return 0;
/*N*/ 	return 2;
/*N*/ }

/*************************************************************************
 *				   virtual SwBlankPortion::FormatEOL()
 *************************************************************************/
// Format end of Line


/*************************************************************************
 *                 virtual SwBlankPortion::Format()
 *************************************************************************/

// 7771: UnderFlows weiterreichen und selbst ausloesen!
/*N*/ sal_Bool SwBlankPortion::Format( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	const sal_Bool bFull = rInf.IsUnderFlow() || SwExpandPortion::Format( rInf );
/*N*/ 	if( bFull && MayUnderFlow( rInf, rInf.GetIdx(), rInf.IsUnderFlow() ) )
/*N*/ 	{
/*N*/ 		Truncate();
/*N*/ 		rInf.SetUnderFlow( this );
/*N*/ 		if( rInf.GetLast()->IsKernPortion() )
/*?*/ 			rInf.SetUnderFlow( rInf.GetLast() );
/*N*/ 	}
/*N*/ 	return bFull;
/*N*/ }

/*************************************************************************
 *				   virtual SwBlankPortion::Paint()
 *************************************************************************/


/*************************************************************************
 *				virtual SwBlankPortion::GetExpTxt()
 *************************************************************************/

/*N*/ sal_Bool SwBlankPortion::GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const
/*N*/ {
/*N*/ 	rTxt = cChar;
/*N*/ 	return sal_True;
/*N*/ }

/*************************************************************************
 *              virtual SwBlankPortion::HandlePortion()
 *************************************************************************/


/*************************************************************************
 *                      class SwPostItsPortion
 *************************************************************************/




/*************************************************************************
 *                 virtual SwPostItsPortion::Format()
 *************************************************************************/


/*************************************************************************
 *              virtual SwPostItsPortion::GetExpTxt()
 *************************************************************************/


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
