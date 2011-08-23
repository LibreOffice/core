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


#include <com/sun/star/i18n/ScriptType.hdl>


#include <errhdl.hxx>

#include <itratr.hxx>		// SwAttrIter
#include <ndtxt.hxx>		// SwTxtNode

#include <horiornt.hxx>

#include <doc.hxx>			// SwDoc
#include <frmsh.hxx>
#include <breakit.hxx>

//////////////////////////



#include <txtfrm.hxx>		// SwTxtFrm
#include <redlnitr.hxx>
#include <extinput.hxx>

namespace binfilter {

using namespace ::com::sun::star;

extern BYTE WhichFont( xub_StrLen nIdx, const String* pTxt,
                       const SwScriptInfo* pSI );

/*************************************************************************
 *						SwAttrIter::CtorInit()
 *************************************************************************/
/*N*/ void SwAttrIter::CtorInit( SwTxtNode& rTxtNode, SwScriptInfo& rScrInf, SwTxtFrm* pFrm )
/*N*/ {
/*N*/ 	// Beim HTML-Import kann es vorkommen, dass kein Layout existiert.
/*N*/ 	SwRootFrm *pRootFrm = rTxtNode.GetDoc()->GetRootFrm();
/*N*/ 	pShell = pRootFrm ? pRootFrm->GetShell() : 0;
/*N*/ 
/*N*/     pScriptInfo = &rScrInf;
/*N*/ 	pAttrSet = &rTxtNode.GetSwAttrSet();
/*N*/     pHints = rTxtNode.GetpSwpHints();
/*N*/ 
/*N*/     SwFontAccess aFontAccess( &rTxtNode.GetAnyFmtColl(), pShell );
/*N*/ 
/*N*/     delete pFnt;
/*N*/     pFnt = new SwFont( *aFontAccess.Get()->GetFont() );
/*N*/ 
/*N*/     // set font to vertical if frame layout is vertical
/*N*/     sal_Bool bVertLayout = sal_False;
/*N*/     sal_Bool bRTL = sal_False;
/*N*/     if ( pFrm )
/*N*/     {
/*N*/         if ( pFrm->IsVertical() )
/*N*/         {
/*N*/             bVertLayout = sal_True;
/*N*/             pFnt->SetVertical( pFnt->GetOrientation(), sal_True );
/*N*/         }
/*N*/         bRTL = pFrm->IsRightToLeft();
/*N*/     }
/*N*/ 
/*N*/     // Initialize the default attribute of the attribute handler
/*N*/     // based on the attribute array cached together with the font.
/*N*/     // If any further attributes for the paragraph are given in pAttrSet
/*N*/     // consider them during construction of the default array, and apply
/*N*/     // them to the font
/*N*/     aAttrHandler.Init( aFontAccess.Get()->GetDefault(),
/*N*/                        rTxtNode.HasSwAttrSet() ? pAttrSet : 0,
/*N*/                        *rTxtNode.GetDoc(), pShell, *pFnt, bVertLayout );
/*N*/ 
/*N*/ 	aMagicNo[SW_LATIN] = aMagicNo[SW_CJK] = aMagicNo[SW_CTL] = NULL;
/*N*/ 
/*N*/ 	// determine script changes if not already done for current paragraph
/*N*/ 	ASSERT( pScriptInfo, "No script info available");
/*N*/     if ( pScriptInfo->GetInvalidity() != STRING_LEN )
/*N*/          pScriptInfo->InitScriptInfo( rTxtNode, bRTL );
/*N*/ 
/*N*/ 	if ( pBreakIt->xBreak.is() )
/*N*/ 	{
/*N*/         pFnt->SetActual( WhichFont( 0, 0, pScriptInfo ) );
/*N*/ 
/*N*/         xub_StrLen nChg = 0;
/*N*/ 		USHORT nCnt = 0;
/*N*/ 
/*N*/         do
/*N*/ 		{
/*N*/ 			nChg = pScriptInfo->GetScriptChg( nCnt );
/*N*/             USHORT nScript = pScriptInfo->GetScriptType( nCnt++ );
/*N*/ 			BYTE nTmp = 4;
/*N*/ 			switch ( nScript ) {
/*N*/ 				case i18n::ScriptType::ASIAN :
/*N*/ 					if( !aMagicNo[SW_CJK] ) nTmp = SW_CJK; break;
/*N*/ 				case i18n::ScriptType::COMPLEX :
/*N*/ 					if( !aMagicNo[SW_CTL] ) nTmp = SW_CTL; break;
/*N*/ 				default:
/*N*/ 					if( !aMagicNo[SW_LATIN ] ) nTmp = SW_LATIN;
/*N*/ 			}
/*N*/ 			if( nTmp < 4 )
/*N*/ 			{
/*N*/ 				pFnt->ChkMagic( pShell, nTmp );
/*N*/ 				pFnt->GetMagic( aMagicNo[ nTmp ], aFntIdx[ nTmp ], nTmp );
/*N*/ 			}
/*N*/ 		} while( nChg < rTxtNode.GetTxt().Len() );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pFnt->ChkMagic( pShell, SW_LATIN );
/*N*/ 		pFnt->GetMagic( aMagicNo[ SW_LATIN ], aFntIdx[ SW_LATIN ], SW_LATIN );
/*N*/ 	}
/*N*/ 
/*N*/     nStartIndex = nEndIndex = nPos = nChgCnt = 0;
/*N*/ 	nPropFont = 0;
/*N*/ 	SwDoc* pDoc = rTxtNode.GetDoc();
/*N*/ 
/*N*/ 	const SwExtTextInput* pExtInp = pDoc->GetExtTextInput( rTxtNode );
/*N*/ 	sal_Bool bShow = ::binfilter::IsShowChanges( pDoc->GetRedlineMode() );
/*N*/     if( pExtInp || bShow )
/*N*/ 	{
/*N*/ 		MSHORT nRedlPos = pDoc->GetRedlinePos( rTxtNode );
/*N*/ 		if( pExtInp || MSHRT_MAX != nRedlPos )
/*N*/ 		{
/*N*/ 			const SvUShorts* pArr = 0;
/*N*/ 			xub_StrLen nInputStt = 0;
/*N*/ 			if( pExtInp )
/*N*/ 			{
/*N*/ 				pArr = &pExtInp->GetAttrs();
/*N*/ 				nInputStt = pExtInp->Start()->nContent.GetIndex();
/*N*/                 Seek( 0 );
/*N*/ 			}
/*N*/ 
/*N*/             pRedln = new SwRedlineItr( rTxtNode, *pFnt, aAttrHandler, nRedlPos,
/*N*/                                         bShow, pArr, nInputStt );
/*N*/ 
/*N*/ 			if( pRedln->IsOn() )
/*N*/ 				++nChgCnt;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
 * SwRedlineItr - Der Redline-Iterator
 *
 * Folgende Informationen/Zustaende gibt es im RedlineIterator:
 *
 * nFirst ist der erste Index der RedlineTbl, der mit dem Absatz ueberlappt.
 *
 * nAct ist der zur Zeit aktive ( wenn bOn gesetzt ist ) oder der naechste
 * in Frage kommende Index.
 * nStart und nEnd geben die Grenzen des Objekts innerhalb des Absatzes an.
 *
 * Wenn bOn gesetzt ist, ist der Font entsprechend manipuliert worden.
 *
 * Wenn nAct auf MSHRT_MAX gesetzt wurde ( durch Reset() ), so ist zur Zeit
 * kein Redline aktiv, nStart und nEnd sind invalid.
 *************************************************************************/



// Der Return-Wert von SwRedlineItr::Seek gibt an, ob der aktuelle Font
// veraendert wurde durch Verlassen (-1) oder Betreten eines Bereichs (+1)











}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
