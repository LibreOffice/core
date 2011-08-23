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



#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>	// SwViewOptions
#endif
#ifndef _PORHYPH_HXX
#include <porhyph.hxx>	//
#endif
#ifndef _ITRFORM2_HXX
#include <itrform2.hxx> //
#endif
#ifndef _GUESS_HXX
#include <guess.hxx>	//
#endif
namespace binfilter {

#ifdef DBG_UTIL
extern const sal_Char *GetLangName( const MSHORT nLang );
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::i18n;

/*************************************************************************
 *						SwTxtFormatInfo::HyphWord()
 *************************************************************************/


/*************************************************************************
 *						SwTxtFrm::Hyphenate
 *
 * Wir formatieren eine Zeile fuer die interaktive Trennung
 *************************************************************************/


/*************************************************************************
 *						SwTxtFormatter::Hyphenate
 *
 * Wir formatieren eine Zeile fuer die interaktive Trennung
 *************************************************************************/
// Wir koennen davon ausgehen, dass bereits formatiert wurde.
// Fuer die CeBIT'93 gehen wir den einfachen, sicheren Weg:
// Die Zeile wird einfach neu formatiert, der Hyphenator wird dann
// so vorbereitet, wie ihn die UI erwartet.
// Hier stehen natuerlich enorme Optimierungsmoeglichkeiten offen.



/*************************************************************************
 *						SwTxtPortion::CreateHyphen()
 *************************************************************************/

/*N*/ sal_Bool SwTxtPortion::CreateHyphen( SwTxtFormatInfo &rInf, SwTxtGuess &rGuess )
/*N*/ {
/*N*/ 	ASSERT( !pPortion, "SwTxtPortion::Hyphenate: another portion, another planet..." );
/*N*/ 	if( rInf.IsHyphForbud() ||
/*N*/ 		pPortion || // robust
/*N*/ 		// Mehrzeilige Felder duerfen nicht interaktiv getrennt werden.
/*N*/ 		( rInf.IsInterHyph() && InFldGrp() ) )
/*N*/ 		return sal_False;
/*N*/
/*N*/ 	Reference< XHyphenatedWord >  xHyphWord = rGuess.HyphWord();
/*N*/ 	SwHyphPortion *pHyphPor = NULL;
/*N*/ 	xub_StrLen nPorEnd = 0;
/*N*/ 	SwTxtSizeInfo aInf( rInf );
/*N*/
/*N*/ 	// first case: hyphenated word has alternative spelling
/*N*/ 	if ( xHyphWord.is() && xHyphWord->isAlternativeSpelling() ) {
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	SvxAlternativeSpelling aAltSpell;
/*N*/ 	} else {
/*N*/ 		// second case: no alternative spelling
/*N*/ 		SwHyphPortion aHyphPor;
/*N*/ 		aHyphPor.SetLen( 1 );
/*N*/
/*N*/ 		static const void* pLastMagicNo = 0;
/*N*/ 		static KSHORT aMiniCacheH = 0, aMiniCacheW = 0;
/*N*/ 		const void* pTmpMagic;
/*N*/ 		MSHORT nFntIdx;
/*N*/ 		rInf.GetFont()->GetMagic( pTmpMagic, nFntIdx, rInf.GetFont()->GetActual() );
/*N*/ 		if( !pLastMagicNo || pLastMagicNo != pTmpMagic ) {
/*N*/ 			pLastMagicNo = pTmpMagic;
/*N*/ 			(SwPosSize&)aHyphPor = aHyphPor.GetTxtSize( rInf );
/*N*/ 			aMiniCacheH = aHyphPor.Height(), aMiniCacheW = aHyphPor.Width();
/*N*/ 		} else {
/*N*/ 			aHyphPor.Height( aMiniCacheH ), aHyphPor.Width( aMiniCacheW );
/*N*/ 		}
/*N*/ 		aHyphPor.SetLen( 0 );
/*N*/ 		pHyphPor = new SwHyphPortion( aHyphPor );
/*N*/
/*N*/ 		pHyphPor->SetWhichPor( POR_HYPH );
/*N*/
/*N*/ 		// values required for this
/*N*/         nPorEnd = xHyphWord->getHyphenPos() + 1 + rGuess.BreakStart()
/*N*/                 - rGuess.FieldDiff();
/*N*/ 	}
/*N*/
/*N*/ 	// portion end must be in front of us
/*N*/ 	// we do not put hyphens at start of line
/*N*/ 	if ( nPorEnd > rInf.GetIdx() ||
/*N*/ 		 ( nPorEnd == rInf.GetIdx() && rInf.GetLineStart() != rInf.GetIdx() ) )
/*N*/ 	{
/*N*/ 		aInf.SetLen( nPorEnd - rInf.GetIdx() );
/*N*/ 		pHyphPor->SetAscent( GetAscent() );
/*N*/ 		SetLen( aInf.GetLen() );
/*N*/ 		CalcTxtSize( aInf );
/*N*/
/*N*/ 		Insert( pHyphPor );
/*N*/
/*N*/ 		short nKern = rInf.GetFont()->CheckKerning();
/*N*/ 		if( nKern )
/*N*/ 			new SwKernPortion( *this, nKern );
/*N*/
/*N*/ 		return sal_True;
/*N*/ 	}
/*N*/
/*N*/ 	// last exit for the lost
/*N*/ 	delete pHyphPor;
/*N*/ 	BreakCut( rInf, rGuess );
/*N*/ 	return sal_False;
/*N*/ }


/*************************************************************************
 *              virtual SwHyphPortion::GetExpTxt()
 *************************************************************************/

/*N*/ sal_Bool SwHyphPortion::GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const
/*N*/ {
/*N*/ 	rTxt = '-';
/*N*/ 	return sal_True;
/*N*/ }

/*************************************************************************
 *              virtual SwHyphPortion::HandlePortion()
 *************************************************************************/


/*************************************************************************
 *                 virtual SwHyphPortion::Format()
 *************************************************************************/

/*N*/ sal_Bool SwHyphPortion::Format( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	register const SwLinePortion *pLast = rInf.GetLast();
/*N*/ 	Height( pLast->Height() );
/*N*/ 	SetAscent( pLast->GetAscent() );
/*N*/ 	XubString aTxt;
/*N*/
/*N*/ 	if( !GetExpTxt( rInf, aTxt ) )
/*N*/ 		return sal_False;
/*N*/
/*N*/ 	PrtWidth( rInf.GetTxtSize( aTxt ).Width() );
/*N*/ 	const sal_Bool bFull = rInf.Width() <= rInf.X() + PrtWidth();
/*N*/ 	if( bFull && !rInf.IsUnderFlow() ) {
/*?*/ 		Truncate();
/*?*/ 		rInf.SetUnderFlow( this );
/*N*/ 	}
/*N*/
/*N*/ 	return bFull;
/*N*/ }

/*************************************************************************
 *              virtual SwHyphStrPortion::GetExpTxt()
 *************************************************************************/


/*************************************************************************
 *              virtual SwHyphStrPortion::HandlePortion()
 *************************************************************************/


/*************************************************************************
 *                      class SwSoftHyphPortion
 *************************************************************************/

/*N*/ SwLinePortion *SwSoftHyphPortion::Compress() { return this; }

/*N*/ SwSoftHyphPortion::SwSoftHyphPortion() :
/*N*/ 	bExpand(sal_False), nViewWidth(0), nHyphWidth(0)
/*N*/ {
/*N*/ 	SetLen(1);
/*N*/ 	SetWhichPor( POR_SOFTHYPH );
/*N*/ }


/*  Faelle:
 *  1) SoftHyph steht in der Zeile, ViewOpt aus.
 *     -> unsichtbar, Nachbarn unveraendert
 *  2) SoftHyph steht in der Zeile, ViewOpt an.
 *     -> sichtbar, Nachbarn veraendert
 *  3) SoftHyph steht am Zeilenende, ViewOpt aus/an.
 *     -> immer sichtbar, Nachbarn unveraendert
 */


/*************************************************************************
 *                 virtual SwSoftHyphPortion::Format()
 *************************************************************************/

/* Die endgueltige Breite erhalten wir im FormatEOL().
 * In der Underflow-Phase stellen wir fest, ob ueberhaupt ein
 * alternatives Spelling vorliegt. Wenn ja ...
 *
 * Fall 1: "Au-to"
 * 1) {Au}{-}{to}, {to} passt nicht mehr => Underflow
 * 2) {-} ruft Hyphenate => keine Alternative
 * 3) FormatEOL() und bFull = sal_True
 *
 * Fall 2: "Zuc-ker"
 * 1) {Zuc}{-}{ker}, {ker} passt nicht mehr => Underflow
 * 2) {-} ruft Hyphenate => Alternative!
 * 3) Underflow() und bFull = sal_True
 * 4) {Zuc} ruft Hyphenate => {Zuk}{-}{ker}
 */

/*N*/ sal_Bool SwSoftHyphPortion::Format( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	sal_Bool bFull = sal_True;
/*N*/
/*N*/ 	// special case for old german spelling
/*N*/ 	if( rInf.IsUnderFlow()	)
/*N*/ 	{
/*N*/ 		if( rInf.GetSoftHyphPos() )
/*N*/ 			return sal_True;
/*N*/
/*N*/ 		const sal_Bool bHyph = rInf.ChgHyph( sal_True );
/*N*/ 		if( rInf.IsHyphenate() )
/*N*/ 		{
/*N*/ 			rInf.SetSoftHyphPos( rInf.GetIdx() );
/*N*/ 			Width(0);
/*N*/ 			// if the soft hyphend word has an alternative spelling
/*N*/ 			// when hyphenated (old german spelling), the soft hyphen
/*N*/ 			// portion has to trigger an underflow
/*N*/ 			SwTxtGuess aGuess;
/*N*/ 			bFull = rInf.IsInterHyph() ||
/*N*/ 					!aGuess.AlternativeSpelling( rInf, rInf.GetIdx() - 1 );
/*N*/ 		}
/*N*/ 		rInf.ChgHyph( bHyph );
/*N*/
/*N*/ 		if( bFull && !rInf.IsHyphForbud() )
/*N*/ 		{
/*N*/ 			rInf.SetSoftHyphPos(0);
/*N*/ 			FormatEOL( rInf );
/*N*/ 			if ( rInf.GetFly() )
/*?*/ 				rInf.GetRoot()->SetMidHyph( sal_True );
/*N*/ 			else
/*N*/ 				rInf.GetRoot()->SetEndHyph( sal_True );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			rInf.SetSoftHyphPos( rInf.GetIdx() );
/*?*/ 			Truncate();
/*?*/ 			rInf.SetUnderFlow( this );
/*N*/ 		}
/*N*/ 		return sal_True;
/*N*/ 	}
/*N*/
/*N*/ 	rInf.SetSoftHyphPos(0);
/*N*/ 	SetExpand( sal_True );
/*N*/ 	bFull = SwHyphPortion::Format( rInf );
/*N*/ 	SetExpand( sal_False );
/*N*/ 	if( !bFull )
/*N*/ 	{
/*N*/ 		// default-maessig besitzen wir keine Breite, aber eine Hoehe
/*N*/ 		nHyphWidth = Width();
/*N*/ 		Width(0);
/*N*/ 	}
/*N*/ 	return bFull;
/*N*/ }

/*************************************************************************
 *				   virtual SwSoftHyphPortion::FormatEOL()
 *************************************************************************/
// Format end of Line

/*N*/ void SwSoftHyphPortion::FormatEOL( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	if( !IsExpand() )
/*N*/ 	{
/*N*/ 		SetExpand( sal_True );
/*N*/ 		if( rInf.GetLast() == this )
/*?*/ 			rInf.SetLast( FindPrevPortion( rInf.GetRoot() ) );
/*N*/
/*N*/ 		// 5964: alte Werte muessen wieder zurueckgesetzt werden.
/*N*/ 		const KSHORT nOldX	 = rInf.X();
/*N*/ 		const xub_StrLen nOldIdx = rInf.GetIdx();
/*N*/ 		rInf.X( rInf.X() - PrtWidth() );
/*N*/ 		rInf.SetIdx( rInf.GetIdx() - GetLen() );
/*N*/ 		const sal_Bool bFull = SwHyphPortion::Format( rInf );
/*N*/ 		nHyphWidth = Width();
/*N*/
/*N*/ 		// 6976: Eine truebe Sache: Wir werden erlaubterweise breiter,
/*N*/ 		// aber gleich wird noch ein Fly verarbeitet, der eine korrekte
/*N*/ 		// X-Position braucht.
/*N*/ 		if( bFull || !rInf.GetFly() )
/*N*/ 			rInf.X( nOldX );
/*N*/ 		else
/*?*/ 			rInf.X( nOldX + Width() );
/*N*/ 		rInf.SetIdx( nOldIdx );
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *				virtual SwSoftHyphPortion::GetExpTxt()
 *
 * Wir expandieren:
 * - wenn die Sonderzeichen sichtbar sein sollen
 * - wenn wir am Ende der Zeile stehen.
 * - wenn wir vor einem (echten/emuliertem) Zeilenumbruch stehen
 *************************************************************************/

/*N*/ sal_Bool SwSoftHyphPortion::GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const
/*N*/ {
/*N*/ 	if( IsExpand() || ( rInf.OnWin() && rInf.GetOpt().IsSoftHyph() ) ||
/*N*/ 		( GetPortion() && ( GetPortion()->InFixGrp() ||
/*N*/ 		  GetPortion()->IsDropPortion() || GetPortion()->IsLayPortion() ||
/*N*/ 		  GetPortion()->IsParaPortion() || GetPortion()->IsBreakPortion() ) ) )
/*N*/ 	{
/*N*/ 		return SwHyphPortion::GetExpTxt( rInf, rTxt );
/*N*/ 	}
/*N*/ 	return sal_False;
/*N*/ }

}
