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

#include "viewsh.hxx"       // IsDbg()
#include "viewopt.hxx"      // IsDbg()

#ifndef DBG_UTIL
#error Wer fummelt denn an den makefiles?
#endif

#define CONSTCHAR( name, string ) static const sal_Char __FAR_DATA name[] = string
//#include "txtcfg.hxx"


#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include "flyfrms.hxx"
#include "inftxt.hxx"
#include "porfly.hxx"
#include "porftn.hxx"
#include "porhyph.hxx"
#include "porref.hxx"
#include "porrst.hxx"
#include "portab.hxx"
#include "portox.hxx"
#include "pordrop.hxx"
#include "pormulti.hxx"
#include "frmsh.hxx"

// So kann man die Layoutstruktur ausgeben lassen
// #define AMA_LAYOUT
#ifdef AMA_LAYOUT
#include <stdio.h>
#include <stdlib.h> 		// getenv()
namespace binfilter {


/*N*/ void lcl_OutFollow( XubString &rTmp, const SwFrm* pFrm )
/*N*/ {
/*N*/ 	if( pFrm->IsFlowFrm() )
/*N*/ 	{
/*N*/ 		const SwFlowFrm *pFlow = SwFlowFrm::CastFlowFrm( pFrm );
/*N*/ 		if( pFlow->IsFollow() || pFlow->GetFollow() )
/*N*/ 		{
/*N*/ 			rTmp += "(";
/*N*/ 			if( pFlow->IsFollow() )
/*N*/ 				rTmp += ".";
/*N*/ 			if( pFlow->GetFollow() )
/*N*/ 			{
/*N*/ 				MSHORT nFrmId = pFlow->GetFollow()->GetFrm()->GetFrmId();
/*N*/ 				rTmp += nFrmId;
/*N*/ 			}
/*N*/ 			rTmp += ")";
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void lcl_OutFrame( SvFileStream& rStr, const SwFrm* pFrm, ByteString& rSp, sal_Bool bNxt )
/*N*/ {
/*N*/ 	if( !pFrm )
/*N*/ 		return;
/*N*/ 	KSHORT nSpc = 0;
/*N*/ 	MSHORT nFrmId = pFrm->GetFrmId();
/*N*/ 	ByteString aTmp;
/*N*/ 	if( pFrm->IsLayoutFrm() )
/*N*/ 	{
/*N*/ 		if( pFrm->IsRootFrm() )
/*N*/ 			aTmp = "R";
/*N*/ 		else if( pFrm->IsPageFrm() )
/*N*/ 			aTmp = "P";
/*N*/ 		else if( pFrm->IsBodyFrm() )
/*N*/ 			aTmp = "B";
/*N*/ 		else if( pFrm->IsColumnFrm() )
/*N*/ 			aTmp = "C";
/*N*/ 		else if( pFrm->IsTabFrm() )
/*N*/ 			aTmp = "Tb";
/*N*/ 		else if( pFrm->IsRowFrm() )
/*N*/ 			aTmp = "Rw";
/*N*/ 		else if( pFrm->IsCellFrm() )
/*N*/ 			aTmp = "Ce";
/*N*/ 		else if( pFrm->IsSctFrm() )
/*N*/ 			aTmp = "S";
/*N*/ 		else if( pFrm->IsFlyFrm() )
/*N*/ 		{
/*N*/ 			aTmp = "F";
/*N*/ 			const SwFlyFrm *pFly = (SwFlyFrm*)pFrm;
/*N*/ 			if( pFly->IsFlyInCntFrm() )
/*N*/ 				aTmp += "in";
/*N*/ 			else if( pFly->IsFlyAtCntFrm() )
/*N*/ 			{
/*N*/ 				aTmp += "a";
/*N*/ 				if( pFly->IsAutoPos() )
/*N*/ 					aTmp += "u";
/*N*/ 				else
/*N*/ 					aTmp += "t";
/*N*/ 			}
/*N*/ 			else
/*N*/ 				aTmp += "l";
/*N*/ 		}
/*N*/ 		else if( pFrm->IsHeaderFrm() )
/*N*/ 			aTmp = "H";
/*N*/ 		else if( pFrm->IsFooterFrm() )
/*N*/ 			aTmp = "Fz";
/*N*/ 		else if( pFrm->IsFtnContFrm() )
/*N*/ 			aTmp = "Fc";
/*N*/ 		else if( pFrm->IsFtnFrm() )
/*N*/ 			aTmp = "Fn";
/*N*/ 		else
/*N*/ 			aTmp = "?L?";
/*N*/ 		aTmp += nFrmId;
/*N*/ 		lcl_OutFollow( aTmp, pFrm );
/*N*/ 		aTmp += " ";
/*N*/ 		rStr << aTmp;
/*N*/ 		nSpc = aTmp.Len();
/*N*/ 		rSp.Expand( nSpc + rSp.Len() );
/*N*/ 		lcl_OutFrame( rStr, ((SwLayoutFrm*)pFrm)->Lower(), rSp, sal_True );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if( pFrm->IsTxtFrm() )
/*N*/ 			aTmp = "T";
/*N*/ 		else if( pFrm->IsNoTxtFrm() )
/*N*/ 			aTmp = "N";
/*N*/ 		else
/*N*/ 			aTmp = "?C?";
/*N*/ 		aTmp += nFrmId;
/*N*/ 		lcl_OutFollow( aTmp, pFrm );
/*N*/ 		aTmp += " ";
/*N*/ 		rStr << aTmp;
/*N*/ 		nSpc = aTmp.Len();
/*N*/ 		rSp.Expand( nSpc + rSp.Len() );
/*N*/ 	}
/*N*/ 	if( pFrm->IsPageFrm() )
/*N*/ 	{
/*N*/ 		const SwPageFrm* pPg = (SwPageFrm*)pFrm;
/*N*/ 		const SwSortDrawObjs *pSorted = pPg->GetSortedObjs();
/*N*/ 		const MSHORT nCnt = pSorted ? pSorted->Count() : 0;
/*N*/ 		if( nCnt )
/*N*/ 		{
/*N*/ 			for( MSHORT i=0; i < nCnt; ++i )
/*N*/ 			{
/*N*/ 				SdrObject *pObj = (*pSorted)[ i ];
/*N*/ 				if( pObj->IsWriterFlyFrame() )
/*N*/ 				{
/*N*/ 					SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/ 					lcl_OutFrame( rStr, pFly, rSp, sal_False );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					aTmp = pObj->IsUnoObj() ? "UNO" : "Drw";
/*N*/ 					rStr << aTmp;
/*N*/ 				}
/*N*/ 				if( i < nCnt - 1 )
/*N*/ 					rStr << endl << rSp;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if( pFrm->GetDrawObjs() )
/*N*/ 	{
/*N*/ 		MSHORT nCnt = pFrm->GetDrawObjs()->Count();
/*N*/ 		if( nCnt )
/*N*/ 		{
/*N*/ 			for( MSHORT i=0; i < nCnt; ++i )
/*N*/ 			{
/*N*/ 				SdrObject *pObj = (*pFrm->GetDrawObjs())[ i ];
/*N*/ 				if( pObj->IsWriterFlyFrame() )
/*N*/ 				{
/*N*/ 					SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/ 					lcl_OutFrame( rStr, pFly, rSp, sal_False );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					aTmp = pObj->IsUnoObj() ? "UNO" : "Drw";
/*N*/ 					rStr << aTmp;
/*N*/ 				}
/*N*/ 				if( i < nCnt - 1 )
/*N*/ 					rStr << endl << rSp;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( nSpc )
/*N*/ 		rSp.Erase( rSp.Len() - nSpc );
/*N*/ 	if( bNxt && pFrm->GetNext() )
/*N*/ 	{
/*N*/ 		do
/*N*/ 		{
/*N*/ 			pFrm = pFrm->GetNext();
/*N*/ 			rStr << endl << rSp;
/*N*/ 			lcl_OutFrame( rStr, pFrm, rSp, sal_False );
/*N*/ 		} while ( pFrm->GetNext() );
/*N*/ 	}
/*N*/ }

/*N*/ #ifdef USED
/*N*/ 		IsFtnContFrm()
/*N*/ 		IsFtnFrm()
/*N*/ #endif

/*N*/ void LayOutPut( const SwFrm* pFrm )
/*N*/ {
/*N*/ #ifndef MAC
/*N*/ 	static char *pOutName = 0;
/*N*/ 	const sal_Bool bFirstOpen = pOutName ? sal_False : sal_True;
/*N*/ 	if( bFirstOpen )
/*N*/ 	{
/*N*/ 		char *pPath = getenv( "TEMP" );
/*N*/ 		char *pName = "layout.txt";
/*N*/ 		if( !pPath )
/*N*/ 			pOutName = pName;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			const int nLen = strlen(pPath);
/*N*/ 			// fuer dieses new wird es kein delete geben.
/*N*/ 			pOutName = new char[nLen + strlen(pName) + 3];
/*N*/ 			if(nLen && (pPath[nLen-1] == '\\') || (pPath[nLen-1] == '/'))
/*N*/ 				snprintf( pOutName, sizeof(pOutName), "%s%s", pPath, pName );
/*N*/ 			else
/*N*/ 				snprintf( pOutName, sizeof(pOutName), "%s/%s", pPath, pName );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SvFileStream aStream( pOutName, (bFirstOpen
/*N*/ 										? STREAM_WRITE | STREAM_TRUNC
/*N*/ 										: STREAM_WRITE ));
/*N*/
/*N*/ 	if( !aStream.GetError() )
/*N*/ 	{
/*N*/ 		if ( bFirstOpen )
/*N*/ 			aStream << "Layout-Struktur";
/*N*/ 		else
/*N*/ 			aStream.Seek( STREAM_SEEK_TO_END );
/*N*/ 		aStream << endl;
/*N*/ 		aStream << "---------------------------------------------" << endl;
/*N*/ 		XubString aSpace;
/*N*/ 		lcl_OutFrame( aStream, pFrm, aSpace, sal_False );
/*N*/ 	}
/*N*/ #endif
/*N*/ }
} //namespace binfilter
/*N*/ #endif
namespace binfilter {//STRIP009
/*N*/ SvStream &operator<<( SvStream &rOs, const SwpHints &rHints ) //$ ostream
/*N*/ {
/*N*/ 	rOs << " {HINTS:";
/*N*/ #ifdef JP_NEWCORE
/*N*/ 	for( MSHORT i = 0; i < rHints.GetSize(); ++i)
/*N*/ 	{
/*N*/ 		SwTxtHint *pHint = (SwTxtHint*) rHints[i];
/*N*/
/*N*/ 		if(0 != GetCharWidth(pHint))
/*N*/ 			rOs << "CHARWIDTH" << ' '; // << GetCharWidth(pHint)->frCPI;
/*N*/ 		else if(0 != GetColor(pHint))
/*N*/ 			rOs << "COLOR" << ' ' ; // << GetColor(pHint)->aColor;
/*N*/ 		else if(0 != GetCrossedOut(pHint))
/*N*/ 			rOs << "CROSSEDOUT" << ' ' << (MSHORT)(GetCrossedOut(pHint)->nState);
/*N*/ 		else if(0 != GetAttrFont(pHint))
/*N*/ 			rOs << "ATTRFONT" << ' ' <<
/*N*/ 			(const char *)(GetAttrFont(pHint)->sFamilyName) << ',' <<
/*N*/ 			((MSHORT) GetAttrFont(pHint)->eFamily);
/*N*/ 		else if(0 != GetPosture(pHint))
/*N*/ 			rOs << "POSTURE" << ' ' << GetPosture(pHint)->nPosture;
/*N*/ 		else if(0 != GetFontSize(pHint))
/*N*/ 			rOs << "FONTSIZE" << ' ' << GetFontSize(pHint)->nSize;
/*N*/ 		else if(0 != GetUnderline(pHint))
/*N*/ 			rOs << "UNDERLINE" << ' ' << (MSHORT)(GetUnderline(pHint)->nState);
/*N*/ 		else if(0 != GetWeight(pHint))
/*N*/ 			rOs << "WEIGHT" << ' ' << GetWeight(pHint)->nWeight;
/*N*/ 		else if(0 != GetContour(pHint))
/*N*/ 			rOs << "CONTOUR" << ' ' << GetContour(pHint)->nState;
/*N*/ 		else if(0 != GetShadowed(pHint))
/*N*/ 			rOs << "SHADOWED" << ' ' << GetShadowed(pHint)->nState;
/*N*/ 		else if(0 != GetAutoKern(pHint))
/*N*/ 			rOs << "AUTOKERN" << ' ' << GetAutoKern(pHint)->nState;
/*N*/ 		else if(0 != GetWordLineMode(pHint))
/*N*/ 			rOs << "WORDLINEMODE" << ' ' << GetWordLineMode(pHint)->nState;
/*N*/ 		else
/*N*/ 			rOs << pHint->Which();
/*N*/
/*N*/ 		rOs << ',' << pHint->GetStart()->GetIndex()
/*N*/ 				<< '-'
/*N*/ 				<< (pHint->GetEnd() ? pHint->GetEnd()->GetIndex() : STRING_LEN)
/*N*/ 				<< "\n";
/*N*/ 	}
/*N*/ #endif
/*N*/ 	// JP_NEWCORE
/*N*/
/*N*/ 	rOs << '}';
/*N*/ 	return rOs;
/*N*/ }

/*************************************************************************
 *                          IsDbg()
 *************************************************************************/

/*N*/ sal_Bool IsDbg( const SwTxtFrm *pFrm )
/*N*/ {
/*N*/ 	if( pFrm && pFrm->GetShell() )
/*N*/ 		return pFrm->GetShell()->GetViewOptions()->IsTest4();
/*N*/ 	else
/*N*/ 		return sal_False;
/*N*/ }
} //namespace binfilter

/*N*/ #if OSL_DEBUG_LEVEL < 2
namespace binfilter {//STRIP009
/*N*/ static void Error()
/*N*/ {
/*N*/ 	// wegen PM und BCC
/*N*/ 	sal_Bool bFalse = sal_False;
/*N*/ 	ASSERT( bFalse, "txtio: No debug version" );
/*N*/ }

#define IMPL_OUTOP(class) \
        SvStream &class::operator<<( SvStream &rOs ) const /*$ostream*/\
        { \
            Error(); \
            return rOs; \
        }

/*N*/ IMPL_OUTOP( SwTxtPortion )
/*N*/ IMPL_OUTOP( SwLinePortion )
/*N*/ IMPL_OUTOP( SwBreakPortion )
/*N*/ IMPL_OUTOP( SwGluePortion )
/*N*/ IMPL_OUTOP( SwFldPortion )
/*N*/ IMPL_OUTOP( SwHiddenPortion )
/*N*/ IMPL_OUTOP( SwHyphPortion )
/*N*/ IMPL_OUTOP( SwFixPortion )
/*N*/ IMPL_OUTOP( SwFlyPortion )
/*N*/ IMPL_OUTOP( SwFlyCntPortion )
/*N*/ IMPL_OUTOP( SwMarginPortion )
/*N*/ IMPL_OUTOP( SwNumberPortion )
/*N*/ IMPL_OUTOP( SwBulletPortion )
/*N*/ IMPL_OUTOP( SwGrfNumPortion )
/*N*/ IMPL_OUTOP( SwLineLayout )
/*N*/ IMPL_OUTOP( SwParaPortion )
/*N*/ IMPL_OUTOP( SwFtnPortion )
/*N*/ IMPL_OUTOP( SwFtnNumPortion )
/*N*/ IMPL_OUTOP( SwHyphStrPortion )
/*N*/ IMPL_OUTOP( SwExpandPortion )
/*N*/ IMPL_OUTOP( SwBlankPortion )
/*N*/ IMPL_OUTOP( SwToxPortion )
/*N*/ IMPL_OUTOP( SwRefPortion )
/*N*/ IMPL_OUTOP( SwIsoToxPortion )
/*N*/ IMPL_OUTOP( SwIsoRefPortion )
/*N*/ IMPL_OUTOP( SwSoftHyphPortion )
/*N*/ IMPL_OUTOP( SwTabPortion )
/*N*/ IMPL_OUTOP( SwTabLeftPortion )
/*N*/ IMPL_OUTOP( SwTabRightPortion )
/*N*/ IMPL_OUTOP( SwTabCenterPortion )
/*N*/ IMPL_OUTOP( SwTabDecimalPortion )
/*N*/ IMPL_OUTOP( SwPostItsPortion )
/*N*/ IMPL_OUTOP( SwQuoVadisPortion )
/*N*/ IMPL_OUTOP( SwErgoSumPortion )
/*N*/ IMPL_OUTOP( SwHolePortion )
/*N*/ IMPL_OUTOP( SwDropPortion )
/*N*/ IMPL_OUTOP( SwKernPortion )
/*N*/ IMPL_OUTOP( SwArrowPortion )
/*N*/ IMPL_OUTOP( SwMultiPortion )

/*N*/ const char *GetPortionName( const MSHORT nType )
/*N*/ {
/*N*/ 	return 0;
/*N*/ }

/*N*/ const char *GetPrepName( const PrepareHint ePrep )
/*N*/ {
/*N*/ 	return 0;
/*N*/ }

/*N*/ void SwLineLayout::DebugPortions( SvStream &rOs, const XubString &rTxt, //$ ostream
/*N*/ 												const xub_StrLen nStart )
/*N*/ {
/*N*/ }

/*N*/ const char *GetLangName( const MSHORT nLang )
/*N*/ {
/*N*/ 	return 0;
/*N*/ }
} //namespace binfilter
#else
# include <limits.h>
# include <stdlib.h>
# include "swtypes.hxx"      // ZTCCONST
# include "swfont.hxx"     // SwDropPortion
namespace binfilter {//STRIP009
/*N*/ CONSTCHAR( pClose, "} " );

/*************************************************************************
 *                    GetPortionName()
 *************************************************************************/

/*N*/ CONSTCHAR( pPOR_LIN, "LIN" );
/*N*/ CONSTCHAR( pPOR_TXT, "TXT" );
/*N*/ CONSTCHAR( pPOR_SHADOW, "SHADOW" );
/*N*/ CONSTCHAR( pPOR_TAB, "TAB" );
/*N*/ CONSTCHAR( pPOR_TABLEFT, "TABLEFT" );
/*N*/ CONSTCHAR( pPOR_TABRIGHT, "TABRIGHT" );
/*N*/ CONSTCHAR( pPOR_TABCENTER, "TABCENTER" );
/*N*/ CONSTCHAR( pPOR_TABDECIMAL, "TABDECIMAL" );
/*N*/ CONSTCHAR( pPOR_EXP, "EXP" );
/*N*/ CONSTCHAR( pPOR_HYPH, "HYPH" );
/*N*/ CONSTCHAR( pPOR_HYPHSTR, "HYPHSTR" );
/*N*/ CONSTCHAR( pPOR_FLD, "FLD" );
/*N*/ CONSTCHAR( pPOR_FIX, "FIX" );
/*N*/ CONSTCHAR( pPOR_FLY, "FLY" );
/*N*/ CONSTCHAR( pPOR_FLYCNT, "FLYCNT" );
/*N*/ CONSTCHAR( pPOR_MARGIN, "MARGIN" );
/*N*/ CONSTCHAR( pPOR_GLUE, "GLUE" );
/*N*/ CONSTCHAR( pPOR_HOLE, "HOLE" );
/*N*/ CONSTCHAR( pPOR_END, "END" );
/*N*/ CONSTCHAR( pPOR_BRK, "BRK" );
/*N*/ CONSTCHAR( pPOR_LAY, "LAY" );
/*N*/ CONSTCHAR( pPOR_BLANK, "BLANK" );
/*N*/ CONSTCHAR( pPOR_FTN, "FTN" );
/*N*/ CONSTCHAR( pPOR_FTNNUM, "FTNNUM" );
/*N*/ CONSTCHAR( pPOR_POSTITS, "POSTITS" );
/*N*/ CONSTCHAR( pPOR_SOFTHYPH, "SOFTHYPH" );
/*N*/ CONSTCHAR( pPOR_SOFTHYPHSTR, "SOFTHYPHSTR" );
/*N*/ CONSTCHAR( pPOR_TOX, "TOX" );
/*N*/ CONSTCHAR( pPOR_REF, "REF" );
/*N*/
/*N*/ CONSTCHAR( pPOR_ISOTOX, "ISOTOX" );
/*N*/ CONSTCHAR( pPOR_ISOREF, "ISOREF" );
/*N*/ CONSTCHAR( pPOR_HIDDEN, "Hidden" );
/*N*/ CONSTCHAR( pPOR_QUOVADIS, "QuoVadis" );
/*N*/ CONSTCHAR( pPOR_ERGOSUM, "ErgoSum" );
/*N*/ CONSTCHAR( pPOR_NUMBER, "NUMBER" );
/*N*/ CONSTCHAR( pPOR_BULLET, "BULLET" );
/*N*/ CONSTCHAR( pPOR_UNKW, "UNKW" );
/*N*/ CONSTCHAR( pPOR_PAR, "PAR" );

/*N*/ const char *GetPortionName( const MSHORT nType )
/*N*/ {
/*N*/ #ifdef USED
/*N*/ 	// Kurz und schmerzlos:
/*N*/ 	const char *ppNameArr[PORTYPE_END] = {
/*N*/ 	  pPOR_LIN, pPOR_TXT, pPOR_HOLE, pPOR_SHADOW,
/*N*/ 	  pPOR_TAB, pPOR_TABLEFT, pPOR_TABRIGHT, pPOR_TABCENTER, pPOR_TABDECIMAL,
/*N*/ 	  pPOR_EXP, pPOR_HYPH, pPOR_HYPHSTR, pPOR_FLD,
/*N*/ 	  pPOR_FIX, pPOR_FLY, pPOR_FLYCNT, pPOR_MARGIN,
/*N*/ 	  pPOR_GLUE, pPOR_END, pPOR_BRK, pPOR_LAY,
/*N*/ 	  pPOR_BLANK, pPOR_FTN, pPOR_FTNNUM,
/*N*/ 	  pPOR_POSTITS, pPOR_SOFTHYPH, pPOR_SOFTHYPHSTR,
/*N*/ 	  pPOR_TOX, pPOR_REF, pPOR_ISOTOX, pPOR_ISOREF,
/*N*/ 	  pPOR_HIDDEN, pPOR_QUOVADIS, pPOR_ERGOSUM,
/*N*/ 	  pPOR_NUMBER, pPOR_BULLET, pPOR_UNKW, pPOR_PAR
/*N*/ 	};
/*N*/ 	ASSERT( eType < PORTYPE_END, "GetPortionName: bad type" );
/*N*/ 	return( ppNameArr[eType] );
/*N*/ #else
/*N*/ 	return 0;
/*N*/ #endif
/*N*/ }

/*N*/ CONSTCHAR( pPREP_CLEAR, "CLEAR" );
/*N*/ CONSTCHAR( pPREP_WIDOWS_ORPHANS, "WIDOWS_ORPHANS" );
/*N*/ CONSTCHAR( pPREP_FIXSIZE_CHG, "FIXSIZE_CHG" );
/*N*/ CONSTCHAR( pPREP_FOLLOW_FOLLOWS, "FOLLOW_FOLLOWS" );
/*N*/ CONSTCHAR( pPREP_ADJUST_FRM, "ADJUST_FRM" );
/*N*/ CONSTCHAR( pPREP_FREE_SPACE, "FREE_SPACE" );
/*N*/ CONSTCHAR( pPREP_FLY_CHGD, "FLY_CHGD" );
/*N*/ CONSTCHAR( pPREP_FLY_ATTR_CHG, "FLY_ATTR_CHG" );
/*N*/ CONSTCHAR( pPREP_FLY_ARRIVE, "FLY_ARRIVE" );
/*N*/ CONSTCHAR( pPREP_FLY_LEAVE, "FLY_LEAVE" );
/*N*/ CONSTCHAR( pPREP_VIEWOPT, "VIEWOPT" );
/*N*/ CONSTCHAR( pPREP_FTN, "FTN" );
/*N*/ CONSTCHAR( pPREP_POS_CHGD, "POS" );
/*N*/ CONSTCHAR( pPREP_UL_SPACE, "UL_SPACE" );
/*N*/ CONSTCHAR( pPREP_MUST_FIT, "MUST_FIT" );
/*N*/ CONSTCHAR( pPREP_WIDOWS, "ORPHANS" );
/*N*/ CONSTCHAR( pPREP_QUOVADIS, "QUOVADIS" );
/*N*/ CONSTCHAR( pPREP_PAGE, "PAGE" );

/*N*/ const char *GetPrepName( const PrepareHint ePrep )
/*N*/ {
/*N*/ 	// Kurz und schmerzlos:
/*N*/ 	const char *ppNameArr[PREP_END] =
/*N*/ 	{
/*N*/ 		pPREP_CLEAR, pPREP_WIDOWS_ORPHANS, pPREP_FIXSIZE_CHG,
/*N*/ 		pPREP_FOLLOW_FOLLOWS, pPREP_ADJUST_FRM, pPREP_FREE_SPACE,
/*N*/ 		pPREP_FLY_CHGD, pPREP_FLY_ATTR_CHG, pPREP_FLY_ARRIVE,
/*N*/ 		pPREP_FLY_LEAVE, pPREP_VIEWOPT, pPREP_FTN, pPREP_POS_CHGD,
/*N*/ 		pPREP_UL_SPACE, pPREP_MUST_FIT, pPREP_WIDOWS, pPREP_QUOVADIS,
/*N*/ 		pPREP_PAGE
/*N*/ 	};
/*N*/ 	ASSERT( ePrep < PREP_END, "GetPrepName: unknown PrepareHint" );
/*N*/ 	return( ppNameArr[ePrep] );
/*N*/ }

/*************************************************************************
 *                    SwLineLayout::DebugPortions()
 *
 * DebugPortion() iteriert ueber alle Portions einer Zeile und deckt die
 * internen Strukturen auf.
 * Im Gegensatz zum Ausgabe-Operator werden auch die Textteile ausgegeben.
 *************************************************************************/

/*N*/ void SwLineLayout::DebugPortions( SvStream &rOs, const XubString &rTxt, //$ ostream
/*N*/ 												const xub_StrLen nStart )
/*N*/ {
/*N*/ 	SwLinePortion *pPortion = GetPortion();
/*N*/
/*N*/ 	xub_StrLen nPos = 0;
/*N*/ 	MSHORT nNr = 0;
/*N*/ 	KSHORT nPrtWidth, nLastPrt;
/*N*/ 	nPrtWidth = nLastPrt = 0;
/*N*/
/*N*/ 	SwLinePortion::operator<<( rOs );
/*N*/ 	rOs << '\"' << endl;
/*N*/
/*N*/ 	while( pPortion )
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 		SwTxtPortion *pTxtPor = pPortion->InTxtGrp() ?
/*N*/ 								(SwTxtPortion *)pPortion : NULL ;
/*N*/ 		++nNr;
/*N*/ 		nLastPrt = nPrtWidth;
/*N*/ 		nPrtWidth += pPortion->PrtWidth();
/*N*/ 		rOs << "\tNr:"  << nNr
/*N*/ 			<< " Pos:" << nPos
/*N*/ 			<< " Org:" << nLastPrt
/*N*/ 			<< endl;
/*N*/
/*N*/ 		rOs << "\t";
/*N*/ 		pPortion->operator<<( rOs );
/*N*/ 		rOs << endl;
/*N*/ 		nPos += pPortion->GetLen();
/*N*/ 		pPortion = pPortion->GetPortion();
/*N*/ 	}
/*N*/ }

/*N*/ #ifdef USED
/*N*/ CONSTCHAR( pRES_LNG_ALBANIAN, "ALBANIAN" );
/*N*/ CONSTCHAR( pRES_LNG_ARABIC, "ARABIC" );
/*N*/ CONSTCHAR( pRES_LNG_AUS_ENGLISH, "AUS_ENGLISH" );
/*N*/ CONSTCHAR( pRES_LNG_BAHASA, "BAHASA" );
/*N*/ CONSTCHAR( pRES_LNG_BELGIAN_DUTCH, "BELGIAN_DUTCH" );
/*N*/ CONSTCHAR( pRES_LNG_BELGIAN_FRENCH, "BELGIAN_FRENCH" );
/*N*/ CONSTCHAR( pRES_LNG_BRAZIL_PORT, "BRAZIL_PORT" );
/*N*/ CONSTCHAR( pRES_LNG_BULGARIAN, "BULGARIAN" );
/*N*/ CONSTCHAR( pRES_LNG_CANADA_FRENCH, "CANADA_FRENCH" );
/*N*/ CONSTCHAR( pRES_LNG_CAST_SPANISH, "CAST_SPANISH" );
/*N*/ CONSTCHAR( pRES_LNG_CATALAN, "CATALAN" );
/*N*/ CONSTCHAR( pRES_LNG_CROATO_SERBIAN, "CROATO_SERBIAN" );
/*N*/ CONSTCHAR( pRES_LNG_CZECH, "CZECH" );
/*N*/ CONSTCHAR( pRES_LNG_DANISH, "DANISH" );
/*N*/ CONSTCHAR( pRES_LNG_DUTCH, "DUTCH" );
/*N*/ CONSTCHAR( pRES_LNG_FINNISH, "FINNISH" );
/*N*/ CONSTCHAR( pRES_LNG_FRENCH, "FRENCH" );
/*N*/ CONSTCHAR( pRES_LNG_GERMAN, "GERMAN" );
/*N*/ CONSTCHAR( pRES_LNG_GREEK, "GREEK" );
/*N*/ CONSTCHAR( pRES_LNG_HEBREW, "HEBREW" );
/*N*/ CONSTCHAR( pRES_LNG_HUNGARIAN, "HUNGARIAN" );
/*N*/ CONSTCHAR( pRES_LNG_ICELANDIC, "ICELANDIC" );
/*N*/ CONSTCHAR( pRES_LNG_ITALIAN, "ITALIAN" );
/*N*/ CONSTCHAR( pRES_LNG_JAPANESE, "JAPANESE" );
/*N*/ CONSTCHAR( pRES_LNG_KOREAN, "KOREAN" );
/*N*/ CONSTCHAR( pRES_LNG_MEXICAN_SPANISH, "MEXICAN_SPANISH" );
/*N*/ CONSTCHAR( pRES_LNG_NORWEG_BOKMAL, "NORWEG_BOKMAL" );
/*N*/ CONSTCHAR( pRES_LNG_NORWEG_NYNORSK, "NORWEG_NYNORSK" );
/*N*/ CONSTCHAR( pRES_LNG_POLISH, "POLISH" );
/*N*/ CONSTCHAR( pRES_LNG_PORTUGUESE, "PORTUGUESE" );
/*N*/ CONSTCHAR( pRES_LNG_RHAETO_ROMANIC, "RHAETO_ROMANIC" );
/*N*/ CONSTCHAR( pRES_LNG_ROMANIAN, "ROMANIAN" );
/*N*/ CONSTCHAR( pRES_LNG_RUSSIAN, "RUSSIAN" );
/*N*/ CONSTCHAR( pRES_LNG_SERBO_CROATIAN, "SERBO_CROATIAN" );
/*N*/ CONSTCHAR( pRES_LNG_SIM_CHINESE, "SIM_CHINESE" );
/*N*/ CONSTCHAR( pRES_LNG_SLOVAKIAN, "SLOVAKIAN" );
/*N*/ CONSTCHAR( pRES_LNG_SWEDISH, "SWEDISH" );
/*N*/ CONSTCHAR( pRES_LNG_SWISS_FRENCH, "SWISS_FRENCH" );
/*N*/ CONSTCHAR( pRES_LNG_SWISS_GERMAN, "SWISS_GERMAN" );
/*N*/ CONSTCHAR( pRES_LNG_SWISS_ITALIAN, "SWISS_ITALIAN" );
/*N*/ CONSTCHAR( pRES_LNG_THAI, "THAI" );
/*N*/ CONSTCHAR( pRES_LNG_TRD_CHINESE, "TRD_CHINESE" );
/*N*/ CONSTCHAR( pRES_LNG_TURKISH, "TURKISH" );
/*N*/ CONSTCHAR( pRES_LNG_UK_ENGLISH, "UK_ENGLISH" );
/*N*/ CONSTCHAR( pRES_LNG_URDU, "URDU" );
/*N*/ CONSTCHAR( pRES_LNG_US_ENGLISH, "US_ENGLISH" );
/*N*/ CONSTCHAR( pRES_LNG_NOLANGUAGE, "NOLANGUAGE" );
/*N*/
/*N*/ const char *GetLangName( const MSHORT nLang )
/*N*/ {
/*N*/ 	switch( nLang )
/*N*/ 	{
/*N*/ 		case 0x041c : return pRES_LNG_ALBANIAN;
/*N*/ 		case 0x0401 : return pRES_LNG_ARABIC;
/*N*/ 		case 0x0c09 : return pRES_LNG_AUS_ENGLISH;
/*N*/ 		case 0x0421 : return pRES_LNG_BAHASA;
/*N*/ 		case 0x0813 : return pRES_LNG_BELGIAN_DUTCH;
/*N*/ 		case 0x080c : return pRES_LNG_BELGIAN_FRENCH;
/*N*/ 		case 0x0416 : return pRES_LNG_BRAZIL_PORT;
/*N*/ 		case 0x0402 : return pRES_LNG_BULGARIAN;
/*N*/ 		case 0x0c0c : return pRES_LNG_CANADA_FRENCH;
/*N*/ 		case 0x040a : return pRES_LNG_CAST_SPANISH;
/*N*/ 		case 0x0403 : return pRES_LNG_CATALAN;
/*N*/ 		case 0x041a : return pRES_LNG_CROATO_SERBIAN;
/*N*/ 		case 0x0405 : return pRES_LNG_CZECH;
/*N*/ 		case 0x0406 : return pRES_LNG_DANISH;
/*N*/ 		case 0x0413 : return pRES_LNG_DUTCH;
/*N*/ 		case 0x040b : return pRES_LNG_FINNISH;
/*N*/ 		case 0x040c : return pRES_LNG_FRENCH;
/*N*/ 		case 0x0407 : return pRES_LNG_GERMAN;
/*N*/ 		case 0x0408 : return pRES_LNG_GREEK;
/*N*/ 		case 0x040d : return pRES_LNG_HEBREW;
/*N*/ 		case 0x040e : return pRES_LNG_HUNGARIAN;
/*N*/ 		case 0x040f : return pRES_LNG_ICELANDIC;
/*N*/ 		case 0x0410 : return pRES_LNG_ITALIAN;
/*N*/ 		case 0x0411 : return pRES_LNG_JAPANESE;
/*N*/ 		case 0x0412 : return pRES_LNG_KOREAN;
/*N*/ 		case 0x080a : return pRES_LNG_MEXICAN_SPANISH;
/*N*/ 		case 0x0414 : return pRES_LNG_NORWEG_BOKMAL;
/*N*/ 		case 0x0814 : return pRES_LNG_NORWEG_NYNORSK;
/*N*/ 		case 0x0415 : return pRES_LNG_POLISH;
/*N*/ 		case 0x0816 : return pRES_LNG_PORTUGUESE;
/*N*/ 		case 0x0417 : return pRES_LNG_RHAETO_ROMANIC;
/*N*/ 		case 0x0418 : return pRES_LNG_ROMANIAN;
/*N*/ 		case 0x0419 : return pRES_LNG_RUSSIAN;
/*N*/ 		case 0x081a : return pRES_LNG_SERBO_CROATIAN;
/*N*/ 		case 0x0804 : return pRES_LNG_SIM_CHINESE;
/*N*/ 		case 0x041b : return pRES_LNG_SLOVAKIAN;
/*N*/ 		case 0x041d : return pRES_LNG_SWEDISH;
/*N*/ 		case 0x100c : return pRES_LNG_SWISS_FRENCH;
/*N*/ 		case 0x0807 : return pRES_LNG_SWISS_GERMAN;
/*N*/ 		case 0x0810 : return pRES_LNG_SWISS_ITALIAN;
/*N*/ 		case 0x041e : return pRES_LNG_THAI;
/*N*/ 		case 0x0404 : return pRES_LNG_TRD_CHINESE;
/*N*/ 		case 0x041f : return pRES_LNG_TURKISH;
/*N*/ 		case 0x0809 : return pRES_LNG_UK_ENGLISH;
/*N*/ 		case 0x0420 : return pRES_LNG_URDU;
/*N*/ 		case 0x0409 : return pRES_LNG_US_ENGLISH;
/*N*/ 		default     : return pRES_LNG_NOLANGUAGE;
/*N*/ 	}
/*N*/ }
/*N*/ #else
/*N*/
/*N*/ const char *GetLangName( const MSHORT nLang )
/*N*/ {
/*N*/ 	return "???";
/*N*/ }
/*N*/ #endif
/*N*/
/*N*/ SvStream &SwLinePortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	rOs << " {";
/*N*/ 	rOs <<	"L:" << nLineLength;
/*N*/ 	rOs << " H:" << Height();
/*N*/ 	rOs << " W:" << PrtWidth();
/*N*/ 	rOs << " A:" << nAscent;
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwTxtPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {TXT:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwLinePortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwBreakPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {BREAK:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwLinePortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwKernPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {KERN:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwLinePortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwArrowPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {ARROW:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwLinePortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwMultiPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {MULTI:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwLinePortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwLineLayout::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {LINE:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwLinePortion::operator<<( rOs );
/*N*/ 	SwLinePortion *pPos = GetPortion();
/*N*/ 	while( pPos )
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 		rOs << "\t";
/*N*/ 		pPos->operator<<( rOs );
/*N*/ 		pPos = pPos->GetPortion();
/*N*/ 	}
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwGluePortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {GLUE:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwLinePortion::operator<<( rOs );
/*N*/ 	rOs << " F:" << GetFixWidth();
/*N*/ 	rOs << " G:" << GetPrtGlue();
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwFixPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {FIX:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwGluePortion::operator<<( rOs );
/*N*/ 	rOs << " Fix:" << nFix;
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwFlyPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {FLY:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwFixPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwMarginPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {MAR:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwGluePortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwFlyCntPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {FLYCNT:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwLinePortion::operator<<( rOs );
/*N*/ 	if( bDraw )
/*N*/ 	{
/*N*/ 		CONSTCHAR( pTxt, " {DRAWINCNT" );
/*N*/ 		rOs << pTxt;
/*N*/ 		rOs << pClose;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		CONSTCHAR( pTxt, " {FRM:" );
/*N*/ 		rOs << pTxt;
/*N*/ 		rOs << " {FRM:" << GetFlyFrm()->Frm() << pClose;
/*N*/ 		rOs << " {PRT:" << GetFlyFrm()->Prt() << pClose;
/*N*/ 		rOs << pClose;
/*N*/ 	}
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwExpandPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {EXP:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwLinePortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwFtnPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {FTN:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwExpandPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwFtnNumPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {FTNNUM:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwNumberPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwNumberPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {NUMBER:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwExpandPortion::operator<<( rOs );
/*N*/ 	rOs << " Exp:\"" << '\"';
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwBulletPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {BULLET:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwNumberPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwGrfNumPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {GRFNUM:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwGrfNumPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwHiddenPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {Hidden:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwFldPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwToxPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {TOX:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwTxtPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwRefPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {Ref:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwTxtPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwIsoToxPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {ISOTOX:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwToxPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwIsoRefPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {ISOREF:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwRefPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwHyphPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {HYPH:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwExpandPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwHyphStrPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {HYPHSTR:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwExpandPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwSoftHyphPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {SOFTHYPH:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwHyphPortion::operator<<( rOs );
/*N*/ 	rOs << (IsExpand() ? " on" : " off");
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwBlankPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {BLANK:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwExpandPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwFldPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {FLD:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwLinePortion::operator<<( rOs );
/*N*/ 	if( IsFollow() )
/*N*/ 		rOs << " F!";
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwPostItsPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {POSTITS" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwLinePortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwTabPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {TAB" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwFixPortion::operator<<( rOs );
/*N*/ 	rOs << " T:" << nTabPos;
/*N*/ 	if( IsFilled() )
/*N*/ 		rOs << " \"" << cFill << '\"';
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwTabLeftPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {TABLEFT" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwTabPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwTabRightPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {TABRIGHT" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwTabPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwTabCenterPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {TABCENTER" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwTabPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwTabDecimalPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {TABDECIMAL" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwTabPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwParaPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {PAR" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwLineLayout::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwHolePortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {HOLE" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwLinePortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwQuoVadisPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {QUOVADIS" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwFldPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwErgoSumPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {ERGOSUM" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwFldPortion::operator<<( rOs );
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &operator<<( SvStream &rOs, const SwTxtSizeInfo &rInf ) //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {SIZEINFO:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	rOs << ' ' << (rInf.OnWin() ? "WIN:" : "PRT:" );
/*N*/ 	rOs << " Idx:" << rInf.GetIdx();
/*N*/ 	rOs << " Len:" << rInf.GetLen();
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
/*N*/
/*N*/ SvStream &SwDropPortion::operator<<( SvStream &rOs ) const //$ ostream
/*N*/ {
/*N*/ 	CONSTCHAR( pTxt, " {DROP:" );
/*N*/ 	rOs << pTxt;
/*N*/ 	SwTxtPortion::operator<<( rOs );
/*N*/     if( pPart && nDropHeight )
/*N*/ 	{
/*N*/ 		rOs << " H:" << nDropHeight;
/*N*/ 		rOs << " L:" << nLines;
/*N*/         rOs <<" Fnt:" << pPart->GetFont().GetHeight();
/*N*/         if( nX || nY )
/*N*/             rOs << " [" << nX << '/' << nY << ']';
/*N*/ 	}
/*N*/ 	rOs << pClose;
/*N*/ 	return rOs;
/*N*/ }
} //namespace binfilter
/*N*/ #endif /* OSL_DEBUG_LEVEL */



