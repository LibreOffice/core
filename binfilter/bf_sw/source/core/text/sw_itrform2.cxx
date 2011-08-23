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

#include "hintids.hxx"

#include <com/sun/star/i18n/ScriptType.hdl>

#include <bf_svx/lspcitem.hxx>

#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <ftninfo.hxx>
#include <charfmt.hxx>
#include <bf_svx/charrotateitem.hxx>
#include <paratr.hxx>		// SwFmtDrop
#include <hintids.hxx>		// CH_TXTATR
#include <txtcfg.hxx>
#include <itrform2.hxx>
#include <portab.hxx>		// pLastTab->
#include <porfly.hxx>		// CalcFlyWidth
#include <portox.hxx>		// WhichTxtPortion
#include <porref.hxx>		// WhichTxtPortion

#include <horiornt.hxx>

#include <porftn.hxx>       // SwFtnPortion
#include <porhyph.hxx>
#include <guess.hxx>
#include <ftnfrm.hxx>		// WhichFirstPortion() -> mal Verlagern.

#include <pagefrm.hxx>
#include <pagedesc.hxx> // SwPageDesc
#include <tgrditem.hxx>

#include <doc.hxx>			// SwDoc
#include <pormulti.hxx> 	// SwMultiPortion
#define _SVSTDARR_LONGS
#include <bf_svtools/svstdarr.hxx>
#include <unotools/charclass.hxx>

#if OSL_DEBUG_LEVEL > 1
#endif
namespace binfilter {

using namespace ::com::sun::star::i18n;
extern BYTE WhichFont( xub_StrLen nIdx, const String* pTxt,
                       const SwScriptInfo* pSI );

extern sal_Bool IsUnderlineBreak( const SwLinePortion& rPor, const SwFont& rFnt );

#define MAX_TXTPORLEN 300

/*N*/ inline void ClearFly( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	if( rInf.GetFly() )
/*N*/ 	{
/*N*/ 		delete rInf.GetFly();
/*N*/ 		rInf.SetFly(0);
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *					SwTxtFormatter::CtorInit()
 *************************************************************************/

/*N*/ void SwTxtFormatter::CtorInit( SwTxtFrm *pFrm, SwTxtFormatInfo *pNewInf )
/*N*/ {
/*N*/ 	SwTxtPainter::CtorInit( pFrm, pNewInf );
/*N*/ 	pInf = pNewInf;
/*N*/ 	pDropFmt = GetInfo().GetDropFmt();
/*N*/ 	pMulti = NULL;
/*N*/ 
/*N*/ 	bOnceMore = sal_False;
/*N*/ 	bChanges = sal_False;
/*N*/ 	bTruncLines = sal_False;
/*N*/ 	nCntEndHyph = 0;
/*N*/ 	nCntMidHyph = 0;
/*N*/ 	nLeftScanIdx = STRING_LEN;
/*N*/ 	nRightScanIdx = 0;
/*N*/ 
/*N*/ 	if( nStart > GetInfo().GetTxt().Len() )
/*N*/ 	{
/*?*/ 		ASSERT( !this, "+SwTxtFormatter::CTOR: bad offset" );
/*?*/ 		nStart = GetInfo().GetTxt().Len();
/*N*/ 	}
/*N*/ 
/*N*/ }

/*************************************************************************
 *						SwTxtFormatter::DTOR
 *************************************************************************/

/*N*/ SwTxtFormatter::~SwTxtFormatter()
/*N*/ {
/*N*/ 	// Auesserst unwahrscheinlich aber denkbar.
/*N*/ 	// z.B.: Feld spaltet sich auf, Widows schlagen zu
/*N*/ 	if( GetInfo().GetRest() )
/*N*/ 	{
/*?*/ 		delete GetInfo().GetRest();
/*?*/ 		GetInfo().SetRest(0);
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						SwTxtFormatter::Insert()
 *************************************************************************/

/*N*/ void SwTxtFormatter::Insert( SwLineLayout *pLay )
/*N*/ {
/*N*/ 	// Einfuegen heute mal ausnahmsweise hinter dem aktuellen Element.
/*N*/ 	if ( pCurr )
/*N*/ 	{
/*N*/ 		pLay->SetNext( pCurr->GetNext() );
/*N*/ 		pCurr->SetNext( pLay );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pCurr = pLay;
/*N*/ }

/*************************************************************************
 *					SwTxtFormatter::GetFrmRstHeight()
 *************************************************************************/


/*************************************************************************
 *					SwTxtFormatter::UnderFlow()
 *************************************************************************/

/*N*/ SwLinePortion *SwTxtFormatter::UnderFlow( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	// Werte sichern und rInf initialisieren.
/*N*/ 	SwLinePortion *pUnderFlow = rInf.GetUnderFlow();
/*N*/ 	if( !pUnderFlow )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	// Wir formatieren rueckwaerts, d.h. dass Attributwechsel in der
/*N*/ 	// naechsten Zeile durchaus noch einmal drankommen koennen.
/*N*/ 	// Zu beobachten in 8081.sdw, wenn man in der ersten Zeile Text eingibt.
/*N*/ 
/*N*/ 	const xub_StrLen nSoftHyphPos = rInf.GetSoftHyphPos();
/*N*/     const xub_StrLen nUnderScorePos = rInf.GetUnderScorePos();
/*N*/ 
/*N*/ 	// 8358, 8359: Flys sichern und auf 0 setzen, sonst GPF
/*N*/ 	// 3983: Nicht ClearFly(rInf) !
/*N*/ 	SwFlyPortion *pFly = rInf.GetFly();
/*N*/ 	rInf.SetFly( 0 );
/*N*/ 
/*N*/ 	FeedInf( rInf );
/*N*/ 	rInf.SetLast( pCurr );
/*N*/ 	// pUnderFlow braucht nicht deletet werden, weil es im folgenden
/*N*/ 	// Truncate() untergehen wird.
/*N*/ 	rInf.SetUnderFlow(0);
/*N*/ 	rInf.SetSoftHyphPos( nSoftHyphPos );
/*N*/     rInf.SetUnderScorePos( nUnderScorePos );
/*N*/     rInf.SetPaintOfst( GetLeftMargin() );
/*N*/ 
/*N*/ 	// Wir suchen die Portion mit der Unterlaufposition
/*N*/ 	SwLinePortion *pPor = pCurr->GetFirstPortion();
/*N*/ 	if( pPor != pUnderFlow )
/*N*/ 	{
/*N*/ 		// pPrev wird die letzte Portion vor pUnderFlow,
/*N*/ 		// die noch eine echte Breite hat.
/*N*/ 		// Ausnahme: SoftHyphPortions duerfen dabei natuerlich
/*N*/ 		// nicht vergessen werden, obwohl sie keine Breite haben.
/*N*/ 		SwLinePortion *pPrev = pPor;
/*N*/ 		while( pPor && pPor != pUnderFlow )
/*N*/ 		{
///*N*/ 			DBG_LOOP;
/*N*/ 			if( !pPor->IsKernPortion() &&
/*N*/ 				( pPor->Width() || pPor->IsSoftHyphPortion() ) )
/*N*/ 			{
/*N*/ 				while( pPrev != pPor )
/*N*/ 				{
/*N*/ 					pPrev->Move( rInf );
/*N*/ 					rInf.SetLast( pPrev );
/*N*/ 					pPrev = pPrev->GetPortion();
/*N*/ 					ASSERT( pPrev, "UnderFlow: Loosing control!" );
/*N*/ 				};
/*N*/ 			}
/*N*/ 			pPor = pPor->GetPortion();
/*N*/ 		}
/*N*/ 		pPor = pPrev;
/*N*/ 		if( pPor && // Flies + Initialen werden nicht beim UnderFlow mitgenommen
/*N*/ 			( pPor->IsFlyPortion() || pPor->IsDropPortion() ||
/*N*/ 			  pPor->IsFlyCntPortion() ) )
/*N*/ 		{
/*N*/ 			pPor->Move( rInf );
/*N*/ 			rInf.SetLast( pPor );
/*N*/ 			rInf.SetStopUnderFlow( sal_True );
/*N*/ 			pPor = pUnderFlow;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Was? Die Unterlaufsituation ist nicht in der Portion-Kette ?
/*N*/ 	ASSERT( pPor, "SwTxtFormatter::UnderFlow: overflow but underflow" );
/*N*/ 
/*N*/ 	if( rInf.IsFtnInside() && pPor && !rInf.IsQuick() )
/*N*/ 	{
/*?*/ 		SwLinePortion *pTmp = pPor->GetPortion();
/*?*/ 		while( pTmp )
/*?*/ 		{
/*?*/ 			if( pTmp->IsFtnPortion() )
                    {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 				((SwFtnPortion*)pTmp)->ClearFtn();
/*?*/ 			pTmp = pTmp->GetPortion();
/*?*/ 		}
/*N*/ 	}
/*N*/ 
    /*-----------------14.12.94 09:45-------------------
     * 9849: Schnellschuss
     * --------------------------------------------------*/
/*N*/ 	if ( pPor==rInf.GetLast() )
/*N*/ 	{
/*N*/ 		// Hier landen wir, wenn die UnderFlow-ausloesende Portion sich
/*N*/ 		// ueber die ganze Zeile erstreckt, z. B. wenn ein Wort ueber
/*N*/ 		// mehrere Zeilen geht und in der zweiten Zeile in einen Fly
/*N*/ 		// hineinlaeuft!
/*?*/ 		rInf.SetFly( pFly ); // wg. 28300
/*?*/ 		pPor->Truncate();
/*?*/ 		return pPor; // Reicht das?
/*N*/ 	}
    /*---------------------------------------------------
     * Ende des Schnellschusses wg. 9849
     * --------------------------------------------------*/
/*N*/ 
/*N*/ 	// 4656: X + Width == 0 bei SoftHyph > Zeile ?!
/*N*/ 	if( !pPor || !(rInf.X() + pPor->Width()) )
/*N*/ 	{
/*?*/ 		delete pFly;
/*?*/ 		return 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Vorbereitungen auf's Format()
/*N*/ 	// Wir muessen die Kette hinter pLast abknipsen, weil
/*N*/ 	// nach dem Format() ein Insert erfolgt.
/*N*/ 	SeekAndChg( rInf );
/*N*/ 
/*N*/ 	// line width is adjusted, so that pPor does not fit to current
/*N*/ 	// line anymore
/*N*/   rInf.Width( (USHORT)(rInf.X() + (pPor->Width() ? pPor->Width() - 1 : 0)) );
/*N*/ 	rInf.SetLen( pPor->GetLen() );
/*N*/ 	rInf.SetFull( sal_False );
/*N*/ 	if( pFly )
/*N*/ 	{
/*N*/ 		// Aus folgendem Grund muss die FlyPortion neu berechnet werden:
/*N*/ 		// Wenn durch einen grossen Font in der Mitte der Zeile die Grundlinie
/*N*/ 		// abgesenkt wird und dadurch eine Ueberlappung mit eine Fly entsteht,
/*N*/ 		// so hat die FlyPortion eine falsche Groesse/Fixsize.
/*N*/ 		rInf.SetFly( pFly );
/*N*/ 		CalcFlyWidth( rInf );
/*N*/ 	}
/*N*/ 	rInf.GetLast()->SetPortion(0);
/*N*/ 
/*N*/ 	// Eine Ausnahme bildet das SwLineLayout, dass sich beim
/*N*/ 	// ersten Portionwechsel aufspaltet. Hier nun der umgekehrte Weg:
/*N*/ 	if( rInf.GetLast() == pCurr )
/*N*/ 	{
/*N*/ 		if( pPor->InTxtGrp() && !pPor->InExpGrp() )
/*N*/ 		{
/*N*/ 			MSHORT nOldWhich = pCurr->GetWhichPor();
/*N*/ 			*(SwLinePortion*)pCurr = *pPor;
/*N*/ 			pCurr->SetPortion( pPor->GetPortion() );
/*N*/ 			pCurr->SetWhichPor( nOldWhich );
/*N*/ 			pPor->SetPortion( 0 );
/*N*/ 			delete pPor;
/*N*/ 			pPor = pCurr;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	pPor->Truncate();
/*N*/ 	delete rInf.GetRest();
/*N*/ 	rInf.SetRest(0);
/*N*/ 	return pPor;
/*N*/ }

/*************************************************************************
 *						SwTxtFormatter::InsertPortion()
 *************************************************************************/

/*N*/ void SwTxtFormatter::InsertPortion( SwTxtFormatInfo &rInf,
/*N*/ 									SwLinePortion *pPor ) const
/*N*/ {
/*N*/ 	// Die neue Portion wird eingefuegt,
/*N*/ 	// bei dem LineLayout ist allerdings alles anders...
/*N*/ 	if( pPor == pCurr )
/*N*/ 	{
/*N*/ 		if( pCurr->GetPortion() )
/*N*/ 			pPor = pCurr->GetPortion();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwLinePortion *pLast = rInf.GetLast();
/*N*/ 		if( pLast->GetPortion() )
/*N*/ 		{
/*?*/ 			while( pLast->GetPortion() )
/*?*/ 				pLast = pLast->GetPortion();
/*?*/ 			rInf.SetLast( pLast );
/*N*/ 		}
/*N*/ 		pLast->Insert( pPor );
/*N*/ 
/*N*/ 		// Maxima anpassen:
/*N*/         if( pCurr->Height() < pPor->Height() )
/*N*/             pCurr->Height( pPor->Height() );
/*N*/         if( pCurr->GetAscent() < pPor->GetAscent() )
/*N*/             pCurr->SetAscent( pPor->GetAscent() );
/*N*/ 	}
/*N*/ 
/*N*/ 	// manchmal werden ganze Ketten erzeugt (z.B. durch Hyphenate)
/*N*/ 	rInf.SetLast( pPor );
/*N*/ 	while( pPor )
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 		pPor->Move( rInf );
/*N*/ 		rInf.SetLast( pPor );
/*N*/ 		pPor = pPor->GetPortion();
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						SwTxtFormatter::BuildPortion()
 *************************************************************************/

/*N*/ void SwTxtFormatter::BuildPortions( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	ASSERT( rInf.GetTxt().Len() < STRING_LEN,
/*N*/ 			"SwTxtFormatter::BuildPortions: bad text length in info" );
/*N*/ 
/*N*/ 	rInf.ChkNoHyph( CntEndHyph(), CntMidHyph() );
/*N*/ 
/*N*/ 	// Erst NewTxtPortion() entscheidet, ob pCurr in pPor landet.
/*N*/ 	// Wir muessen in jedem Fall dafuer sorgen, dass der Font eingestellt
/*N*/ 	// wird. In CalcAscent geschieht dies automatisch.
/*N*/     rInf.SetLast( pCurr );
/*N*/ 	rInf.ForcedLeftMargin( 0 );
/*N*/ 
/*N*/     ASSERT( pCurr->FindLastPortion() == pCurr, "pLast supposed to equal pCurr" );
/*N*/ 
/*N*/     if( !pCurr->GetAscent() && !pCurr->Height() )
/*N*/         CalcAscent( rInf, pCurr );
/*N*/ 
/*N*/     SeekAndChg( rInf );
/*N*/ 
/*N*/     // In CalcFlyWidth wird Width() verkuerzt, wenn eine FlyPortion vorliegt.
/*N*/     ASSERT( !rInf.X() || pMulti, "SwTxtFormatter::BuildPortion X=0?" );
/*N*/     CalcFlyWidth( rInf );
/*N*/     SwFlyPortion *pFly = rInf.GetFly();
/*N*/     if( pFly )
/*N*/     {
/*N*/         if ( 0 < pFly->Fix() )
/*N*/             ClearFly( rInf );
/*N*/         else
/*N*/             rInf.SetFull(sal_True);
/*N*/     }
/*N*/ 
/*N*/ 	SwLinePortion *pPor = NewPortion( rInf );
/*N*/ 
    // Asian grid stuff
/*N*/     GETGRID( pFrm->FindPageFrm() )
/*N*/     const sal_Bool bHasGrid = pGrid && rInf.SnapToGrid() &&
/*N*/                               GRID_LINES_CHARS == pGrid->GetGridType();
/*N*/ 
/*N*/     const USHORT nGridWidth = bHasGrid ?
/*N*/                               pGrid->GetBaseHeight() : 0;
/*N*/ 
/*N*/     // used for grid mode only:
/*N*/     // the pointer is stored, because after formatting of non-asian text,
/*N*/     // the width of the kerning portion has to be adjusted
/*N*/     SwKernPortion* pGridKernPortion = 0;
/*N*/ 
/*N*/ 	sal_Bool bFull;
/*N*/     SwTwips nUnderLineStart = 0;
/*N*/ 	rInf.Y( Y() );
/*N*/ 
/*N*/ 	while( pPor && !rInf.IsStop() )
/*N*/ 	{
/*N*/ 		ASSERT( rInf.GetLen() < STRING_LEN &&
/*N*/ 				rInf.GetIdx() <= rInf.GetTxt().Len(),
/*N*/ 				"SwTxtFormatter::BuildPortions: bad length in info" );
///*N*/ 		DBG_LOOP;
/*N*/ 
/*N*/         // We have to check the script for fields in order to set the
/*N*/         // correct nActual value for the font.
/*N*/         if( pPor->InFldGrp() && ! pPor->IsFtnPortion() )
/*N*/             ((SwFldPortion*)pPor)->CheckScript( rInf );
/*N*/ 
/*N*/         if( ! bHasGrid && rInf.HasScriptSpace() &&
/*N*/             rInf.GetLast() && rInf.GetLast()->InTxtGrp() &&
/*N*/             rInf.GetLast()->Width() && !rInf.GetLast()->InNumberGrp() )
/*N*/         {
/*N*/             BYTE nNxtActual = rInf.GetFont()->GetActual();
/*N*/             BYTE nLstActual = nNxtActual;
/*N*/             USHORT nLstHeight = (USHORT)rInf.GetFont()->GetHeight();
/*N*/             sal_Bool bAllowBefore = sal_False;
/*N*/             sal_Bool bAllowBehind = sal_False;
/*N*/             const CharClass& rCC = GetAppCharClass();
/*N*/ 
/*N*/             // are there any punctuation characters on both sides
/*N*/             // of the kerning portion?
/*N*/             if ( pPor->InFldGrp() )
/*N*/             {
/*N*/                 XubString aAltTxt;
/*N*/                 if ( ((SwFldPortion*)pPor)->GetExpTxt( rInf, aAltTxt ) &&
/*N*/                         aAltTxt.Len() )
/*N*/                 {
/*N*/                     bAllowBehind = rCC.isLetterNumeric( aAltTxt, 0 );
/*N*/ 
/*N*/                     const SwFont* pTmpFnt = ((SwFldPortion*)pPor)->GetFont();
/*N*/                     if ( pTmpFnt )
/*N*/                         nNxtActual = pTmpFnt->GetActual();
/*N*/                 }
/*N*/             }
/*N*/             else
/*N*/                 bAllowBehind = rCC.isLetterNumeric( rInf.GetTxt(), rInf.GetIdx() );
/*N*/ 
/*N*/             const SwLinePortion* pLast = rInf.GetLast();
/*N*/             if ( bAllowBehind && pLast )
/*N*/             {
/*N*/                 if ( pLast->InFldGrp() )
/*N*/                 {
/*N*/                     XubString aAltTxt;
/*N*/                     if ( ((SwFldPortion*)pLast)->GetExpTxt( rInf, aAltTxt ) &&
/*N*/                          aAltTxt.Len() )
/*N*/                     {
/*N*/                         bAllowBefore = rCC.isLetterNumeric( aAltTxt, aAltTxt.Len() - 1 );
/*N*/ 
/*N*/                         const SwFont* pTmpFnt = ((SwFldPortion*)pLast)->GetFont();
/*N*/                         if ( pTmpFnt )
/*N*/                         {
/*?*/                             nLstActual = pTmpFnt->GetActual();
/*?*/                             nLstHeight = (USHORT)pTmpFnt->GetHeight();
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/                 else if ( rInf.GetIdx() )
/*N*/                 {
/*N*/                     bAllowBefore = rCC.isLetterNumeric( rInf.GetTxt(), rInf.GetIdx() - 1 );
/*N*/                     // Note: ScriptType returns values in [1,4]
/*N*/                     if ( bAllowBefore )
/*N*/                         nLstActual = pScriptInfo->ScriptType( rInf.GetIdx() - 1 ) - 1;
/*N*/                 }
/*N*/ 
/*N*/                 nLstHeight /= 5;
/*N*/                 // does the kerning portion still fit into the line?
/*N*/                 if( bAllowBefore && ( nLstActual != nNxtActual ) &&
/*N*/                     nLstHeight && rInf.X() + nLstHeight <= rInf.Width() )
/*N*/                 {
/*?*/                     SwKernPortion* pKrn =
/*?*/                         new SwKernPortion( *rInf.GetLast(), nLstHeight,
/*?*/                                            pLast->InFldGrp() && pPor->InFldGrp() );
/*?*/                     rInf.GetLast()->SetPortion( NULL );
/*?*/                     InsertPortion( rInf, pKrn );
/*N*/                 }
/*N*/             }
/*N*/         }
/*N*/         else if ( bHasGrid && ! pGridKernPortion && ! pMulti )
/*N*/         {
/*?*/             // insert a grid kerning portion
/*?*/             if ( ! pGridKernPortion )
                        {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/                 pGridKernPortion = pPor->IsKernPortion() ?
/*?*/ 
/*?*/             // if we have a new GridKernPortion, we initially calculate
/*?*/             // its size so that its ends on the grid
/*?*/             const SwPageFrm* pPageFrm = pFrm->FindPageFrm();
/*?*/             const SwLayoutFrm* pBody = pPageFrm->FindBodyCont();
/*?*/             SWRECTFN( pPageFrm )
/*?*/ 
/*?*/             const long nGridOrigin = pBody ?
/*?*/                                     (pBody->*fnRect->fnGetPrtLeft)() :
/*?*/                                     (pPageFrm->*fnRect->fnGetPrtLeft)();
/*?*/ 
/*?*/             SwTwips nStartX = rInf.X() + GetLeftMargin();
/*?*/             if ( bVert )
/*?*/             {
                    {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/                 Point aPoint( nStartX, 0 );
/*?*/             }
/*?*/ 
/*?*/             const SwTwips nOfst = nStartX - nGridOrigin;
/*?*/             if ( nOfst )
/*?*/             {
/*?*/                 const ULONG i = ( nOfst > 0 ) ?
/*?*/                                  ( ( nOfst - 1 ) / nGridWidth + 1 ) :
/*?*/                                  0;
/*?*/                 const SwTwips nKernWidth = i * nGridWidth - nOfst;
/*?*/                 const SwTwips nRestWidth = rInf.Width() - rInf.X();
/*?*/ 
/*?*/                 if ( nKernWidth <= nRestWidth )
/*?*/                     pGridKernPortion->Width( (USHORT)nKernWidth );
/*?*/             }
/*?*/ 
/*?*/             if ( pGridKernPortion != pPor )
/*?*/                 InsertPortion( rInf, pGridKernPortion );
/*N*/         }
/*N*/ 
/*N*/ 		// the multi-portion has it's own format function
/*N*/         if( pPor->IsMultiPortion() && ( !pMulti || pMulti->IsBidi() ) )
            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 			bFull = BuildMultiPortion( rInf, *((SwMultiPortion*)pPor) );
/*N*/ 		else
/*N*/ 			bFull = pPor->Format( rInf );
/*N*/ 
/*N*/ 		if( rInf.IsRuby() && !rInf.GetRest() )
/*N*/ 			bFull = sal_True;
/*N*/ 
/*N*/         // if we are underlined, we store the beginning of this underlined
/*N*/         // segment for repaint optimization
/*N*/         if ( UNDERLINE_NONE != pFnt->GetUnderline() && ! nUnderLineStart )
/*N*/             nUnderLineStart = GetLeftMargin() + rInf.X();
/*N*/ 
/*N*/         if ( pPor->IsFlyPortion() )
/*N*/             pCurr->SetFly( sal_True );
/*N*/         // some special cases, where we have to take care for the repaint
/*N*/         // offset:
/*N*/         // 1. Underlined portions due to special underline feature
/*N*/         // 2. Right Tab
/*N*/         // 3. BidiPortions
/*N*/         // 4. other Multiportions
/*N*/         // 5. DropCaps
/*N*/         // 6. Grid Mode
/*N*/         else if ( ( ! rInf.GetPaintOfst() || nUnderLineStart < rInf.GetPaintOfst() ) &&
/*N*/                   // 1. Underlined portions
/*N*/                   nUnderLineStart &&
/*N*/                      // reformat is at end of an underlined portion and next portion
/*N*/                      // is not underlined
/*N*/                   ( ( rInf.GetReformatStart() == rInf.GetIdx() &&
/*N*/                       UNDERLINE_NONE == pFnt->GetUnderline()
/*N*/                     ) ||
/*N*/                      // reformat is inside portion and portion is underlined
/*N*/                     ( rInf.GetReformatStart() >= rInf.GetIdx() &&
/*N*/                       rInf.GetReformatStart() <= rInf.GetIdx() + pPor->GetLen() &&
/*N*/                       UNDERLINE_NONE != pFnt->GetUnderline() ) ) )
/*N*/             rInf.SetPaintOfst( nUnderLineStart );
/*N*/         else if (  ! rInf.GetPaintOfst() &&
/*N*/                    // 2. Right Tab
/*N*/                    ( ( pPor->InTabGrp() && !pPor->IsTabLeftPortion() ) ||
/*N*/                    // 3. BidiPortions
/*N*/                      ( pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->IsBidi() ) ||
/*N*/                    // 4. Multi Portion and 5. Drop Caps
/*N*/                      ( ( pPor->IsDropPortion() || pPor->IsMultiPortion() ) &&
/*N*/                        rInf.GetReformatStart() >= rInf.GetIdx() &&
/*N*/                        rInf.GetReformatStart() <= rInf.GetIdx() + pPor->GetLen() )
/*N*/                    // 6. Grid Mode
/*N*/                      || ( bHasGrid && SW_CJK != pFnt->GetActual() )
/*N*/                    )
/*N*/                 )
/*N*/             // we store the beginning of the critical portion as our
/*N*/             // paint offset
/*N*/             rInf.SetPaintOfst( GetLeftMargin() + rInf.X() );
/*N*/ 
/*N*/         // under one of these conditions we are allowed to delete the
/*N*/         // start of the underline portion
/*N*/         if ( IsUnderlineBreak( *pPor, *pFnt ) )
/*N*/             nUnderLineStart = 0;
/*N*/ 
/*N*/         if( pPor->IsFlyCntPortion() || ( pPor->IsMultiPortion() &&
/*N*/ 			((SwMultiPortion*)pPor)->HasFlyInCntnt() ) )
/*N*/ 			SetFlyInCntBase();
/*N*/ 		// 5964: bUnderFlow muss zurueckgesetzt werden, sonst wird beim
/*N*/ 		// 		 naechsten Softhyphen wieder umgebrochen!
/*N*/ 		if ( !bFull )
/*N*/ 		{
/*N*/ 			rInf.ClrUnderFlow();
/*N*/             if( ! bHasGrid && rInf.HasScriptSpace() && pPor->InTxtGrp() &&
/*N*/                 pPor->GetLen() && !pPor->InFldGrp() )
/*N*/ 			{
/*N*/                 // The distance between two different scripts is set
/*N*/                 // to 20% of the fontheight.
/*N*/                 xub_StrLen nTmp = rInf.GetIdx() + pPor->GetLen();
/*N*/                 if( nTmp == pScriptInfo->NextScriptChg( nTmp - 1 ) &&
/*N*/                     nTmp != rInf.GetTxt().Len() )
/*N*/                 {
/*N*/                     USHORT nDist = (USHORT)(rInf.GetFont()->GetHeight()/5);
/*N*/ 
/*N*/                     if( nDist )
/*N*/                     {
/*N*/                         // we do not want a kerning portion if any end
/*N*/                         // would be a punctuation character
/*N*/                         const CharClass& rCC = GetAppCharClass();
/*N*/                         if ( rCC.isLetterNumeric( rInf.GetTxt(), nTmp - 1 ) &&
/*N*/                              rCC.isLetterNumeric( rInf.GetTxt(), nTmp ) )
/*N*/                         {
/*N*/                             // does the kerning portion still fit into the line?
/*?*/                             if ( rInf.X() + pPor->Width() + nDist <= rInf.Width() )
/*?*/                                 new SwKernPortion( *pPor, nDist );
/*?*/                             else
/*?*/                                 bFull = sal_True;
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/         if ( bHasGrid && pPor != pGridKernPortion && ! pMulti )
/*N*/         {
/*?*/             xub_StrLen nTmp = rInf.GetIdx() + pPor->GetLen();
/*?*/             const SwTwips nRestWidth = rInf.Width() - rInf.X() - pPor->Width();
/*?*/ 
/*?*/             const BYTE nCurrScript = pFnt->GetActual(); // pScriptInfo->ScriptType( rInf.GetIdx() );
/*?*/             const BYTE nNextScript = nTmp >= rInf.GetTxt().Len() ?
/*?*/                                      SW_CJK :
/*?*/                                      WhichFont( nTmp, 0, pScriptInfo );
/*?*/ 
/*?*/             // snap non-asian text to grid if next portion is ASIAN or
/*?*/             // there are no more portions in this line
/*?*/             // be careful when handling an underflow event: the gridkernportion
/*?*/             // could have been deleted
/*?*/             if ( nRestWidth > 0 && SW_CJK != nCurrScript &&
/*?*/                 ! rInf.IsUnderFlow() && ( bFull || SW_CJK == nNextScript ) )
/*?*/             {
/*?*/                 ASSERT( pGridKernPortion, "No GridKernPortion available" )
/*?*/ 
/*?*/                 // calculate size
/*?*/                 SwLinePortion* pTmpPor = pGridKernPortion->GetPortion();
/*?*/                 USHORT nSumWidth = pPor->Width();
/*?*/                 while ( pTmpPor )
/*?*/                 {
/*?*/                     nSumWidth += pTmpPor->Width();
/*?*/                     pTmpPor = pTmpPor->GetPortion();
/*?*/                 }
/*?*/ 
/*?*/                 const USHORT i = nSumWidth ?
/*?*/                                  ( nSumWidth - 1 ) / nGridWidth + 1 :
/*?*/                                  0;
/*?*/                 const SwTwips nTmpWidth = i * nGridWidth;
/*?*/                 const SwTwips nKernWidth = Min( (SwTwips)(nTmpWidth - nSumWidth),
/*?*/                                                 nRestWidth );
/*?*/                 const USHORT nKernWidth_1 = (USHORT)(nKernWidth / 2);
/*?*/ 
/*?*/                 ASSERT( nKernWidth <= nRestWidth,
/*?*/                         "Not enough space left for adjusting non-asian text in grid mode" )
/*?*/ 
/*?*/                 pGridKernPortion->Width( pGridKernPortion->Width() + nKernWidth_1 );
/*?*/                 rInf.X( rInf.X() + nKernWidth_1 );
/*?*/ 
/*?*/                 if ( ! bFull )
/*?*/                     new SwKernPortion( *pPor, (short)(nKernWidth - nKernWidth_1),
/*?*/                                        sal_False, sal_True );
/*?*/ 
/*?*/                 pGridKernPortion = 0;
/*?*/             }
/*?*/             else if ( pPor->IsMultiPortion() || pPor->InFixMargGrp() ||
/*?*/                       pPor->IsFlyCntPortion() || pPor->InNumberGrp() ||
/*?*/                       pPor->InFldGrp() || nCurrScript != nNextScript )
/*?*/                 // next portion should snap to grid
/*?*/                 pGridKernPortion = 0;
/*N*/         }
/*N*/ 
/*N*/ 		rInf.SetFull( bFull );
/*N*/ 
/*N*/         // Restportions von mehrzeiligen Feldern haben bisher noch
/*N*/ 		// nicht den richtigen Ascent.
/*N*/ 		if ( !pPor->GetLen() && !pPor->IsFlyPortion()
/*N*/             && !pPor->IsGrfNumPortion() && ! pPor->InNumberGrp()
/*N*/             && !pPor->IsMultiPortion() )
/*N*/ 			CalcAscent( rInf, pPor );
/*N*/ 
/*N*/ 		InsertPortion( rInf, pPor );
/*N*/ 		pPor = NewPortion( rInf );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !rInf.IsStop() )
/*N*/ 	{
/*N*/ 		// der letzte rechte, zentrierte, dezimale Tab
/*N*/ 		SwTabPortion *pLastTab = rInf.GetLastTab();
/*N*/ 		if( pLastTab )
/*N*/ 			pLastTab->FormatEOL( rInf );
/*N*/ 		else if( rInf.GetLast() && rInf.LastKernPortion() )
/*N*/ 			rInf.GetLast()->FormatEOL( rInf );
/*N*/ 	}
/*N*/ 	if( pCurr->GetPortion() && pCurr->GetPortion()->InNumberGrp()
/*N*/ 		&& ((SwNumberPortion*)pCurr->GetPortion())->IsHide() )
/*?*/ 		rInf.SetNumDone( sal_False );
/*N*/ 
/*N*/ 	// 3260, 3860: Fly auf jeden Fall loeschen!
/*N*/ 	ClearFly( rInf );
/*N*/ }

/*************************************************************************
 *				   SwTxtFormatter::CalcAdjustLine()
 *************************************************************************/

/*N*/ void SwTxtFormatter::CalcAdjustLine( SwLineLayout *pCurr )
/*N*/ {
/*N*/     if( SVX_ADJUST_LEFT != GetAdjust() && !pMulti)
/*N*/ 	{
/*N*/ 		pCurr->SetFormatAdj(sal_True);
/*N*/ 		if( IsFlyInCntBase() )
/*N*/ 		{
/*N*/ 			CalcAdjLine( pCurr );
/*N*/ 			// 23348: z.B. bei zentrierten Flys muessen wir den RefPoint
/*N*/ 			// auf jeden Fall umsetzen, deshalb bAllWays = sal_True
/*N*/ 			UpdatePos( pCurr, GetTopLeft(), GetStart(), sal_True );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						SwTxtFormatter::CalcAscent()
 *************************************************************************/

/*M*/ void SwTxtFormatter::CalcAscent( SwTxtFormatInfo &rInf, SwLinePortion *pPor )
/*M*/ {
/*M*/ 	if ( pPor->InFldGrp() && ((SwFldPortion*)pPor)->GetFont() )
/*M*/ 	{
/*M*/ 		// Numerierungen + InterNetFlds koennen einen eigenen Font beinhalten,
/*M*/ 		// dann ist ihre Groesse unabhaengig von harten Attributierungen.
/*M*/ 		SwFont* pFldFnt = ((SwFldPortion*)pPor)->pFnt;
/*M*/ 		SwFontSave aSave( rInf, pFldFnt );
/*M*/ 		((SwFldPortion*)pPor)->Height( pFldFnt->GetHeight( rInf.GetVsh(), rInf.GetOut() ) );
/*M*/ 		((SwFldPortion*)pPor)->SetAscent( pFldFnt->GetAscent( rInf.GetVsh(), rInf.GetOut() ) );
/*M*/ 	}
/*M*/ 	else
/*M*/ 	{
/*M*/ 		const SwLinePortion *pLast = rInf.GetLast();
/*M*/ 		sal_Bool bChg;
/*M*/ 
/*M*/ 		// Fallunterscheidung: in leeren Zeilen werden die Attribute
/*M*/ 		// per SeekStart angeschaltet.
/*M*/ 		const sal_Bool bFirstPor = rInf.GetLineStart() == rInf.GetIdx();
/*M*/ 		if ( pPor->IsQuoVadisPortion() )
/*M*/ 			bChg = SeekStartAndChg( rInf, sal_True );
/*M*/ 		else
/*M*/ 		{
/*M*/ 			if( bFirstPor )
/*M*/ 			{
/*M*/ 				if( rInf.GetTxt().Len() )
/*M*/ 				{
/*M*/ 					if ( pPor->GetLen() || !rInf.GetIdx()
/*M*/ 						 || ( pCurr != pLast && !pLast->IsFlyPortion() )
/*M*/ 						 || !pCurr->IsRest() ) // statt !rInf.GetRest()
/*M*/ 						bChg = SeekAndChg( rInf );
/*M*/ 					else
/*M*/ 						bChg = SeekAndChgBefore( rInf );
/*M*/ 				}
/*M*/                 else if ( pMulti )
/*M*/                     // do not open attributes starting at 0 in empty multi
/*M*/                     // portions (rotated numbering followed by a footnote
/*M*/                     // can cause trouble, because the footnote attribute
/*M*/                     // starts at 0, but if we open it, the attribute handler
/*M*/                     // cannot handle it.
/*M*/                     bChg = sal_False;
/*M*/                 else
/*M*/                     bChg = SeekStartAndChg( rInf );
/*M*/ 			}
/*M*/ 			else
/*M*/ 				bChg = SeekAndChg( rInf );
/*M*/ 		}
/*M*/ 		if( bChg || bFirstPor || !pPor->GetAscent()
/*M*/ 			|| !rInf.GetLast()->InTxtGrp() )
/*M*/ 		{
/*M*/ 			pPor->SetAscent( rInf.GetAscent()  );
/*M*/ 			pPor->Height( rInf.GetTxtHeight() );
/*M*/ 		}
/*M*/ 		else
/*M*/ 		{
/*M*/ 			pPor->Height( pLast->Height() );
/*M*/ 			pPor->SetAscent( pLast->GetAscent() );
/*M*/ 		}
/*M*/ 	}
/*M*/ }

/*************************************************************************
 *						SwTxtFormatter::WhichTxtPor()
 *************************************************************************/

/*N*/ SwTxtPortion *SwTxtFormatter::WhichTxtPor( SwTxtFormatInfo &rInf ) const
/*N*/ {
/*N*/ 	SwTxtPortion *pPor = 0;
/*N*/ 	if( GetFnt()->IsTox() )
/*N*/ 		pPor = new SwToxPortion;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if( GetFnt()->IsRef() )
/*N*/ 			pPor = new SwRefPortion;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// Erst zum Schluss !
/*N*/ 			// Wenn pCurr keine Breite hat, kann sie trotzdem schon Inhalt haben,
/*N*/ 			// z.B. bei nicht darstellbaren Zeichen.
/*N*/ 			if( !rInf.X() && !pCurr->GetPortion() && !pCurr->GetLen() &&
/*N*/ 				!GetFnt()->IsURL() )
/*N*/ 				pPor = pCurr;
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pPor = new SwTxtPortion;
/*N*/ 				if( GetFnt()->IsURL() )
/*?*/ 					pPor->SetWhichPor( POR_URL );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pPor;
/*N*/ }

/*************************************************************************
 *						SwTxtFormatter::NewTxtPortion()
 *************************************************************************/
// Die Laenge wird ermittelt, folgende Portion-Grenzen sind definiert:
// 1) Tabs
// 2) Linebreaks
// 3) CH_TXTATR_BREAKWORD / CH_TXTATR_INWORD
// 4) naechster Attributwechsel

/*N*/ SwTxtPortion *SwTxtFormatter::NewTxtPortion( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	// Wenn wir am Zeilenbeginn stehen, nehmen wir pCurr
/*N*/ 	// Wenn pCurr nicht von SwTxtPortion abgeleitet ist,
/*N*/ 	// muessen wir duplizieren ...
/*N*/ 	Seek( rInf.GetIdx() );
/*N*/ 	SwTxtPortion *pPor = WhichTxtPor( rInf );
/*N*/ 
/*N*/ 	// maximal bis zum naechsten Attributwchsel.
/*N*/ 	xub_StrLen nNextAttr = GetNextAttr();
/*N*/ 	xub_StrLen nNextChg = Min( nNextAttr, rInf.GetTxt().Len() );
/*N*/ 
/*N*/ 	nNextAttr = pScriptInfo->NextScriptChg( rInf.GetIdx() );
/*N*/ 
/*N*/     xub_StrLen nNextDir = pScriptInfo->NextDirChg( rInf.GetIdx() );
/*N*/     nNextAttr = Min( nNextAttr, nNextDir );
/*N*/ 
/*N*/ 	if( nNextChg > nNextAttr )
/*N*/ 		nNextChg = nNextAttr;
/*N*/ 
/*N*/ 	// 7515, 7516, 3470, 6441 : Turbo-Boost
/*N*/ 	// Es wird unterstellt, dass die Buchstaben eines Fonts nicht
/*N*/ 	// groesser als doppelt so breit wie hoch sind.
/*N*/ 	// 7659: Ganz verrueckt: man muss sich auf den Ascent beziehen.
/*N*/ 	// Falle: GetSize() enthaelt die Wunschhoehe, die reale Hoehe
/*N*/ 	// ergibt sich erst im CalcAscent!
/*N*/ 	// 7697: Das Verhaeltnis ist noch krasser: ein Blank im Times
/*N*/ 	// New Roman besitzt einen Ascent von 182, eine Hoehe von 200
/*N*/ 	// und eine Breite von 53! Daraus folgt, dass eine Zeile mit
/*N*/ 	// vielen Blanks falsch eingeschaetzt wird. Wir erhoehen von
/*N*/ 	// Faktor 2 auf 8 (wg. negativen Kernings).
/*N*/ 
/*N*/ 	pPor->SetLen(1);
/*N*/ 	CalcAscent( rInf, pPor );
/*N*/ 
/*N*/ 	const SwFont* pFnt = rInf.GetFont();
/*N*/ 	KSHORT nExpect = Min( KSHORT( ((Font *)pFnt)->GetSize().Height() ),
/*N*/ 						  KSHORT( pPor->GetAscent() ) ) / 8;
/*N*/ 	if ( !nExpect )
/*N*/ 		nExpect = 1;
/*N*/     nExpect = (USHORT)(rInf.GetIdx() + ((rInf.Width() - rInf.X()) / nExpect));
/*N*/ 	if( nExpect > rInf.GetIdx() && nNextChg > nExpect )
/*N*/ 		nNextChg = Min( nExpect, rInf.GetTxt().Len() );
/*N*/ 
/*N*/ 	// we keep an invariant during method calls:
/*N*/ 	// there are no portion ending characters like hard spaces
/*N*/ 	// or tabs in [ nLeftScanIdx, nRightScanIdx ]
/*N*/     if ( nLeftScanIdx <= rInf.GetIdx() && rInf.GetIdx() <= nRightScanIdx )
/*N*/ 	{
/*N*/ 		if ( nNextChg > nRightScanIdx )
/*N*/             nNextChg = nRightScanIdx =
/*N*/                 rInf.ScanPortionEnd( nRightScanIdx, nNextChg );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nLeftScanIdx = rInf.GetIdx();
/*N*/         nNextChg = nRightScanIdx =
/*N*/                 rInf.ScanPortionEnd( rInf.GetIdx(), nNextChg );
/*N*/ 	}
/*N*/ 
/*N*/ 	pPor->SetLen( nNextChg - rInf.GetIdx() );
/*N*/ 	rInf.SetLen( pPor->GetLen() );
/*N*/ 	return pPor;
/*N*/ }


/*************************************************************************
 *				   SwTxtFormatter::WhichFirstPortion()
 *************************************************************************/

/*M*/ SwLinePortion *SwTxtFormatter::WhichFirstPortion(SwTxtFormatInfo &rInf)
/*M*/ {
/*M*/ 	SwLinePortion *pPor = 0;
/*M*/ 
/*M*/ 	if( rInf.GetRest() )
/*M*/ 	{
/*M*/ 		// 5010: Tabs und Felder
/*M*/ 		if( '\0' != rInf.GetHookChar() )
/*M*/ 			return 0;
/*M*/ 
/*M*/ 		pPor = rInf.GetRest();
/*M*/ 		if( pPor->IsErgoSumPortion() )
/*M*/ 			rInf.SetErgoDone(sal_True);
/*M*/ 		else
/*M*/ 			if( pPor->IsFtnNumPortion() )
/*M*/ 				rInf.SetFtnDone(sal_True);
/*M*/ 			else
/*M*/ 				if( pPor->InNumberGrp() )
/*M*/ 					rInf.SetNumDone(sal_True);
/*M*/ 		if( pPor )
/*M*/ 		{
/*M*/ 			rInf.SetRest(0);
/*M*/ 			pCurr->SetRest( sal_True );
/*M*/ 			return pPor;
/*M*/ 		}
/*M*/ 	}
/*M*/ 
/*M*/ 	// ???? und ????: im Follow duerfen wir schon stehen,
/*M*/ 	// entscheidend ist, ob pFrm->GetOfst() == 0 ist!
/*M*/ 	if( rInf.GetIdx() )
/*M*/ 	{
/*M*/ 		// Nun koennen auch FtnPortions und ErgoSumPortions
/*M*/ 		// verlaengert werden.
/*M*/ 
/*M*/ 		// 1) Die ErgoSumTexte
/*M*/ 		if( !rInf.IsErgoDone() )
/*M*/ 		{
/*M*/ 			if( pFrm->IsInFtn() && !pFrm->GetIndPrev() )
/*M*/ 				pPor = (SwLinePortion*)NewErgoSumPortion( rInf );
/*M*/ 			rInf.SetErgoDone( sal_True );
/*M*/ 		}
/*M*/ 		if( !pPor && !rInf.IsArrowDone() )
/*M*/ 		{
/*M*/ 			if( pFrm->GetOfst() && !pFrm->IsFollow() &&
/*M*/ 				rInf.GetIdx() == pFrm->GetOfst() )
/*M*/ 				pPor = new SwArrowPortion( *pCurr );
/*M*/ 			rInf.SetArrowDone( sal_True );
/*M*/ 		}
/*M*/ 
/*M*/         if ( ! pPor && ! pCurr->GetPortion() )
/*M*/         {
/*M*/             GETGRID( GetTxtFrm()->FindPageFrm() )
/*M*/             if ( pGrid )
/*?*/                 {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 pPor = new SwKernPortion( *pCurr );
/*M*/         }
/*M*/ 
/*M*/ 		// 2) Die Zeilenreste (mehrzeilige Felder)
/*M*/ 		if( !pPor )
/*M*/ 		{
/*M*/ 			pPor = rInf.GetRest();
/*M*/ 			// 6922: Nur bei pPor natuerlich.
/*M*/ 			if( pPor )
/*M*/ 			{
/*M*/ 				pCurr->SetRest( sal_True );
/*M*/ 				rInf.SetRest(0);
/*M*/ 			}
/*M*/ 		}
/*M*/ 	}
/*M*/     else
/*M*/ 	{
/*M*/ 		// 1) Die Fussnotenzahlen
/*M*/         if( !rInf.IsFtnDone() )
/*M*/ 		{
/*M*/             ASSERT( ( ! rInf.IsMulti() && ! pMulti ) || pMulti->HasRotation(),
/*M*/                      "Rotated number portion trouble" )
/*M*/ 
/*M*/             sal_Bool bFtnNum = pFrm->IsFtnNumFrm();
/*M*/ 			rInf.GetParaPortion()->SetFtnNum( bFtnNum );
/*M*/ 			if( bFtnNum )
/*M*/ 				pPor = (SwLinePortion*)NewFtnNumPortion( rInf );
/*M*/             rInf.SetFtnDone( sal_True );
/*M*/ 		}
/*M*/ 
/*M*/ 		// 2) Die ErgoSumTexte gibt es natuerlich auch im TextMaster,
/*M*/ 		// entscheidend ist, ob der SwFtnFrm ein Follow ist.
/*M*/         if( !rInf.IsErgoDone() && !pPor && ! rInf.IsMulti() )
/*M*/ 		{
/*M*/ 			if( pFrm->IsInFtn() && !pFrm->GetIndPrev() )
/*M*/ 				pPor = (SwLinePortion*)NewErgoSumPortion( rInf );
/*M*/ 			rInf.SetErgoDone( sal_True );
/*M*/ 		}
/*M*/ 
/*M*/ 		// 3) Die Numerierungen
/*M*/ 		if( !rInf.IsNumDone() && !pPor )
/*M*/ 		{
/*M*/             ASSERT( ( ! rInf.IsMulti() && ! pMulti ) || pMulti->HasRotation(),
/*M*/                      "Rotated number portion trouble" )
/*M*/ 
/*M*/ 			// Wenn wir im Follow stehen, dann natuerlich nicht.
/*M*/ 			if( GetTxtFrm()->GetTxtNode()->GetNum() ||
/*M*/ 				GetTxtFrm()->GetTxtNode()->GetOutlineNum() )
/*M*/                 pPor = (SwLinePortion*)NewNumberPortion( rInf );
/*M*/ 			rInf.SetNumDone( sal_True );
/*M*/ 		}
/*M*/         // 4) Die DropCaps
/*M*/         if( !pPor && GetDropFmt() && ! rInf.IsMulti() )
/*M*/ 			pPor = (SwLinePortion*)NewDropPortion( rInf );
/*M*/ 
/*M*/         if ( ! pPor && ! pCurr->GetPortion() )
/*M*/         {
/*M*/             GETGRID( GetTxtFrm()->FindPageFrm() )
/*M*/             if ( pGrid )
/*?*/                 {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 pPor = new SwKernPortion( *pCurr );
/*M*/         }
/*M*/     }
/*M*/ 	return pPor;
/*M*/ }

/*N*/ sal_Bool lcl_OldFieldRest( const SwLineLayout* pCurr )
/*N*/ {
/*N*/ 	if( !pCurr->GetNext() )
/*N*/ 		return sal_False;
/*N*/ 	const SwLinePortion *pPor = pCurr->GetNext()->GetPortion();
/*N*/ 	sal_Bool bRet = sal_False;
/*N*/ 	while( pPor && !bRet )
/*N*/ 	{
/*N*/ 		bRet = (pPor->InFldGrp() && ((SwFldPortion*)pPor)->IsFollow()) ||
/*N*/ 			(pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->IsFollowFld());
/*N*/ 		if( !pPor->GetLen() )
/*N*/ 			break;
/*N*/ 		pPor = pPor->GetPortion();
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*************************************************************************
 *						SwTxtFormatter::NewPortion()
 *************************************************************************/

/* NewPortion stellt rInf.nLen ein.
 * Eine SwTxtPortion wird begrenzt durch ein tab, break, txtatr,
 * attrwechsel.
 * Drei Faelle koennen eintreten:
 * 1) Die Zeile ist voll und der Umbruch wurde nicht emuliert
 *	  -> return 0;
 * 2) Die Zeile ist voll und es wurde ein Umbruch emuliert
 *	  -> Breite neu einstellen und return new FlyPortion
 * 3) Es muss eine neue Portion gebaut werden.
 *	  -> CalcFlyWidth emuliert ggf. die Breite und return Portion
 */

/*M*/ SwLinePortion *SwTxtFormatter::NewPortion( SwTxtFormatInfo &rInf )
/*M*/ {
/*M*/ 	// Underflow hat Vorrang
/*M*/ 	rInf.SetStopUnderFlow( sal_False );
/*M*/ 	if( rInf.GetUnderFlow() )
/*M*/ 	{
/*M*/ 		ASSERT( rInf.IsFull(), "SwTxtFormatter::NewPortion: underflow but not full" );
/*M*/ 		return UnderFlow( rInf );
/*M*/ 	}
/*M*/ 
/*M*/ 	// Wenn die Zeile voll ist, koennten noch Flys oder
/*M*/ 	// UnderFlow-LinePortions warten ...
/*M*/ 	if( rInf.IsFull() )
/*M*/ 	{
/*M*/ 		// ????: LineBreaks und Flys (bug05.sdw)
/*M*/ 		// 8450: IsDummy()
/*M*/ 		if( rInf.IsNewLine() && (!rInf.GetFly() || !pCurr->IsDummy()) )
/*M*/ 			return 0;
/*M*/ 
/*M*/ 		// Wenn der Text an den Fly gestossen ist, oder wenn
/*M*/ 		// der Fly als erstes drankommt, weil er ueber dem linken
/*M*/ 		// Rand haengt, wird GetFly() returnt.
/*M*/ 		// Wenn IsFull() und kein GetFly() vorhanden ist, gibt's
/*M*/ 		// naturgemaesz eine 0.
/*M*/ 		if( rInf.GetFly() )
/*M*/ 		{
/*M*/             if( rInf.GetLast()->IsBreakPortion() )
/*M*/             {
/*M*/                 delete rInf.GetFly();
/*M*/                 rInf.SetFly( 0 );
/*M*/             }
/*M*/ 
/*M*/             return rInf.GetFly();
/*M*/ 		}
/*M*/ 		// Ein fieser Sonderfall: ein Rahmen ohne Umlauf kreuzt den
/*M*/ 		// Ftn-Bereich. Wir muessen die Ftn-Portion als Zeilenrest
/*M*/ 		// bekanntgeben, damit SwTxtFrm::Format nicht abbricht
/*M*/ 		// (die Textmasse wurde ja durchformatiert).
/*M*/ 		if( rInf.GetRest() )
/*M*/ 			rInf.SetNewLine( sal_True );
/*M*/ 		else
/*M*/ 		{
/*M*/ 			// Wenn die naechste Zeile mit einem Rest eines Feldes beginnt,
/*M*/ 			// jetzt aber kein Rest mehr anliegt,
/*M*/ 			// muss sie auf jeden Fall neu formatiert werden!
/*M*/ 			if( lcl_OldFieldRest( GetCurr() ) )
/*M*/ 				rInf.SetNewLine( sal_True );
/*M*/ 			else
/*M*/ 			{
/*M*/ 				SwLinePortion *pFirst = WhichFirstPortion( rInf );
/*M*/ 				if( pFirst )
/*M*/ 				{
/*M*/ 					rInf.SetNewLine( sal_True );
/*M*/ 					if( pFirst->InNumberGrp() )
/*M*/ 						rInf.SetNumDone( sal_False) ;
/*M*/ 					delete pFirst;
/*M*/ 				}
/*M*/ 			}
/*M*/ 		}
/*M*/ 
/*M*/ 		return 0;
/*M*/ 	}
/*M*/ 
/*M*/ 	SwLinePortion *pPor = WhichFirstPortion( rInf );
/*M*/ 
/*M*/ 	if( !pPor )
/*M*/ 	{
/*M*/         if( !pMulti || pMulti->IsBidi() )
/*M*/ 		{   // We open a multiportion part, if we enter a multi-line part
/*M*/ 			// of the paragraph.
/*M*/ 			xub_StrLen nEnd = rInf.GetIdx();
/*M*/             SwMultiCreator* pCreate = rInf.GetMultiCreator( nEnd, pMulti );
/*M*/ 			if( pCreate )
/*M*/ 			{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*M*/ 			}
/*M*/ 		}
/*M*/ 		// 5010: Tabs und Felder
/*M*/ 		xub_Unicode cChar = rInf.GetHookChar();
/*M*/ 
/*M*/ 		if( cChar )
/*M*/ 		{
             /* Wir holen uns nocheinmal cChar, um sicherzustellen, dass das
              * Tab jetzt wirklich ansteht und nicht auf die naechste Zeile
              * gewandert ist ( so geschehen hinter Rahmen ).
              * Wenn allerdings eine FldPortion im Rest wartet, muessen wir
              * das cChar natuerlich aus dem Feldinhalt holen, z.B. bei
             * DezimalTabs und Feldern (22615)
            */
/*M*/ 			if( !rInf.GetRest() || !rInf.GetRest()->InFldGrp() )
/*M*/ 				cChar = rInf.GetChar( rInf.GetIdx() );
/*M*/ 			rInf.SetHookChar(0);
/*M*/ 		}
/*M*/ 		else
/*M*/ 		{
/*M*/ 			if( rInf.GetIdx() >= rInf.GetTxt().Len() )
/*M*/ 			{
/*M*/ 				rInf.SetFull(sal_True);
/*M*/                 CalcFlyWidth( rInf );
/*M*/ 				return pPor;
/*M*/ 			}
/*M*/ 			cChar = rInf.GetChar( rInf.GetIdx() );
/*M*/ 		}
/*M*/ 
/*M*/ 		switch( cChar )
/*M*/ 		{
/*M*/ 			case CH_TAB    : pPor = NewTabPortion( rInf );	break;
/*M*/ 			case CH_BREAK  : pPor = new SwBreakPortion( *rInf.GetLast() ); break;
/*M*/ 
/*M*/ 			case CHAR_SOFTHYPHEN:					// soft hyphen
/*M*/ 				pPor = new SwSoftHyphPortion; break;
/*M*/ 
/*M*/ 			case CHAR_HARDBLANK:					// no-break space
/*M*/ 				pPor = new SwBlankPortion( ' ' ); break;
/*M*/ 			case CHAR_HARDHYPHEN:			    // non-breaking hyphen
/*M*/ 				pPor = new SwBlankPortion( '-' ); break;
/*M*/ 
/*M*/ 			case CH_TXTATR_BREAKWORD:
/*M*/ 			case CH_TXTATR_INWORD:
/*M*/ 							if( rInf.HasHint( rInf.GetIdx() ) )
/*M*/ 							{
/*M*/ 								pPor = NewExtraPortion( rInf );
/*M*/ 								break;
/*M*/ 							}
/*M*/ 							// No break
/*M*/ 			default 	   :
/*M*/ 			{
/*M*/ 				if( rInf.GetLastTab() && cChar == rInf.GetTabDecimal() )
/*M*/ 					rInf.SetFull( rInf.GetLastTab()->Format( rInf ) );
/*M*/ 
/*M*/ 				if( rInf.GetRest() )
/*M*/ 				{
/*M*/ 					if( rInf.IsFull() )
/*M*/ 					{
/*M*/ 						rInf.SetNewLine(sal_True);
/*M*/ 						return 0;
/*M*/ 					}
/*M*/ 					pPor = rInf.GetRest();
/*M*/ 					rInf.SetRest(0);
/*M*/ 				}
/*M*/ 				else
/*M*/ 				{
/*M*/ 					if( rInf.IsFull() )
/*M*/ 						return 0;
/*M*/ 					pPor = NewTxtPortion( rInf );
/*M*/ 				}
/*M*/ 				break;
/*M*/ 			}
/*M*/ 		}
/*M*/ 
/*M*/ 		// Wenn eine Portion erzeugt wird, obwohl eine RestPortion ansteht,
/*M*/ 		// dann haben wir es mit einem Feld zu tun, das sich aufgesplittet
/*M*/ 		// hat, weil z.B. ein Tab enthalten ist.
/*M*/ 		if( pPor && rInf.GetRest() )
/*M*/ 			pPor->SetLen( 0 );
/*M*/ 
/*M*/ 		// robust:
/*M*/ 		if( !pPor || rInf.IsStop() )
/*M*/ 		{
/*M*/ 			delete pPor;
/*M*/ 			return 0;
/*M*/ 		}
/*M*/ 	}
/*M*/ 
/*M*/     // Special portions containing numbers (footnote anchor, footnote number,
/*M*/     // numbering) can be contained in a rotated portion, if the user
/*M*/     // choose a rotated character attribute.
/*M*/     if ( pPor && ! pMulti )
/*M*/     {
/*M*/         if ( pPor->IsFtnPortion() )
/*M*/         {
/*M*/             const SwTxtFtn* pTxtFtn = ((SwFtnPortion*)pPor)->GetTxtFtn();
/*M*/ 
/*M*/             if ( pTxtFtn )
/*M*/             {
/*M*/                 SwFmtFtn& rFtn = (SwFmtFtn&)pTxtFtn->GetFtn();
/*M*/                 const SwDoc *pDoc = rInf.GetTxtFrm()->GetNode()->GetDoc();
/*M*/                 const SwEndNoteInfo* pInfo;
/*M*/                 if( rFtn.IsEndNote() )
/*M*/                     pInfo = &pDoc->GetEndNoteInfo();
/*M*/                 else
/*M*/                     pInfo = &pDoc->GetFtnInfo();
/*M*/                 const SwAttrSet& rSet = pInfo->GetAnchorCharFmt((SwDoc&)*pDoc)->GetAttrSet();
/*M*/ 
/*M*/                 const SfxPoolItem* pItem;
/*M*/                 USHORT nDir = 0;
/*M*/                 if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_ROTATE,
/*M*/                     sal_True, &pItem ))
/*M*/                     nDir = ((SvxCharRotateItem*)pItem)->GetValue();
/*M*/ 
/*M*/                 if ( 0 != nDir )
/*M*/                 {
/*?*/                    DBG_BF_ASSERT(0, "STRIP"); //STRIP001  delete pPor;
/*M*/                 }
/*M*/             }
/*M*/         }
/*M*/         else if ( pPor->InNumberGrp() )
/*M*/         {
/*M*/             const SwFont* pNumFnt = ((SwFldPortion*)pPor)->GetFont();
/*M*/ 
/*M*/             if ( pNumFnt )
/*M*/             {
/*N*/                 USHORT nDir = pNumFnt->GetOrientation( rInf.GetTxtFrm()->IsVertical() );
/*M*/                 if ( 0 != nDir )
/*M*/                 {
/*?*/                    DBG_BF_ASSERT(0, "STRIP"); //STRIP001  delete pPor;
/*M*/                 }
/*M*/             }
/*M*/         }
/*M*/     }
/*M*/ 
/*M*/     // Der Font wird im Outputdevice eingestellt,
/*M*/ 	// der Ascent und die Hoehe werden berechnet.
/*M*/ 	if( !pPor->GetAscent() && !pPor->Height() )
/*M*/ 		CalcAscent( rInf, pPor );
/*M*/ 	rInf.SetLen( pPor->GetLen() );
/*M*/ 
/*M*/ 	// In CalcFlyWidth wird Width() verkuerzt, wenn eine FlyPortion vorliegt.
/*M*/     CalcFlyWidth( rInf );
/*M*/ 
/*M*/ 	// Man darf nicht vergessen, dass pCurr als GetLast() vernuenftige
/*M*/ 	// Werte bereithalten muss:
/*M*/ 	if( !pCurr->Height() )
/*M*/ 	{
/*M*/ 		ASSERT( pCurr->Height(), "SwTxtFormatter::NewPortion: limbo dance" );
/*M*/ 		pCurr->Height( pPor->Height() );
/*M*/ 		pCurr->SetAscent( pPor->GetAscent() );
/*M*/ 	}
/*M*/ 
/*M*/ 	ASSERT( !pPor || pPor->Height(),
/*M*/ 			"SwTxtFormatter::NewPortion: something went wrong");
/*M*/ 	if( pPor->IsPostItsPortion() && rInf.X() >= rInf.Width() && rInf.GetFly() )
/*M*/ 	{
/*M*/ 		delete pPor;
/*M*/ 		pPor = rInf.GetFly();
/*M*/ 	}
/*M*/ 	return pPor;
/*M*/ }

/*************************************************************************
 *						SwTxtFormatter::FormatLine()
 *************************************************************************/

/*M*/ xub_StrLen SwTxtFormatter::FormatLine( const xub_StrLen nStart )
/*M*/ {
/*M*/     ASSERT( ! pFrm->IsVertical() || pFrm->IsSwapped(),
/*M*/             "SwTxtFormatter::FormatLine( nStart ) with unswapped frame" );
/*N*/ 
/*N*/     // For the formatting routines, we set pOut to the reference device.
/*N*/     SwHookOut aHook( GetInfo() );
/*M*/ 	if( GetInfo().GetLen() < GetInfo().GetTxt().Len() )
/*M*/ 		GetInfo().SetLen( GetInfo().GetTxt().Len() );
/*M*/ 
/*M*/ 	sal_Bool bBuild = sal_True;
/*M*/ 	SetFlyInCntBase( sal_False );
/*M*/ 	GetInfo().SetLineHeight( 0 );
/*M*/     GetInfo().SetLineNettoHeight( 0 );
/*M*/ 
/*M*/ 	// Recycling muss bei geaenderter Zeilenhoehe unterdrueckt werden
/*M*/ 	// und auch bei geaendertem Ascent (Absenken der Grundlinie).
/*M*/ 	const KSHORT nOldHeight = pCurr->Height();
/*M*/ 	const KSHORT nOldAscent = pCurr->GetAscent();
/*M*/ 
/*M*/ 	pCurr->SetEndHyph( sal_False );
/*M*/ 	pCurr->SetMidHyph( sal_False );
/*M*/ 
/*M*/     // fly positioning can make it necessary format a line several times
/*M*/     // for this, we have to keep a copy of our rest portion
/*M*/     SwLinePortion* pFld = GetInfo().GetRest();
/*M*/     SwFldPortion* pSaveFld = 0;
/*M*/ 
/*M*/     if ( pFld && pFld->InFldGrp() && ! pFld->IsFtnPortion() )
/*M*/         pSaveFld = new SwFldPortion( *((SwFldPortion*)pFld) );
/*M*/ 
/*M*/     // for an optimal repaint rectangle, we want to compare fly portions
/*M*/     // before and after the BuildPortions call
/*M*/     const sal_Bool bOptimizeRepaint = AllowRepaintOpt();
/*M*/     const xub_StrLen nOldLineEnd = nStart + pCurr->GetLen();
/*M*/     SvLongs* pFlyStart = 0;
/*M*/ 
/*M*/     // these are the conditions for a fly position comparison
/*M*/     if ( bOptimizeRepaint && pCurr->IsFly() )
/*M*/     {
/*M*/         pFlyStart = new SvLongs;
/*M*/         SwLinePortion* pPor = pCurr->GetFirstPortion();
/*M*/         long nPOfst = 0;
/*M*/         USHORT nCnt = 0;
/*M*/ 
/*M*/         while ( pPor )
/*M*/         {
/*M*/             if ( pPor->IsFlyPortion() )
/*M*/                 // insert start value of fly portion
/*M*/                 pFlyStart->Insert( nPOfst, nCnt++ );
/*M*/ 
/*M*/             nPOfst += pPor->Width();
/*M*/             pPor = pPor->GetPortion();
/*M*/         }
/*M*/     }
/*M*/ 
/*M*/     // Hier folgt bald die Unterlaufpruefung.
/*M*/ 	while( bBuild )
/*M*/ 	{
/*M*/ 		GetInfo().SetFtnInside( sal_False );
/*M*/ 
/*M*/ 		// These values must not be reset by FormatReset();
/*M*/ 		sal_Bool bOldNumDone = GetInfo().IsNumDone();
/*M*/ 		sal_Bool bOldArrowDone = GetInfo().IsArrowDone();
/*M*/ 		sal_Bool bOldErgoDone = GetInfo().IsErgoDone();
/*M*/ 
/*M*/         // besides other things, this sets the repaint offset to 0
/*M*/         FormatReset( GetInfo() );
/*M*/ 
/*M*/ 		GetInfo().SetNumDone( bOldNumDone );
/*M*/ 		GetInfo().SetArrowDone( bOldArrowDone );
/*M*/ 		GetInfo().SetErgoDone( bOldErgoDone );
/*M*/ 
/*M*/         // build new portions for this line
/*M*/         BuildPortions( GetInfo() );
/*M*/ 
/*M*/         if( GetInfo().IsStop() )
/*M*/ 		{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	pCurr->SetLen( 0 );
/*M*/ 		}
/*M*/ 		else if( GetInfo().IsDropInit() )
/*M*/ 		{
/*M*/ 			DropInit();
/*M*/ 			GetInfo().SetDropInit( sal_False );
/*M*/ 		}
/*M*/ 
/*M*/         pCurr->CalcLine( *this, GetInfo() );
/*M*/ 		CalcRealHeight( GetInfo().IsNewLine() );
/*M*/ 
/*M*/ 		if ( IsFlyInCntBase() && !IsQuick() )
/*M*/ 		{
/*M*/ 			KSHORT nTmpAscent, nTmpHeight;
/*M*/ 			CalcAscentAndHeight( nTmpAscent, nTmpHeight );
/*M*/ 			AlignFlyInCntBase( Y() + long( nTmpAscent ) );
/*M*/ 			pCurr->CalcLine( *this, GetInfo() );
/*M*/ 			CalcRealHeight();
/*M*/ 		}
/*M*/ 
/*M*/ 		// bBuild entscheidet, ob noch eine Ehrenrunde gedreht wird
/*M*/         if ( pCurr->GetRealHeight() <= GetInfo().GetLineHeight() )
/*M*/         {
/*M*/             pCurr->SetRealHeight( GetInfo().GetLineHeight() );
/*M*/             bBuild = sal_False;
/*M*/         }
/*M*/         else
/*M*/         {
/*M*/             bBuild = ( GetInfo().GetTxtFly()->IsOn() && ChkFlyUnderflow( GetInfo() )
/*M*/                      || GetInfo().CheckFtnPortion( pCurr ) );
/*M*/             if( bBuild )
/*M*/             {
/*M*/                 GetInfo().SetNumDone( bOldNumDone );
/*M*/                 GetInfo().ResetMaxWidthDiff();
/*M*/ 
/*M*/                 // delete old rest
/*M*/                 if ( GetInfo().GetRest() )
/*M*/                 {
/*M*/                     delete GetInfo().GetRest();
/*M*/                     GetInfo().SetRest( 0 );
/*M*/                 }
/*M*/ 
/*M*/                 // set original rest portion
/*M*/                 if ( pSaveFld )
/*M*/                     GetInfo().SetRest( new SwFldPortion( *pSaveFld ) );
/*M*/ 
/*M*/                 pCurr->SetLen( 0 );
/*M*/                 pCurr->Width(0);
/*M*/                 pCurr->Truncate();
/*M*/             }
/*M*/         }
/*M*/ 	}
/*M*/ 
/*M*/     // calculate optimal repaint rectangle
/*M*/     if ( bOptimizeRepaint )
/*M*/     {
/*M*/         GetInfo().SetPaintOfst( CalcOptRepaint( nOldLineEnd, pFlyStart ) );
/*M*/         if ( pFlyStart )
/*M*/             delete pFlyStart;
/*M*/     }
/*M*/     else
/*M*/         // Special case: We do not allow an optimitation of the repaint
/*M*/         // area, but during formatting the repaint offset is set to indicate
/*M*/         // a maximum value for the offset. This value has to be reset:
/*M*/         GetInfo().SetPaintOfst( 0 );
/*M*/ 
/*M*/     // This corrects the start of the reformat range if something has
/*M*/     // moved to the next line. Otherwise IsFirstReformat in AllowRepaintOpt
/*M*/     // will give us a wrong result if we have to reformat another line
/*M*/     GetInfo().GetParaPortion()->GetReformat()->LeftMove( GetInfo().GetIdx() );
/*M*/ 
/*M*/     // delete master copy of rest portion
/*M*/     if ( pSaveFld )
/*M*/         delete pSaveFld;
/*M*/ 
/*M*/ 	xub_StrLen nNewStart = nStart + pCurr->GetLen();
/*M*/ 
/*M*/     // adjust text if kana compression is enabled
/*M*/     const SwScriptInfo& rSI = GetInfo().GetParaPortion()->GetScriptInfo();
/*M*/ 
/*M*/     if ( GetInfo().CompressLine() )
/*M*/     {
/*?*/      DBG_BF_ASSERT(0, "STRIP"); //STRIP001    USHORT nRepaintOfst = CalcKanaAdj( pCurr );
/*M*/     }
/*M*/ 
/*M*/     CalcAdjustLine( pCurr );
/*M*/ 
/*M*/ 	if( nOldHeight != pCurr->Height() || nOldAscent != pCurr->GetAscent() )
/*M*/ 	{
/*M*/ 		SetFlyInCntBase();
/*M*/ 		GetInfo().SetPaintOfst( 0 ); //geaenderte Zeilenhoehe => kein Recycling
/*M*/ 		// alle weiteren Zeilen muessen gepaintet und, wenn Flys im Spiel sind
/*M*/ 		// auch formatiert werden.
/*M*/ 		GetInfo().SetShift( sal_True );
/*M*/ 	}
/*M*/ 
/*M*/ 	if ( IsFlyInCntBase() && !IsQuick() )
/*M*/ 		UpdatePos( pCurr, GetTopLeft(), GetStart() );
/*M*/ 
/*M*/ 	return nNewStart;
/*M*/ }

/*************************************************************************
 *                      SwTxtFormatter::RecalcRealHeight()
 *************************************************************************/

/*N*/ void SwTxtFormatter::RecalcRealHeight()
/*N*/ {
/*N*/ 	sal_Bool bMore = sal_True;
/*N*/ 	while(bMore)
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 		CalcRealHeight();
/*N*/ 		bMore = Next() != 0;
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *                    SwTxtFormatter::CalcRealHeight()
 *************************************************************************/

/*N*/ void SwTxtFormatter::CalcRealHeight( sal_Bool bNewLine )
/*N*/ {
/*N*/ 	KSHORT nLineHeight = pCurr->Height();
/*N*/ 	pCurr->SetClipping( sal_False );
/*N*/ 
/*N*/     GETGRID( pFrm->FindPageFrm() )
/*N*/     if ( pGrid && GetInfo().SnapToGrid() )
/*N*/     {
/*?*/         const USHORT nGridWidth = pGrid->GetBaseHeight();
/*?*/         const USHORT nRubyHeight = pGrid->GetRubyHeight();
/*?*/         const sal_Bool bRubyTop = ! pGrid->GetRubyTextBelow();
/*?*/ 
/*?*/         USHORT nLineHeight = nGridWidth + nRubyHeight;
/*?*/         USHORT nLineDist = nLineHeight;
/*?*/ 
/*?*/         while ( pCurr->Height() > nLineHeight )
/*?*/             nLineHeight += nLineDist;
/*?*/ 
/*?*/         KSHORT nAsc = pCurr->GetAscent() +
/*?*/                       ( bRubyTop ?
/*?*/                        ( nLineHeight - pCurr->Height() + nRubyHeight ) / 2 :
/*?*/                        ( nLineHeight - pCurr->Height() - nRubyHeight ) / 2 );
/*?*/ 
/*?*/         pCurr->Height( nLineHeight );
/*?*/         pCurr->SetAscent( nAsc );
/*?*/         pInf->GetParaPortion()->SetFixLineHeight();
/*?*/ 
/*?*/         // we ignore any line spacing options except from ...
/*?*/         const SvxLineSpacingItem* pSpace = aLineInf.GetLineSpacing();
/*?*/         if ( ! IsParaLine() && pSpace &&
/*?*/              SVX_INTER_LINE_SPACE_PROP == pSpace->GetInterLineSpaceRule() )
/*?*/         {
/*?*/             ULONG nTmp = pSpace->GetPropLineSpace();
/*?*/ 
/*?*/             if( nTmp < 100 )
/*?*/                 nTmp = 100;
/*?*/ 
/*?*/             nTmp *= nLineHeight;
/*?*/             nLineHeight = (USHORT)(nTmp / 100);
/*?*/         }
/*?*/ 
/*?*/         pCurr->SetRealHeight( nLineHeight );
/*?*/         return;
/*N*/     }
/*N*/ 
/*N*/ 	// Das Dummyflag besitzen Zeilen, die nur Flyportions enthalten, diese
/*N*/ 	// sollten kein Register etc. beachten. Dummerweise hat kann es eine leere
/*N*/ 	// Zeile am Absatzende geben (bei leeren Abs„tzen oder nach einem
/*N*/ 	// Shift-Return), die das Register durchaus beachten soll.
/*N*/     if( !pCurr->IsDummy() || ( !pCurr->GetNext() &&
/*N*/         GetStart() >= GetTxtFrm()->GetTxt().Len() && !bNewLine ) )
/*N*/     {
/*N*/         const SvxLineSpacingItem *pSpace = aLineInf.GetLineSpacing();
/*N*/         if( pSpace )
/*N*/         {
/*N*/             switch( pSpace->GetLineSpaceRule() )
/*N*/             {
/*N*/                 case SVX_LINE_SPACE_AUTO:
/*N*/                 break;
/*N*/                 case SVX_LINE_SPACE_MIN:
/*N*/                 {
/*N*/                     if( nLineHeight < KSHORT( pSpace->GetLineHeight() ) )
/*N*/                         nLineHeight = pSpace->GetLineHeight();
/*N*/                     break;
/*N*/                 }
/*N*/                 case SVX_LINE_SPACE_FIX:
/*N*/                 {
/*?*/                     nLineHeight = pSpace->GetLineHeight();
/*?*/                     KSHORT nAsc = ( 4 * nLineHeight ) / 5;  // 80%
/*?*/                     if( nAsc < pCurr->GetAscent() ||
/*?*/                         nLineHeight - nAsc < pCurr->Height() - pCurr->GetAscent() )
/*?*/                         pCurr->SetClipping( sal_True );
/*?*/                     pCurr->Height( nLineHeight );
/*?*/                     pCurr->SetAscent( nAsc );
/*?*/                     pInf->GetParaPortion()->SetFixLineHeight();
/*?*/                 }
/*?*/                 break;
/*?*/                 default: ASSERT( sal_False, ": unknown LineSpaceRule" );
/*N*/             }
/*N*/             if( !IsParaLine() )
/*N*/                 switch( pSpace->GetInterLineSpaceRule() )
/*N*/                 {
/*N*/                     case SVX_INTER_LINE_SPACE_OFF:
/*N*/                     break;
/*N*/                     case SVX_INTER_LINE_SPACE_PROP:
/*N*/                     {
/*N*/                         long nTmp = pSpace->GetPropLineSpace();
/*N*/                         // 50% ist das Minimum, bei 0% schalten wir auf
/*N*/                         // den Defaultwert 100% um ...
/*N*/                         if( nTmp < 50 )
/*N*/                             nTmp = nTmp ? 50 : 100;
/*N*/ 
/*N*/                         nTmp *= nLineHeight;
/*N*/                         nTmp /= 100;
/*N*/                         if( !nTmp )
/*N*/                             ++nTmp;
/*N*/                         nLineHeight = (KSHORT)nTmp;
/*N*/                         break;
/*N*/                     }
/*N*/                     case SVX_INTER_LINE_SPACE_FIX:
/*N*/                     {
/*?*/                         nLineHeight += pSpace->GetInterLineSpace();
/*?*/                         break;
/*N*/                     }
/*N*/                     default: ASSERT( sal_False, ": unknown InterLineSpaceRule" );
/*N*/                 }
/*N*/         }
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/         KSHORT nDummy = nLineHeight + 1;
/*N*/ #endif
/*N*/ 
/*N*/         if( IsRegisterOn() )
/*N*/         {
/*N*/             SwTwips nTmpY = Y() + pCurr->GetAscent() + nLineHeight - pCurr->Height();
/*N*/             SWRECTFN( pFrm )
/*N*/             if ( bVert )
                    {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/                 nTmpY = pFrm->SwitchHorizontalToVertical( nTmpY );
/*N*/             nTmpY = (*fnRect->fnYDiff)( nTmpY, RegStart() );
/*N*/             KSHORT nDiff = KSHORT( nTmpY % RegDiff() );
/*N*/             if( nDiff )
/*N*/                 nLineHeight += RegDiff() - nDiff;
/*N*/         }
/*N*/     }
/*N*/ 	pCurr->SetRealHeight( nLineHeight );
/*N*/ }

/*************************************************************************
 *						SwTxtFormatter::FeedInf()
 *************************************************************************/

/*N*/ void SwTxtFormatter::FeedInf( SwTxtFormatInfo &rInf ) const
/*N*/ {
/*N*/ 	// 3260, 3860: Fly auf jeden Fall loeschen!
/*N*/ 	ClearFly( rInf );
/*N*/ 	rInf.Init();
/*N*/ 
/*N*/ 	rInf.ChkNoHyph( CntEndHyph(), CntMidHyph() );
/*N*/ 	rInf.SetRoot( pCurr );
/*N*/ 	rInf.SetLineStart( nStart );
/*N*/ 	rInf.SetIdx( nStart );
/*N*/ 	rInf.Left( KSHORT(Left()) );
/*N*/ 	rInf.Right( KSHORT(Right()) );
/*N*/ 	rInf.First( short(FirstLeft()) );
/*N*/ 	rInf.RealWidth( KSHORT(rInf.Right()) - KSHORT(GetLeftMargin()) );
/*N*/ 	rInf.Width( rInf.RealWidth() );
/*N*/ 	if( ((SwTxtFormatter*)this)->GetRedln() )
/*N*/ 	{
            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 		((SwTxtFormatter*)this)->GetRedln()->Clear( ((SwTxtFormatter*)this)->GetFnt() );
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						SwTxtFormatter::FormatReset()
 *************************************************************************/

/*N*/ void SwTxtFormatter::FormatReset( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	pCurr->Truncate();
/*N*/ 	pCurr->Init();
/*N*/ 
/*N*/     // delete pSpaceAdd und pKanaComp
/*N*/     pCurr->FinishSpaceAdd();
/*N*/     pCurr->FinishKanaComp();
/*N*/ 	pCurr->ResetFlags();
/*N*/ 	FeedInf( rInf );
/*N*/ }

/*************************************************************************
 *				  SwTxtFormatter::CalcOnceMore()
 *************************************************************************/

/*N*/ sal_Bool SwTxtFormatter::CalcOnceMore()
/*N*/ {
/*N*/ 	if( pDropFmt )
/*N*/ 	{
/*N*/ 		const KSHORT nOldDrop = GetDropHeight();
/*N*/ 		CalcDropHeight( pDropFmt->GetLines() );
/*N*/ 		bOnceMore = nOldDrop != GetDropHeight();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		bOnceMore = sal_False;
/*N*/ 	return bOnceMore;
/*N*/ }

/*************************************************************************
 *				  SwTxtFormatter::CalcBottomLine()
 *************************************************************************/

/*N*/ SwTwips SwTxtFormatter::CalcBottomLine() const
/*N*/ {
/*N*/ 	SwTwips nRet = Y() + GetLineHeight();
/*N*/ 	SwTwips nMin = GetInfo().GetTxtFly()->GetMinBottom();
/*N*/ 	if( nMin && ++nMin > nRet )
/*N*/ 	{
/*N*/ 		SwTwips nDist = pFrm->Frm().Height() - pFrm->Prt().Height()
/*N*/ 						- pFrm->Prt().Top();
/*N*/ 		if( nRet + nDist < nMin )
/*N*/ 		{
/*N*/ 			sal_Bool bRepaint = HasTruncLines() &&
/*N*/ 				GetInfo().GetParaPortion()->GetRepaint()->Bottom() == nRet-1;
/*N*/ 			nRet = nMin - nDist;
/*N*/ 			if( bRepaint )
/*N*/ 			{
/*?*/ 				((SwRepaint*)GetInfo().GetParaPortion()
/*?*/ 					->GetRepaint())->Bottom( nRet-1 );
/*?*/ 				((SwTxtFormatInfo&)GetInfo()).SetPaintOfst( 0 );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/*************************************************************************
 *				  SwTxtFormatter::_CalcFitToContent()
 *
 * FME: This routine does a limited text formatting under the assumption,
 * that the line length is USHORT twips. I'm not sure why we do not do
 * a full text formatting using "FormatLine" or "BuildPortion" here,
 * similar to SwTxtFormatter::Hyphenate(). If we compare this function
 * to BuildPortions(), it looks like they used to be very similar
 * (back in 1995), but BuildPortions() changed and _CalcFitToContent()
 * did not. So _CalcFitToContent() does not give you exact results,
 * although the results should be good enought for most situations.
 *************************************************************************/


/*************************************************************************
 *                      SwTxtFormatter::AllowRepaintOpt()
 *
 * determines if the calculation of a repaint offset is allowed
 * otherwise each line is painted from 0 (this is a copy of the beginning
 * of the former SwTxtFormatter::Recycle() function
 *************************************************************************/
/*N*/ sal_Bool SwTxtFormatter::AllowRepaintOpt() const
/*N*/ {
/*N*/     // reformat position in front of current line? Only in this case
/*N*/     // we want to set the repaint offset
/*N*/     sal_Bool bOptimizeRepaint = nStart < GetInfo().GetReformatStart() &&
/*N*/                                 pCurr->GetLen();
/*N*/ 
/*N*/     // a special case is the last line of a block adjusted paragraph:
/*N*/     if ( bOptimizeRepaint )
/*N*/     {
/*N*/         switch( GetAdjust() )
/*N*/         {
/*N*/         case SVX_ADJUST_BLOCK:
/*N*/         {
/*N*/             if( IsLastBlock() || IsLastCenter() )
/*?*/                 bOptimizeRepaint = sal_False;
/*N*/             else
/*N*/             {
/*N*/                 // ????: Blank in der letzten Masterzeile (blocksat.sdw)
/*N*/                 bOptimizeRepaint = 0 == pCurr->GetNext() && !pFrm->GetFollow();
/*N*/                 if ( bOptimizeRepaint )
/*N*/                 {
/*N*/                     SwLinePortion *pPos = pCurr->GetFirstPortion();
/*N*/                     while ( pPos && !pPos->IsFlyPortion() )
/*N*/                         pPos = pPos->GetPortion();
/*N*/                     bOptimizeRepaint = !pPos;
/*N*/                 }
/*N*/             }
/*N*/             break;
/*N*/         }
/*N*/         case SVX_ADJUST_CENTER:
/*N*/         case SVX_ADJUST_RIGHT:
/*N*/             bOptimizeRepaint = sal_False;
/*N*/             break;
/*N*/         default: ;
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/ 	// Schon wieder ein Sonderfall: unsichtbare SoftHyphs
/*N*/     const xub_StrLen nReformat = GetInfo().GetReformatStart();
/*N*/     if( bOptimizeRepaint && STRING_LEN != nReformat )
/*N*/ 	{
/*N*/         const xub_Unicode cCh = GetInfo().GetTxt().GetChar( nReformat );
/*N*/         bOptimizeRepaint = ( CH_TXTATR_BREAKWORD != cCh && CH_TXTATR_INWORD != cCh )
/*N*/                             || ! GetInfo().HasHint( nReformat );
/*N*/ 	}
/*N*/ 
/*N*/     return bOptimizeRepaint;
/*N*/ }

/*************************************************************************
 *                      SwTxtFormatter::CalcOptRepaint()
 *
 * calculates an optimal repaint offset for the current line
 *************************************************************************/
/*N*/ long SwTxtFormatter::CalcOptRepaint( xub_StrLen nOldLineEnd,
/*N*/                                      const SvLongs* pFlyStart )
/*N*/ {
/*N*/     if ( GetInfo().GetIdx() < GetInfo().GetReformatStart() )
/*N*/     // the reformat position is behind our new line, that means
/*N*/     // something of our text has moved to the next line
/*N*/         return 0;
/*N*/ 
/*N*/     xub_StrLen nReformat = Min( GetInfo().GetReformatStart(), nOldLineEnd );
/*N*/ 
/*N*/     // in case we do not have any fly in our line, our repaint position
/*N*/     // is the changed position - 1
/*N*/     if ( ! pFlyStart && ! pCurr->IsFly() )
/*N*/     {
/*N*/         // this is the maximum repaint offset determined during formatting
/*N*/         // for example: the beginning of the first right tab stop
/*N*/         // if this value is 0, this means that we do not have an upper
/*N*/         // limit for the repaint offset
/*N*/         const long nFormatRepaint = GetInfo().GetPaintOfst();
/*N*/ 
/*N*/         if ( nReformat < GetInfo().GetLineStart() + 3 )
/*N*/             return 0;
/*N*/ 
/*N*/         // step back for smoother repaint
/*N*/         --nReformat;
/*N*/ 
/*N*/         // step back two more characters for complex scripts
/*N*/         const SwScriptInfo& rSI = GetInfo().GetParaPortion()->GetScriptInfo();
/*N*/         if ( ScriptType::COMPLEX == rSI.ScriptType( nReformat ) )
/*N*/             nReformat -= 2;
/*N*/ 
/*N*/         // Weird situation: Our line used to end with a hole portion
/*N*/         // and we delete some characters at the end of our line. We have
/*N*/         // to take care for repainting the blanks which are not anymore
/*N*/         // covered by the hole portion
/*N*/         while ( nReformat > GetInfo().GetLineStart() &&
/*N*/                 CH_BLANK == GetInfo().GetChar( nReformat ) )
/*N*/             --nReformat;
/*N*/ 
/*N*/         ASSERT( nReformat < GetInfo().GetIdx(), "Reformat too small for me!" );
/*N*/         SwRect aRect;
/*N*/ 
/*N*/         // Note: GetChareRect is not const. It definitely changes the
/*N*/         // bMulti flag. We have to save and resore the old value.
/*N*/         sal_Bool bOldMulti = GetInfo().IsMulti();
/*N*/         GetCharRect( &aRect, nReformat );
/*N*/         GetInfo().SetMulti( bOldMulti );
/*N*/ 
/*N*/         return nFormatRepaint ? Min( aRect.Left(), nFormatRepaint ) :
/*N*/                                 aRect.Left();
/*N*/     }
/*N*/     else
/*N*/     {
/*?*/         // nReformat may be wrong, if something around flys has changed:
/*?*/         // we compare the former and the new fly positions in this line
/*?*/         // if anything has changed, we carefully have to adjust the right
/*?*/         // repaint position
/*?*/         long nPOfst = 0;
/*?*/         USHORT nCnt = 0;
/*?*/         USHORT nX = 0;
/*?*/         USHORT nIdx = GetInfo().GetLineStart();
/*?*/         SwLinePortion* pPor = pCurr->GetFirstPortion();
/*?*/ 
/*?*/         while ( pPor )
/*?*/         {
/*?*/             if ( pPor->IsFlyPortion() )
/*?*/             {
/*?*/                 // compare start of fly with former start of fly
/*?*/                 if ( pFlyStart &&
/*?*/                      nCnt < pFlyStart->Count() &&
/*?*/                      nX == (*pFlyStart)[ nCnt ] &&
/*?*/                      nIdx < nReformat
/*?*/                    )
/*?*/                     // found fix position, nothing has changed left from nX
/*?*/                     nPOfst = nX + pPor->Width();
/*?*/                 else
/*?*/                     break;
/*?*/ 
/*?*/                 nCnt++;
/*?*/             }
/*?*/             nX += pPor->Width();
/*?*/             nIdx += pPor->GetLen();
/*?*/             pPor = pPor->GetPortion();
/*?*/         }
/*?*/ 
/*?*/         return nPOfst + GetLeftMargin();
/*N*/     }
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
