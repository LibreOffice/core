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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif


#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>	// SwViewOptions
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _INFTXT_HXX
#include <inftxt.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _PORFTN_HXX
#include <porftn.hxx>   // SwFtnPortion
#endif
namespace binfilter {

using namespace ::com::sun::star;

/*************************************************************************
 *                      class SwFldPortion
 *************************************************************************/

/*N*/ SwLinePortion *SwFldPortion::Compress()
/*N*/ { return (GetLen() || aExpand.Len() || SwLinePortion::Compress()) ? this : 0; }

/*N*/ SwFldPortion *SwFldPortion::Clone( const XubString &rExpand ) const
/*N*/ {
/*N*/ 	SwFont *pNewFnt;
/*N*/ 	if( 0 != ( pNewFnt = pFnt ) )
/*?*/ 		pNewFnt = new SwFont( *pFnt );
/*N*/     SwFldPortion* pClone = new SwFldPortion( rExpand, pNewFnt );
/*N*/     pClone->SetNextOffset( nNextOffset );
/*N*/     return pClone;
/*N*/ }

/*N*/ void SwFldPortion::TakeNextOffset( const SwFldPortion* pFld )
/*N*/ {
/*N*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	ASSERT( pFld, "TakeNextOffset: Missing Source" );
/*N*/ }

/*N*/ SwFldPortion::SwFldPortion( const XubString &rExpand, SwFont *pFnt )
/*N*/ 	: aExpand(rExpand), pFnt(pFnt), nViewWidth(0), nNextOffset(0),
/*N*/ 	  bFollow( sal_False ), bHasFollow( sal_False )
/*N*/ {
/*N*/ 	SetWhichPor( POR_FLD );
/*N*/ }

/*N*/ SwFldPortion::SwFldPortion( const SwFldPortion& rFld )
/*N*/     : aExpand( rFld.GetExp() ),
/*N*/       bCenter( rFld.IsCenter() ),
/*N*/       bFollow( rFld.IsFollow() ),
/*N*/       bHasFollow( rFld.HasFollow() ),
/*N*/       bHide( rFld.IsHide() ),
/*N*/       bLeft( rFld.IsLeft() ),
/*N*/       nNextOffset( rFld.GetNextOffset() )
/*N*/ {
/*N*/     if ( rFld.HasFont() )
/*N*/         pFnt = new SwFont( *rFld.GetFont() );
/*N*/     else
/*N*/         pFnt = 0;
/*N*/ 
/*N*/     SetWhichPor( POR_FLD );
/*N*/ }

/*N*/ SwFldPortion::~SwFldPortion()
/*N*/ {
/*N*/ 	delete pFnt;
/*N*/ }

/*************************************************************************
 *               virtual SwFldPortion::GetViewWidth()
 *************************************************************************/


/*************************************************************************
 *                 virtual SwFldPortion::Format()
 *************************************************************************/

// 8653: in keinem Fall nur SetLen(0);

/*************************************************************************
 *	 Hilfsklasse SwFldSlot
 **************************************************************************/

class SwFldSlot
{
    const XubString *pOldTxt;
    XubString aTxt;
    xub_StrLen nIdx;
    xub_StrLen nLen;
    sal_Bool bOn;
    SwTxtFormatInfo *pInf;
public:
    SwFldSlot( const SwTxtFormatInfo* pNew, const SwFldPortion *pPor );
    ~SwFldSlot();
};

/*N*/ SwFldSlot::SwFldSlot( const SwTxtFormatInfo* pNew, const SwFldPortion *pPor )
/*N*/ {
/*N*/ 	bOn = pPor->GetExpTxt( *pNew, aTxt );
/*N*/ 
/*N*/ 	// Der Text wird ausgetauscht...
/*N*/ 	if( bOn )
/*N*/ 	{
/*N*/         pInf = (SwTxtFormatInfo*)pNew;
/*N*/ 		nIdx = pInf->GetIdx();
/*N*/ 		nLen = pInf->GetLen();
/*N*/ 		pOldTxt = &(pInf->GetTxt());
/*N*/ 		pInf->SetLen( aTxt.Len() );
/*N*/ 		if( pPor->IsFollow() )
/*N*/         {
/*N*/             pInf->SetFakeLineStart( nIdx > pInf->GetLineStart() );
/*N*/             pInf->SetIdx( 0 );
/*N*/         }
/*N*/ 		else
/*N*/ 		{
/*N*/ 			XubString aTmp( aTxt );
/*N*/ 			aTxt = *pOldTxt;
/*N*/ 			aTxt.Erase( nIdx, 1 );
/*N*/ 			aTxt.Insert( aTmp, nIdx );
/*N*/ 		}
/*N*/ 		pInf->SetTxt( aTxt );
/*N*/ 	}
/*N*/ }

/*N*/ SwFldSlot::~SwFldSlot()
/*N*/ {
/*N*/ 	if( bOn )
/*N*/ 	{
/*N*/ 		pInf->SetTxt( *pOldTxt );
/*N*/ 		pInf->SetIdx( nIdx );
/*N*/ 		pInf->SetLen( nLen );
/*N*/         pInf->SetFakeLineStart( sal_False );
/*N*/ 	}
/*N*/ }

/*N*/ BYTE SwFldPortion::ScriptChange( const SwTxtSizeInfo &rInf, xub_StrLen& rFull )
/*N*/ {
/*N*/ 	BYTE nRet = 0;
/*N*/ 	const String& rTxt = rInf.GetTxt();
/*N*/ 	rFull += rInf.GetIdx();
/*N*/ 	if( rFull > rTxt.Len() )
/*?*/ 		rFull = rTxt.Len();
/*N*/ 	if( rFull && pBreakIt->xBreak.is() )
/*N*/ 	{
/*N*/ 		BYTE nActual = pFnt ? pFnt->GetActual() : rInf.GetFont()->GetActual();
/*N*/ 		xub_StrLen nChg = rInf.GetIdx();
/*N*/ 		USHORT nScript;
/*N*/ 		{
/*N*/ 			nScript = i18n::ScriptType::LATIN;
/*N*/ 			if( nActual )
/*N*/ 				nScript = nActual == SW_CJK ? i18n::ScriptType::ASIAN
/*N*/ 											: i18n::ScriptType::COMPLEX;
/*N*/ 			nChg = (xub_StrLen)pBreakIt->xBreak->endOfScript(rTxt,nChg,nScript);
/*N*/ 		}
/*N*/ 		if( rFull > nChg )
/*N*/ 		{
/*?*/ 			nRet = nActual;
/*?*/ 			nScript = pBreakIt->xBreak->getScriptType( rTxt, nChg );
/*?*/ 			if( i18n::ScriptType::ASIAN == nScript )
/*?*/ 				nRet += SW_CJK;
/*?*/ 			else if( i18n::ScriptType::COMPLEX == nScript )
/*?*/ 				nRet += SW_CTL;
/*?*/ 			rFull = nChg;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	rFull -= rInf.GetIdx();
/*N*/ 	return nRet;
/*N*/ }

/*N*/ void SwFldPortion::CheckScript( const SwTxtSizeInfo &rInf )
/*N*/ {
/*N*/ 	String aTxt;
/*N*/ 	if( GetExpTxt( rInf, aTxt ) && aTxt.Len() && pBreakIt->xBreak.is() )
/*N*/ 	{
/*N*/ 		BYTE nActual = pFnt ? pFnt->GetActual() : rInf.GetFont()->GetActual();
/*N*/ 		USHORT nScript;
/*N*/ 		{
/*N*/ 			nScript = pBreakIt->xBreak->getScriptType( aTxt, 0 );
/*N*/ 			xub_StrLen nChg = 0;
/*N*/ 			USHORT nCnt = 0;
/*N*/ 			if( i18n::ScriptType::WEAK == nScript )
/*N*/ 			{
/*N*/ 				nChg =(xub_StrLen)pBreakIt->xBreak->endOfScript(aTxt,0,nScript);
/*N*/ 				if( nChg < aTxt.Len() )
/*?*/ 					nScript = pBreakIt->xBreak->getScriptType( aTxt, nChg );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		BYTE nTmp;
/*N*/ 		switch ( nScript ) {
/*N*/ 			case i18n::ScriptType::LATIN : nTmp = SW_LATIN; break;
/*N*/ 			case i18n::ScriptType::ASIAN : nTmp = SW_CJK; break;
/*N*/ 			case i18n::ScriptType::COMPLEX : nTmp = SW_CTL; break;
/*N*/ 			default: nTmp = nActual;
/*N*/ 		}
/*N*/ 		if( nTmp != nActual )
/*N*/ 		{
/*?*/ 			if( !pFnt )
/*?*/ 				pFnt = new SwFont( *rInf.GetFont() );
/*?*/ 			pFnt->SetActual( nTmp );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*M*/ sal_Bool SwFldPortion::Format( SwTxtFormatInfo &rInf )
/*M*/ {
/*M*/ 	// Scope wegen aDiffTxt::DTOR!
/*M*/ 	xub_StrLen nRest;
/*M*/ 	sal_Bool bFull;
/*M*/ 	sal_Bool bEOL = sal_False;
/*M*/ 	long nTxtRest = rInf.GetTxt().Len() - rInf.GetIdx();
/*M*/ 	{
/*M*/ 		SwFldSlot aDiffTxt( &rInf, this );
/*N*/         SwLayoutModeModifier aLayoutModeModifier( *rInf.GetOut() );
/*N*/         aLayoutModeModifier.SetAuto();
/*N*/ 
/*M*/ 		const xub_StrLen nOldFullLen = rInf.GetLen();
/*M*/ 		const MSHORT nFollow = IsFollow() ? 0 : 1;
/*M*/ 		xub_StrLen nFullLen;
/*M*/ 		// In Numerierungen kennen wir keine Platzhalter, sondern
/*M*/ 		// nur "normale" Zeichen.
/*M*/ 		if( InNumberGrp() )
/*M*/ 			nFullLen = nOldFullLen;
/*M*/ 		else
/*M*/ 		{
/*M*/ 			nFullLen = rInf.ScanPortionEnd( rInf.GetIdx(),
/*N*/                     rInf.GetIdx() + nOldFullLen ) - rInf.GetIdx();
/*M*/ 			if( nFullLen && CH_BREAK == aExpand.GetChar( nFullLen - 1 ) )
/*M*/ 				--nFullLen;
/*M*/ 
/*M*/             if ( STRING_LEN != rInf.GetUnderScorePos() &&
/*M*/                  rInf.GetUnderScorePos() > rInf.GetIdx() )
/*M*/                 rInf.SetUnderScorePos( rInf.GetIdx() );
/*M*/ 		}
/*M*/ 		BYTE nScriptChg = ScriptChange( rInf, nFullLen );
/*M*/ 		rInf.SetLen( nFullLen );
/*M*/ 		if( pFnt )
/*M*/ 			pFnt->GoMagic( rInf.GetVsh(), pFnt->GetActual() );
/*M*/ 
/*M*/ 		SwFontSave aSave( rInf, pFnt );
/*M*/ 
/*M*/ 		// 8674: Laenge muss 0 sein, bei bFull nach Format ist die Laenge
/*M*/ 		// gesetzt und wird in nRest uebertragen. Ansonsten bleibt die
/*M*/ 		// Laenge erhalten und wuerde auch in nRest einfliessen!
/*M*/ 		SetLen(0);
/*M*/ 
/*M*/ 		// So komisch es aussieht, die Abfrage auf GetLen() muss wegen der
/*M*/ 		// ExpandPortions _hinter_ aDiffTxt (vgl. SoftHyphs)
/*M*/ 		// sal_False returnen wegen SetFull ...
/*M*/ 		if( !nFullLen )
/*M*/ 		{
/*M*/ 			// nicht Init(), weil wir Hoehe und Ascent brauchen
/*M*/ 			Width(0);
/*M*/ 			bFull = rInf.Width() <= rInf.GetPos().X();
/*M*/ 		}
/*M*/ 		else
/*M*/ 		{
/*M*/ 			xub_StrLen nOldLineStart = rInf.GetLineStart();
/*M*/ 			if( IsFollow() )
/*M*/ 				rInf.SetLineStart( 0 );
/*M*/ 			rInf.SetNotEOL( nFullLen == nOldFullLen && nTxtRest > nFollow );
/*M*/ 
/*M*/             // the height depending on the fields font is set,
/*M*/             // this is required for SwTxtGuess::Guess
/*M*/             Height( rInf.GetTxtHeight() );
/*M*/             // If a kerning portion is inserted after our field portion,
/*M*/             // the ascent and height must be known
/*M*/             SetAscent( rInf.GetAscent() );
/*M*/             bFull = SwTxtPortion::Format( rInf );
/*M*/ 			rInf.SetNotEOL( sal_False );
/*M*/ 			rInf.SetLineStart( nOldLineStart );
/*M*/ 		}
/*M*/ 		xub_StrLen nTmpLen = GetLen();
/*M*/ 		bEOL = !nTmpLen && nFollow && bFull;
/*M*/ 		nRest = nOldFullLen - nTmpLen;
/*M*/ 
/*M*/         // Das Zeichen wird in der ersten Portion gehalten.
/*M*/ 		// Unbedingt nach Format!
/*M*/ 		SetLen( nFollow );
/*M*/ 
/*M*/         if( nRest )
/*M*/ 		{
/*M*/ 			// aExpand ist noch nicht gekuerzt worden, der neue Ofst
/*M*/ 			// ergibt sich durch nRest.
/*M*/             xub_StrLen nNextOfst = aExpand.Len() - nRest;
/*M*/ 
/*M*/             if ( IsQuoVadisPortion() )
/*?*/                { DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 nNextOfst += ((SwQuoVadisPortion*)this)->GetContTxt().Len();
/*M*/ 
/*M*/ 			XubString aNew( aExpand, nNextOfst, STRING_LEN );
/*M*/ 			aExpand.Erase( nNextOfst, STRING_LEN );
/*M*/ 
/*M*/ 			// Trailingspace et al. !
/*M*/ 			switch( aNew.GetChar( 0 ))
/*M*/ 			{
/*M*/ 				case CH_BREAK  : bFull = sal_True;
/*M*/ 							// kein break;
/*M*/ 				case ' ' :
/*M*/ 				case CH_TAB    :
/*M*/                 case CHAR_HARDHYPHEN:               // non-breaking hyphen
/*N*/                 case CHAR_SOFTHYPHEN:
/*N*/                 case CHAR_HARDBLANK:
/*M*/ 				{
/*M*/ 					aNew.Erase( 0, 1 );
/*M*/ 					++nNextOfst;
/*M*/ 					break;
/*M*/ 				}
/*M*/ 				default: ;
/*M*/ 			}
/*M*/ 
/*M*/ 			if( aNew.Len() || IsQuoVadisPortion() )
/*M*/ 			{
/*M*/ 				// sal_True, weil es ein FollowFeld ist
/*M*/ 				// SwFont *pFont = new SwFont( rInf.GetFont()->GetFnt() );
/*M*/ 				SwFldPortion *pFld = Clone( aNew );
/*M*/ 				if( !pFld->GetFont() )
/*M*/ 				{
/*M*/ 					SwFont *pNewFnt = new SwFont( *rInf.GetFont() );
/*M*/ 					pFld->SetFont( pNewFnt );
/*M*/ 				}
/*M*/ 				pFld->SetFollow( sal_True );
/*M*/ 				SetHasFollow( sal_True );
/*M*/ 				// In nNextOffset steht bei einem neuangelegten Feld zunaechst
/*M*/ 				// der Offset, an dem es selbst im Originalstring beginnt.
/*M*/ 				// Wenn beim Formatieren ein FollowFeld angelegt wird, wird
/*M*/ 				// der Offset dieses FollowFelds in nNextOffset festgehalten.
/*M*/ 				nNextOffset += nNextOfst;
/*M*/ 				pFld->SetNextOffset( nNextOffset );
/*M*/ 				rInf.SetRest( pFld );
/*M*/ 			}
/*M*/ 		}
/*M*/ 	}
/*M*/ 
/*M*/ 	if( bEOL && rInf.GetLast() && !rInf.GetUnderFlow() )
/*M*/ 		rInf.GetLast()->FormatEOL( rInf );
/*M*/ 	return bFull;
/*M*/ }

/*************************************************************************
 *               virtual SwFldPortion::Paint()
 *************************************************************************/


/*************************************************************************
 *              virtual SwFldPortion::GetExpTxt()
 *************************************************************************/

/*N*/ sal_Bool SwFldPortion::GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const
/*N*/ {
/*N*/ 	rTxt = aExpand;
/*N*/     if( !rTxt.Len() && rInf.OnWin() &&
/*N*/         !rInf.GetOpt().IsPagePreview() && !rInf.GetOpt().IsReadonly() &&
/*N*/             SwViewOption::IsFieldShadings() &&
/*N*/             !HasFollow() )
/*N*/ 		rTxt = ' ';
/*N*/ 	return sal_True;
/*N*/ }

/*************************************************************************
 *              virtual SwFldPortion::HandlePortion()
 *************************************************************************/


/*************************************************************************
 *                virtual SwFldPortion::GetTxtSize()
 *************************************************************************/

/*N*/ SwPosSize SwFldPortion::GetTxtSize( const SwTxtSizeInfo &rInf ) const
/*N*/ {
/*N*/ 	SwFontSave aSave( rInf, pFnt );
/*N*/ 	SwPosSize aSize( SwExpandPortion::GetTxtSize( rInf ) );
/*N*/ 	return aSize;
/*N*/ }

/*************************************************************************
 *                      class SwHiddenPortion
 *************************************************************************/

/*************************************************************************
 *               virtual SwHiddenPortion::Paint()
 *************************************************************************/


/*************************************************************************
 *              virtual SwHiddenPortion::GetExpTxt()
 *************************************************************************/

/*N*/ sal_Bool SwHiddenPortion::GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const
/*N*/ {
/*N*/ 	// Nicht auf IsHidden() abfragen !
/*N*/ 	return SwFldPortion::GetExpTxt( rInf, rTxt );
/*N*/ }

/*************************************************************************
 *                      class SwNumberPortion
 *************************************************************************/

/*N*/ SwNumberPortion::SwNumberPortion( const XubString &rExpand, SwFont *pFnt,
/*N*/ 					const sal_Bool bLft, const sal_Bool bCntr, const KSHORT nMinDst )
/*N*/ 		: SwFldPortion( rExpand, pFnt ), nFixWidth(0), nMinDist( nMinDst )
/*N*/ {
/*N*/ 	SetWhichPor( POR_NUMBER );
/*N*/ 	SetLeft( bLft );
/*N*/ 	SetHide( sal_False );
/*N*/ 	SetCenter( bCntr );
/*N*/ }



/*************************************************************************
 *                 virtual SwNumberPortion::Format()
 *************************************************************************/

// 5010: Wir sind in der Lage, mehrzeilige NumFelder anzulegen!
// 3689: Fies ist, wenn man in der Dialogbox soviel Davor-Text
// eingibt, bis die Zeile ueberlaeuft.
// Man muss die Fly-Ausweichmanoever beachten!

/*M*/ sal_Bool SwNumberPortion::Format( SwTxtFormatInfo &rInf )
/*M*/ {
/*M*/ 	SetHide( sal_False );
/*M*/ 	const sal_Bool bFull = SwFldPortion::Format( rInf );
/*M*/ 	SetLen( 0 );
/*M*/     // a numbering portion can be contained in a rotated portion!!!
/*M*/     nFixWidth = rInf.IsMulti() ? Height() : Width();
/*M*/ 	rInf.SetNumDone( !rInf.GetRest() );
/*M*/ 	if( rInf.IsNumDone() )
/*M*/ 	{
/*M*/ //        SetAscent( rInf.GetAscent() );
/*M*/         ASSERT( Height() && nAscent, "NumberPortions without Height | Ascent" )
/*M*/ 
/*M*/ 		long nDiff = rInf.Left() - rInf.First() + rInf.ForcedLeftMargin();
/*M*/ 		// Ein Vorschlag von Juergen und Volkmar:
/*M*/ 		// Der Textteil hinter der Numerierung sollte immer
/*M*/ 		// mindestens beim linken Rand beginnen.
/*M*/ 		if( nDiff < 0 )
/*M*/ 			nDiff = 0;
/*M*/ 		else if ( nDiff > rInf.X() )
/*M*/ 			nDiff -= rInf.X();
/*M*/ 		if( nDiff < nFixWidth + nMinDist )
/*M*/ 			nDiff = nFixWidth + nMinDist;
/*M*/ 		// 2739: Numerierung weicht Fly aus, kein nDiff in der zweiten Runde
/*M*/ 		// fieser Sonderfall: FlyFrm liegt in dem Bereich,
/*M*/ 		// den wir uns gerade unter den Nagel reissen wollen.
/*M*/ 		// Die NumberPortion wird als verborgen markiert.
/*M*/ 		if( nDiff > rInf.Width() )
/*M*/ 		{
/*M*/ 			nDiff = rInf.Width();
/*M*/ 			SetHide( sal_True );
/*M*/ 		}
/*M*/ 
/*M*/         // A numbering portion can be inside a SwRotatedPortion. Then the
/*M*/         // Height has to be changed
/*M*/         if ( rInf.IsMulti() )
/*M*/         {
/*M*/             if ( Height() < nDiff )
/*M*/                 Height( KSHORT( nDiff ) );
/*M*/         }
/*M*/         else if( Width() < nDiff )
/*M*/             Width( KSHORT(nDiff) );
/*M*/ 	}
/*M*/ 	return bFull;
/*M*/ }

/*	Ein FormatEOL deutet daraufhin, dass der folgende Text
 *	nicht mit auf die Zeile passte. Damit die Numerierung mitwandert,
 *  wird diese NumberPortion verborgen.
 */

    // This caused trouble with flys anchored as characters.
    // If one of these is numbered but does not fit to the line,
    // it calls this function, causing a loop because both the number
    // portion and the fly portion go to the next line
//    SetHide( sal_True );

/*************************************************************************
 *               virtual SwNumberPortion::Paint()
 *************************************************************************/



/*************************************************************************
 *                      class SwBulletPortion
 *************************************************************************/

/*N*/ SwBulletPortion::SwBulletPortion( const xub_Unicode cBullet, SwFont *pFont,
/*N*/ 					const sal_Bool bLft, const sal_Bool bCntr, const KSHORT nMinDst )
/*N*/ 	: SwNumberPortion( XubString( cBullet ), pFont, bLft, bCntr, nMinDst )
/*N*/ {
/*N*/ 	SetWhichPor( POR_BULLET );
/*N*/ }

}
