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


// So kann man die Linguistik-Statistik ( (Tmp-Path)\swlingu.stk ) aktivieren:
//#define LINGU_STATISTIK
#ifdef LINGU_STATISTIK
    #include <stdio.h>			// in SwLinguStatistik::DTOR
    #include <stdlib.h> 		// getenv()
    #include <time.h> 			// clock()
#endif

#include <hintids.hxx>

#include <vcl/svapp.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_svx/scripttypeitem.hxx>
#include <com/sun/star/i18n/WordType.hdl>
#include <com/sun/star/i18n/ScriptType.hdl>

#include <acmplwrd.hxx>

#include <horiornt.hxx>

#include <doc.hxx>		// GetDoc()
#include <txatbase.hxx>
#include <hints.hxx>
#include <ndtxt.hxx>
#include <txtfrm.hxx>
#include <wrong.hxx>
#include <breakit.hxx>
#include <drawfont.hxx> // SwDrawTextInfo
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

#define C2U(cChar) ::rtl::OUString::createFromAscii(cChar)

// Wir ersparen uns in Hyphenate ein GetFrm()
// Achtung: in edlingu.cxx stehen die Variablen!
extern const SwTxtNode *pLinguNode;
extern       SwTxtFrm  *pLinguFrm;

bool lcl_IsSkippableWhiteSpace( xub_Unicode cCh )
{
    return 0x3000 == cCh ||
           ' ' == cCh ||
           '\t' == cCh ||
           0x0a == cCh;
}

/*
 * Ein Zeichen wurde eingefuegt.
 */

/*N*/ SwTxtNode& SwTxtNode::Insert( xub_Unicode c, const SwIndex &rIdx )
/*N*/ {
/*N*/ 	xub_StrLen nOrigLen = aText.Len();
/*N*/
/*N*/ 	ASSERT( rIdx <= nOrigLen, "Array ueberindiziert." );
/*N*/ 	ASSERT( nOrigLen < STRING_LEN, "USHRT_MAX ueberschritten." );
/*N*/
/*N*/ 	if( nOrigLen == aText.Insert( c, rIdx.GetIndex() ).Len() )
/*N*/ 		return *this;
/*N*/
/*N*/ 	Update(rIdx,1);
/*N*/
/*N*/ 	// leere Hints und Feldattribute an rIdx.GetIndex suchen
/*N*/ 	if( pSwpHints )
/*N*/ 	{
/*N*/ 		USHORT* pEndIdx;
/*N*/ 		for( USHORT i=0; i < pSwpHints->Count() &&
/*N*/ 				rIdx >= *(*pSwpHints)[i]->GetStart(); ++i)
/*N*/ 		{
/*N*/ 			SwTxtAttr *pHt = pSwpHints->GetHt(i);
/*N*/ 			if( 0 != ( pEndIdx = pHt->GetEnd()) )
/*N*/ 			{
/*N*/ 				// leere Hints an rIdx.GetIndex ?
/*N*/ 				BOOL bEmpty = *pEndIdx == *pHt->GetStart()
/*N*/ 							&& rIdx == *pHt->GetStart();
/*N*/
/*N*/ 				if( bEmpty )
/*N*/ 				{
/*N*/ 					pSwpHints->DeleteAtPos(i);
/*N*/ 					if( bEmpty )
/*N*/ 						*pHt->GetStart() -= 1;
/*N*/ 					else
/*N*/ 						*pEndIdx -= 1;
/*N*/ 					Insert(pHt);
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( pSwpHints->CanBeDeleted() )
/*N*/ 			DELETEZ( pSwpHints );
/*N*/ 	}
/*N*/ 	// den Frames Bescheid sagen
/*N*/ 	SwInsChr aHint( rIdx.GetIndex()-1 );
/*N*/ 	SwModify::Modify( 0, &aHint );
/*N*/ 	return *this;
/*N*/ }


/*
 * void SwTxtNode::RstAttr(const SwIndex &rIdx, USHORT nLen)
 *
 * loescht alle Attribute ab der Position rIdx ueber eine Laenge
 * von nLen.
 */

/* 5 Faelle:
 * 1) Das Attribut liegt vollstaendig im Bereich:
 *    -> loeschen
 * 2) Das Attributende liegt im Bereich:
 *    -> Loeschen, mit neuem Ende einfuegen
 * 3) Der Attributanfang liegt im Bereich:
 *    -> Loeschen, mit neuem Anfang einfuegen
 * 4) Das Attrib umfasst den Bereich:
 *       Aufsplitten, d.h.
 *    -> Loeschen, mit alten Anfang und Anfang des Bereiches einfuegen
 *    -> Neues Attribut mit Ende des Bereiches und altem Ende einfuegen
 * 5) Das Attribut liegt ausserhalb des Bereiches
 *	   -> nichts tun.
 */



/*M*/ void SwTxtNode::RstAttr(const SwIndex &rIdx, xub_StrLen nLen, USHORT nWhich,
/*M*/ 						const SfxItemSet* pSet, BOOL bInclRefToxMark )
/*M*/ {
/*M*/ 	// Attribute?
/*M*/ 	if ( !GetpSwpHints() )
/*M*/ 		return;
/*M*/
/*M*/ 	USHORT i = 0;
/*M*/ 	xub_StrLen nStart = rIdx.GetIndex();
/*M*/ 	xub_StrLen nEnd = nStart + nLen;
/*M*/ 	xub_StrLen *pAttrEnd;
/*M*/ 	xub_StrLen nAttrStart;
/*M*/ 	SwTxtAttr *pHt;
/*M*/
/*M*/ 	BOOL	bChanged = FALSE;
/*M*/
/*M*/ 	// nMin und nMax werden invers auf das Maximum bzw. Minimum gesetzt.
/*M*/ 	xub_StrLen nMin = aText.Len();
/*M*/ 	xub_StrLen nMax = nStart;
/*M*/
/*M*/ 	const BOOL bNoLen = !nMin;
/*M*/
/*M*/     // We have to remember the "new" attributes, which have
/*M*/     // been introduced by splitting surrounding attributes (case 4).
/*M*/     // They may not be forgotten inside the "Forget" function
/*M*/     std::vector< const SwTxtAttr* > aNewAttributes;
/*M*/
/*M*/ 	// durch das Attribute-Array, bis der Anfang des Geltungsbereiches
/*M*/ 	// des Attributs hinter dem Bereich liegt
/*M*/ 	while( (i < pSwpHints->Count()) &&
/*M*/ 				((( nAttrStart = *(*pSwpHints)[i]->GetStart()) < nEnd )
/*M*/ 					|| nLen==0) )
/*M*/ 	{
/*M*/ 		pHt = pSwpHints->GetHt(i);
/*M*/
/*M*/ 		// Attribute ohne Ende bleiben drin!
/*M*/ 		if ( 0 == (pAttrEnd=pHt->GetEnd()) )
/*M*/ 		{
/*M*/ 			i++;
/*M*/ 			continue;
/*M*/ 		}
/*M*/
/*M*/ 		// loesche alle TextAttribute die als Attribut im Set vorhanden sind
/*M*/ 		if( pSet ? SFX_ITEM_SET != pSet->GetItemState( pHt->Which(), FALSE )
/*M*/ 				 : ( nWhich ? nWhich != pHt->Which()
/*M*/ 							: (!bInclRefToxMark &&
/*M*/ 								( RES_TXTATR_REFMARK == pHt->Which() ||
/*M*/ 								RES_TXTATR_TOXMARK == pHt->Which() ))))
/*M*/ 		{
/*M*/ 			// Es sollen nur Attribute mit nWhich beachtet werden
/*M*/ 			i++;
/*M*/ 			continue;
/*M*/ 		}
/*M*/
/*M*/
/*M*/ 		if( nStart <= nAttrStart )          // Faelle: 1,3,5
/*M*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*M*/ 		}
/*M*/ 		else								// Faelle: 2,4,5
/*M*/ 			if( *pAttrEnd > nStart )		// Faelle: 2,4
/*M*/ 			{DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*M*/ 		}
/*M*/ 		++i;
/*M*/ 	}
/*M*/
/*M*/ 	if ( pSwpHints && pSwpHints->CanBeDeleted() )
/*M*/ 		DELETEZ( pSwpHints );
/*M*/ 	if(bChanged)
/*M*/ 	{
/*M*/ 		if ( pSwpHints )
/*M*/ 		{
/*M*/ 			pSwpHints->ClearDummies( *this );
/*M*/ 			((SwpHintsArr*)pSwpHints)->Resort();
/*M*/ 			pSwpHints->Merge( *this );
/*M*/ 		}
/*M*/ 		//TxtFrm's reagieren auf aHint, andere auf aNew
/*M*/ 		SwUpdateAttr aHint( nMin, nMax, 0 );
/*M*/ 		SwModify::Modify( 0, &aHint );
/*M*/ 		SwFmtChg aNew( GetFmtColl() );
/*M*/ 		SwModify::Modify( 0, &aNew );
/*M*/ 	}
/*M*/ }



/*************************************************************************
 *				  SwTxtNode::GetCurWord()
 *
 * Aktuelles Wort zurueckliefern:
 * Wir suchen immer von links nach rechts, es wird also das Wort
 * vor nPos gesucht. Es sei denn, wir befinden uns am Anfang des
 * Absatzes, dann wird das erste Wort zurueckgeliefert.
 * Wenn dieses erste Wort nur aus Whitespaces besteht, returnen wir
 * einen leeren String.
 *************************************************************************/





/*M*/ void SwTxtNode::SetWrong( SwWrongList *pNew )
/*M*/ {
/*M*/ 	delete pWrong;
/*M*/ 	pWrong = pNew;
/*M*/ }

/*M*/ SwScanner::SwScanner( const SwTxtNode& rNd, const SwWrongList* pWrng,
/*M*/                       USHORT nType, xub_StrLen nStart, xub_StrLen nEnde,
/*M*/                       BOOL bRev, BOOL bOS )
/*M*/     : rNode( rNd ), pWrong( pWrng ), nWordType( nType ), nLen( 0 ),
/*M*/       bReverse( bRev ), bStart( TRUE ), bIsOnlineSpell( bOS )
/*M*/ {
/*M*/     ASSERT( rNd.GetTxt().Len(), "SwScanner: EmptyString" );
/*M*/ 	if( bReverse )
/*M*/ 	{
/*M*/ 		nBegin = nEnde;
/*M*/ 		nEndPos = nStart;
/*M*/ 	}
/*M*/ 	else
/*M*/ 	{
/*M*/ 		nBegin = nStart;
/*M*/ 		nEndPos = nEnde;
/*M*/ 	}
/*M*/
/*M*/     aCurrLang = rNd.GetLang( nBegin );
/*M*/ }


/*N*/ BOOL SwScanner::NextWord()
/*N*/ {
/*N*/     ASSERT( ! bReverse,
/*N*/             "SwScanner::NextWord() currently not implemented for reverse mode" )
/*N*/
/*N*/     nBegin += nLen;
/*N*/
/*N*/     // first we have to skip some whitespace characters
/*N*/     const XubString& rText = rNode.GetTxt();
/*N*/     Boundary aBound;
/*N*/
/*N*/     while ( true )
/*N*/     {
/*N*/
/*N*/     while ( nBegin < rText.Len() &&
/*N*/             lcl_IsSkippableWhiteSpace( rText.GetChar( nBegin ) ) )
/*N*/         ++nBegin;
/*N*/
/*N*/     if ( nBegin >= rText.Len() || nBegin >= nEndPos )
/*N*/         return FALSE;
/*N*/
/*N*/     // get next language in order to find next or previous word
/*N*/     const USHORT nNextScript =
/*N*/             pBreakIt->xBreak->getScriptType( rText, nBegin );
/*N*/     if ( nNextScript != GetI18NScriptTypeOfLanguage( aCurrLang ) )
/*N*/     {
/*N*/         LanguageType aNextLang = rNode.GetLang( nBegin, nNextScript );
/*N*/         aCurrLang = aNextLang;
/*N*/     }
/*N*/
/*N*/     // get the word boundaries
/*N*/     aBound = pBreakIt->xBreak->getWordBoundary( rText, nBegin,
/*N*/             pBreakIt->GetLocale( aCurrLang ), nWordType, sal_True );
/*N*/
/*N*/      //no word boundaries could be found
/*N*/      if(aBound.endPos == aBound.startPos)
/*N*/          return FALSE;
/*N*/
/*N*/      if( nBegin == aBound.endPos )
/*N*/          ++nBegin;
/*N*/      else
/*N*/          break;
/*N*/
/*N*/      } // end while( true )

/*N*/
/*N*/     // we have to differenciate between these cases:
/*N*/     if ( aBound.startPos <= nBegin )
/*N*/     {
/*N*/         ASSERT( aBound.endPos >= nBegin, "Unexpected aBound result" )
/*N*/
/*N*/         // restrict boundaries to script boundaries and nEndPos
/*N*/         const USHORT nCurrScript =
/*N*/                 pBreakIt->xBreak->getScriptType( rText, nBegin );
/*N*/
/*N*/         XubString aTmpWord = rText.Copy( nBegin, aBound.endPos - nBegin );
/*N*/         const sal_Int32 nScriptEnd = nBegin +
/*N*/             pBreakIt->xBreak->endOfScript( aTmpWord, 0, nCurrScript );
/*N*/         const sal_Int32 nEnd = Min( aBound.endPos, nScriptEnd );
/*N*/
/*N*/         // restrict word start to last script change position
/*N*/         sal_Int32 nScriptBegin = 0;
/*N*/         if ( aBound.startPos < nBegin )
/*N*/         {
/*N*/             // search from nBegin backwards until the next script change
/*N*/             aTmpWord = rText.Copy( aBound.startPos, nBegin - aBound.startPos + 1 );
/*N*/             nScriptBegin = aBound.startPos +
/*N*/                 pBreakIt->xBreak->beginOfScript( aTmpWord, nBegin - aBound.startPos,
/*N*/                                                 nCurrScript );
/*N*/         }
/*N*/
/*N*/         nBegin = (xub_StrLen)Max( aBound.startPos, nScriptBegin );
/*N*/         nLen = (xub_StrLen)(nEnd - nBegin);
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         const USHORT nCurrScript =
/*N*/                 pBreakIt->xBreak->getScriptType( rText, aBound.startPos );
/*N*/         XubString aTmpWord = rText.Copy( aBound.startPos, aBound.endPos - aBound.startPos );
/*N*/         const sal_Int32 nScriptEnd = aBound.startPos +
/*N*/             pBreakIt->xBreak->endOfScript( aTmpWord, 0, nCurrScript );
/*N*/         const sal_Int32 nEnd = Min( aBound.endPos, nScriptEnd );
/*N*/         nBegin = (xub_StrLen)aBound.startPos;
/*N*/         nLen = (xub_StrLen)(nEnd - nBegin);
/*N*/     }
/*N*/
/*N*/     if( ! nLen )
/*N*/         return FALSE;
/*N*/
/*N*/     aWord = rText.Copy( nBegin, nLen );
/*N*/
/*N*/     return TRUE;
/*N*/ }

#ifdef LINGU_STATISTIK

// globale Variable
SwLinguStatistik aSwLinguStat;



#endif

// change text to Upper/Lower/Hiragana/Katagana/...

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
