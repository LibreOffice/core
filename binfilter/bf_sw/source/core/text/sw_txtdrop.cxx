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


#include <vcl/metric.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>

#include <paratr.hxx>
#include <charfmt.hxx>
#include <viewsh.hxx>	// ViewShell
#include <pordrop.hxx>
#include <itrform2.hxx>
#include <breakit.hxx>
#include <com/sun/star/i18n/ScriptType.hdl>
#include <com/sun/star/i18n/WordType.hpp>
#include <bf_svx/langitem.hxx>
#include <charatr.hxx>
namespace binfilter {

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star;

/*************************************************************************
 *                SwDropPortionPart DTor
 *************************************************************************/

/*N*/ SwDropPortionPart::~SwDropPortionPart()
/*N*/ {
/*N*/     if ( pFollow )
/*N*/         delete pFollow;
/*N*/     delete pFnt;
/*N*/ }

/*************************************************************************
 *                SwDropPortion CTor, DTor
 *************************************************************************/

/*N*/ SwDropPortion::SwDropPortion( const MSHORT nLineCnt,
/*N*/                               const KSHORT nDropHeight,
/*N*/                               const KSHORT nDropDescent,
/*N*/                               const KSHORT nDistance )
/*N*/   : pPart( 0 ),
/*N*/     nLines( nLineCnt ),
/*N*/ 	nDropHeight(nDropHeight),
/*N*/ 	nDropDescent(nDropDescent),
/*N*/ 	nDistance(nDistance),
/*N*/ 	nX(0),
/*N*/ 	nFix(0)
/*N*/ {
/*N*/     SetWhichPor( POR_DROP );
/*N*/ }

/*N*/ SwDropPortion::~SwDropPortion()
/*N*/ {
/*N*/     delete pPart;
/*N*/ }

/*N*/ sal_Bool SwTxtSizeInfo::_HasHint( const SwTxtNode* pTxtNode, xub_StrLen nPos )
/*N*/ {
/*N*/ 	const SwpHints *pHints = pTxtNode->GetpSwpHints();
/*N*/ 	if( !pHints )
/*N*/ 		return sal_False;
/*N*/ 	for( MSHORT i = 0; i < pHints->Count(); ++i )
/*N*/ 	{
/*N*/ 		const SwTxtAttr *pPos = (*pHints)[i];
/*N*/ 		xub_StrLen nStart = *pPos->GetStart();
/*N*/ 		if( nPos < nStart )
/*N*/ 			return sal_False;
/*N*/ 		if( nPos == nStart && !pPos->GetEnd() )
/*N*/ 			return sal_True;
/*N*/ 	}
/*N*/ 	return sal_False;
/*N*/ }

/*************************************************************************
 *					  SwTxtNode::GetDropLen()
 *
 * nWishLen = 0 indicates that we want a whole word
 *************************************************************************/

/*N*/ MSHORT SwTxtNode::GetDropLen( MSHORT nWishLen ) const
/*N*/ {
/*N*/     xub_StrLen nEnd = GetTxt().Len();
/*N*/     if( nWishLen && nWishLen < nEnd )
/*N*/         nEnd = nWishLen;
/*N*/ 
/*N*/     if ( ! nWishLen && pBreakIt->xBreak.is() )
/*N*/     {
/*N*/         // find first word
/*?*/         const SwAttrSet& rAttrSet = GetSwAttrSet();
/*?*/         const USHORT nTxtScript = pBreakIt->GetRealScriptOfText( GetTxt(), 0 );
/*?*/ 
/*?*/         LanguageType eLanguage;
/*?*/ 
/*?*/         switch ( nTxtScript )
/*?*/         {
/*?*/         case i18n::ScriptType::ASIAN :
/*?*/             eLanguage = rAttrSet.GetCJKLanguage().GetLanguage();
/*?*/             break;
/*?*/         case i18n::ScriptType::COMPLEX :
/*?*/             eLanguage = rAttrSet.GetCTLLanguage().GetLanguage();
/*?*/             break;
/*?*/         default :
/*?*/             eLanguage = rAttrSet.GetLanguage().GetLanguage();
/*?*/             break;
/*?*/         }
/*?*/ 
/*?*/         Boundary aBound =
/*?*/             pBreakIt->xBreak->getWordBoundary( GetTxt(), 0,
/*?*/             pBreakIt->GetLocale( eLanguage ), WordType::DICTIONARY_WORD, sal_True );
/*?*/ 
/*?*/         nEnd = (xub_StrLen)aBound.endPos;
/*N*/     }
/*N*/ 
/*N*/     xub_StrLen i = 0;
/*N*/     for( ; i < nEnd; ++i )
/*N*/     {
/*N*/         xub_Unicode cChar = GetTxt().GetChar( i );
/*N*/         if( CH_TAB == cChar || CH_BREAK == cChar ||
/*N*/             (( CH_TXTATR_BREAKWORD == cChar || CH_TXTATR_INWORD == cChar )
/*N*/                 && SwTxtSizeInfo::_HasHint( this, i ) ) )
/*N*/             break;
/*N*/     }
/*N*/     return i;
/*N*/ }

/*************************************************************************
 *                SwTxtFormatter::CalcDropHeight()
 *************************************************************************/

/*N*/ void SwTxtFormatter::CalcDropHeight( const MSHORT nLines )
/*N*/  {
/*N*/  	const SwLinePortion *const pOldCurr = GetCurr();
/*N*/  	KSHORT nDropHght = 0;
/*N*/  	KSHORT nAscent = 0;
/*N*/  	KSHORT nHeight = 0;
/*N*/  	KSHORT nDropLns = 0;
/*N*/  	sal_Bool bRegisterOld = IsRegisterOn();
/*N*/  	bRegisterOn = sal_False;
/*N*/  
/*N*/  	Top();
/*N*/  
/*N*/  	while( GetCurr()->IsDummy() )
/*N*/  	{
/*N*/  		if ( !Next() )
/*N*/  			break;
/*N*/  	}
/*N*/  
/*N*/  	// Wenn wir nur eine Zeile haben returnen wir 0
/*N*/  	if( GetNext() || GetDropLines() == 1 )
/*N*/  	{
/*N*/  		for( ; nDropLns < nLines; nDropLns++ )
/*N*/  		{
/*N*/  			if ( GetCurr()->IsDummy() )
/*N*/  				break;
/*N*/  			else
/*N*/  			{
/*N*/  				CalcAscentAndHeight( nAscent, nHeight );
/*N*/  				nDropHght += nHeight;
/*N*/  				bRegisterOn = bRegisterOld;
/*N*/  			}
/*N*/  			if ( !Next() )
/*N*/  			{
/*N*/  				nDropLns++; // Fix: 11356
/*N*/  				break;
/*N*/  			}
/*N*/  		}
/*N*/  
/*N*/  		// In der letzten Zeile plumpsen wir auf den Zeilenascent!
/*N*/  		nDropHght -= nHeight;
/*N*/  		nDropHght += nAscent;
/*N*/  		Top();
/*N*/  	}
/*N*/  	bRegisterOn = bRegisterOld;
/*N*/  	SetDropDescent( nHeight - nAscent );
/*N*/  	SetDropHeight( nDropHght );
/*N*/  	SetDropLines( nDropLns );
/*N*/  	// Alte Stelle wiederfinden!
/*N*/  	while( pOldCurr != GetCurr() )
/*N*/  	{
/*N*/  		if( !Next() )
/*N*/  		{
/*N*/  			ASSERT( !this, "SwTxtFormatter::_CalcDropHeight: left Toulouse" );
/*N*/  			break;
/*N*/  		}
/*N*/  	}
/*N*/  }

/*************************************************************************
 *                SwTxtFormatter::GuessDropHeight()
 *
 *  Wir schaetzen mal, dass die Fonthoehe sich nicht aendert und dass
 *  erst mindestens soviele Zeilen gibt, wie die DropCap-Einstellung angibt.
 *
 *************************************************************************/



/*N*/ void SwTxtFormatter::GuessDropHeight( const MSHORT nLines )
/*N*/ {
/*N*/ 	ASSERT( nLines, "GuessDropHeight: Give me more Lines!" );
/*N*/ 	KSHORT nAscent = 0;
/*N*/ 	KSHORT nHeight = 0;
/*N*/ 	SetDropLines( nLines );
/*N*/ 	if ( GetDropLines() > 1 )
/*N*/ 	{
/*N*/ 		CalcRealHeight();
/*N*/ 		CalcAscentAndHeight( nAscent, nHeight );
/*N*/ 	}
/*N*/ 	SetDropDescent( nHeight - nAscent );
/*N*/ 	SetDropHeight( nHeight * nLines - GetDropDescent() );
/*N*/ }

/*************************************************************************
 *                SwTxtFormatter::NewDropPortion
 *************************************************************************/

/*N*/ SwDropPortion *SwTxtFormatter::NewDropPortion( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	if( !pDropFmt )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	xub_StrLen nPorLen = pDropFmt->GetWholeWord() ? 0 : pDropFmt->GetChars();
/*N*/     nPorLen = pFrm->GetTxtNode()->GetDropLen( nPorLen );
/*N*/ 	if( !nPorLen )
/*N*/ 	{
/*?*/ 		((SwTxtFormatter*)this)->ClearDropFmt();
/*?*/ 		return 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwDropPortion *pDropPor = 0;
/*N*/ 
/*N*/ 	// erste oder zweite Runde?
/*N*/ 	if ( !( GetDropHeight() || IsOnceMore() ) )
/*N*/ 	{
/*N*/ 		if ( GetNext() )
/*?*/             CalcDropHeight( pDropFmt->GetLines() );
/*N*/ 		else
/*N*/             GuessDropHeight( pDropFmt->GetLines() );
/*N*/ 	}
/*N*/ 
/*N*/     // the DropPortion
/*N*/     if( GetDropHeight() )
/*?*/         pDropPor = new SwDropPortion( GetDropLines(), GetDropHeight(),
/*N*/                                       GetDropDescent(), pDropFmt->GetDistance() );
/*N*/ 	else
/*N*/         pDropPor = new SwDropPortion( 0,0,0,pDropFmt->GetDistance() );
/*N*/ 
/*N*/     pDropPor->SetLen( nPorLen );
/*N*/ 
/*N*/     // If it was not possible to create a proper drop cap portion
/*N*/     // due to avoiding endless loops. We return a drop cap portion
/*N*/     // with an empty SwDropCapPart. For these portions the current
/*N*/     // font is used.
/*N*/     if ( GetDropLines() < 2 )
/*N*/     {
/*?*/         ((SwTxtFormatter*)this)->SetPaintDrop( sal_True );
/*?*/         return pDropPor;
/*N*/     }
/*N*/ 
/*N*/     // build DropPortionParts:
/*N*/     ASSERT( ! rInf.GetIdx(), "Drop Portion not at 0 position!" );
/*N*/     xub_StrLen nIdx = rInf.GetIdx();
/*N*/     xub_StrLen nNextChg = 0;
/*N*/     const SwCharFmt* pFmt = pDropFmt->GetCharFmt();
/*N*/     SwDropPortionPart* pCurrPart = 0;
/*N*/ 
/*N*/     while ( nNextChg  < nPorLen )
/*N*/     {
/*N*/         // check for attribute changes and if the portion has to split:
/*N*/         Seek( nNextChg );
/*N*/ 
/*N*/         // the font is deleted in the destructor of the drop portion part
/*N*/         SwFont* pTmpFnt = new SwFont( *rInf.GetFont() );
/*N*/         if ( pFmt )
/*N*/         {
/*N*/             const SwAttrSet& rSet = pFmt->GetAttrSet();
/*N*/             pTmpFnt->SetDiffFnt( &rSet, rInf.GetDoc() );
/*N*/         }
/*N*/ 
/*N*/         // we do not allow a vertical font for the drop portion
/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/         pTmpFnt->SetVertical( 0, rInf.GetTxtFrm()->IsVertical() );
/*N*/ #else
/*N*/         pTmpFnt->SetVertical( 0 );
/*N*/ #endif
/*N*/ 
/*N*/         // find next attribute change / script change
/*N*/         const xub_StrLen nIdx = nNextChg;
/*N*/         xub_StrLen nNextAttr = Min( GetNextAttr(), rInf.GetTxt().Len() );
/*N*/         nNextChg = pScriptInfo->NextScriptChg( nIdx );
/*N*/         if( nNextChg > nNextAttr )
/*N*/             nNextChg = nNextAttr;
/*N*/         if ( nNextChg > nPorLen )
/*N*/             nNextChg = nPorLen;
/*N*/ 
/*N*/         SwDropPortionPart* pPart =
/*N*/                 new SwDropPortionPart( *pTmpFnt, nNextChg - nIdx );
/*N*/ 
/*N*/         if ( ! pCurrPart )
/*N*/             pDropPor->SetPart( pPart );
/*N*/         else
/*?*/             pCurrPart->SetFollow( pPart );
/*N*/ 
/*N*/         pCurrPart = pPart;
/*N*/     }
/*N*/ 
/*N*/ 	((SwTxtFormatter*)this)->SetPaintDrop( sal_True );
/*N*/ 	return pDropPor;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
