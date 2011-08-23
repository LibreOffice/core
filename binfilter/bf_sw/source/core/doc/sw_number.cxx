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

#include <string.h>

#ifndef _SV_FONT_HXX //autogen
#include <vcl/font.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
namespace binfilter {

USHORT SwNumRule::nRefCount = 0;
SwNumFmt* SwNumRule::aBaseFmts[ RULE_END ][ MAXLEVEL ] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0 };

Font* SwNumRule::pDefBulletFont = 0;
sal_Char* SwNumRule::pDefOutlineName = "Outline";

USHORT SwNumRule::aDefNumIndents[ MAXLEVEL ] = {
//cm:   0,5  1,0  1,5  2,0   2,5   3,0   3,5   4,0   4,5   5,0
        283, 567, 850, 1134, 1417, 1701, 1984, 2268, 2551, 2835
};

#if defined( UNX ) && defined( GCC )
extern const sal_Char __FAR_DATA sBulletFntName[];
const sal_Char __FAR_DATA sBulletFntName[] = "StarSymbol";
#else
extern const sal_Char __FAR_DATA sBulletFntName[] = "StarSymbol";
#endif

/*N*/ inline void lcl_SetRuleChgd( SwTxtNode& rNd, BYTE nLevel )
/*N*/ {
/*N*/ 	if( rNd.GetNum() &&
/*N*/ 		(~NO_NUMLEVEL & rNd.GetNum()->GetLevel() ) == nLevel )
/*N*/ 		rNd.NumRuleChgd();
/*N*/ }
/* -----------------------------22.02.01 13:41--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwNumFmt::SwNumFmt() :
/*N*/ 	SwClient( 0 ),
/*N*/ 	SvxNumberFormat(SVX_NUM_ARABIC),
/*N*/     pVertOrient(new SwFmtVertOrient( 0, VERT_NONE))
/*N*/ {
/*N*/ }
/* -----------------------------22.02.01 13:42--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwNumFmt::SwNumFmt( const SwNumFmt& rFmt) :
/*N*/ 	SwClient( rFmt.pRegisteredIn ),
/*N*/ 	SvxNumberFormat(rFmt),
/*N*/     pVertOrient(new SwFmtVertOrient( 0, (SwVertOrient)rFmt.GetVertOrient()))
/*N*/ {
/*N*/ 	SvxFrameVertOrient eVertOrient = rFmt.GetVertOrient();
/*N*/ 	SetGraphicBrush( rFmt.GetBrush(), &rFmt.GetGraphicSize(),
/*N*/ 												&eVertOrient);
/*N*/ }
/* -----------------------------22.02.01 13:58--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwNumFmt::SwNumFmt(const SvxNumberFormat& rNumFmt, SwDoc* pDoc) :
/*N*/ 	SvxNumberFormat(rNumFmt),
/*N*/     pVertOrient(new SwFmtVertOrient( 0, (SwVertOrient)rNumFmt.GetVertOrient()))
/*N*/ {
/*N*/     SvxFrameVertOrient eVertOrient = rNumFmt.GetVertOrient();
/*N*/     SetGraphicBrush( rNumFmt.GetBrush(), &rNumFmt.GetGraphicSize(),
/*N*/ 												&eVertOrient);
/*N*/     const String& rCharStyleName = rNumFmt.SvxNumberFormat::GetCharFmtName();
/*N*/ 	if( rCharStyleName.Len() )
/*N*/ 	{
/*N*/ 		SwCharFmt* pCFmt = pDoc->FindCharFmtByName( rCharStyleName );
/*N*/ 		if( !pCFmt )
/*N*/ 		{
/*N*/ 			USHORT nId = SwStyleNameMapper::GetPoolIdFromUIName( rCharStyleName,
/*N*/ 											GET_POOLID_CHRFMT );
/*?*/ 			pCFmt = nId != USHRT_MAX
/*?*/ 						? pDoc->GetCharFmtFromPool( nId )
/*?*/ 						: pDoc->MakeCharFmt( rCharStyleName, 0 );
/*N*/ 		}
/*N*/ 		pCFmt->Add( this );
/*N*/ 	}
/*N*/ 	else if( GetRegisteredIn() )
/*?*/ 		pRegisteredIn->Remove( this );

/*N*/ }
/* -----------------------------22.02.01 13:42--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwNumFmt::~SwNumFmt()
/*N*/ {
/*N*/ 	delete pVertOrient;
/*N*/ }
/* -----------------------------02.07.01 15:37--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void SwNumFmt::NotifyGraphicArrived()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }
/* -----------------------------23.02.01 09:28--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SwNumFmt& SwNumFmt::operator=( const SwNumFmt& rNumFmt)
/*N*/ {
/*N*/ 	SvxNumberFormat::operator=(rNumFmt);
/*N*/ 	if( rNumFmt.GetRegisteredIn() )
/*N*/ 		rNumFmt.pRegisteredIn->Add( this );
/*N*/ 	else if( GetRegisteredIn() )
/*?*/ 		pRegisteredIn->Remove( this );
/*N*/ 	return *this;
/*N*/ }
/* -----------------------------23.02.01 09:28--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ BOOL SwNumFmt::operator==( const SwNumFmt& rNumFmt) const
/*N*/ {
/*N*/ 	BOOL bRet = SvxNumberFormat::operator==(rNumFmt) &&
/*N*/ 		pRegisteredIn == rNumFmt.pRegisteredIn;
/*N*/     return bRet;
/*N*/ }

/* -----------------------------22.02.01 13:42--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ const Graphic* SwNumFmt::GetGraphic() const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return 0; //STRIP001 
/*N*/ }
/* -----------------------------22.02.01 13:44--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void SwNumFmt::SetCharFmt( SwCharFmt* pChFmt)
/*N*/ {
/*N*/ 	if( pChFmt )
/*N*/ 		pChFmt->Add( this );
/*N*/ 	else if( GetRegisteredIn() )
/*?*/ 		pRegisteredIn->Remove( this );
/*N*/ }
/* -----------------------------22.02.01 13:45--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void SwNumFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
/*N*/ {
/*N*/ 	// dann suche mal in dem Doc nach dem NumRules-Object, in dem dieses
/*N*/ 	// NumFormat gesetzt ist. Das Format muss es nicht geben!
/*N*/ 	const SwCharFmt* pFmt = 0;
/*N*/ 	switch( pOld ? pOld->Which() : pNew ? pNew->Which() : 0 )
/*N*/ 	{
/*N*/ 	case RES_ATTRSET_CHG:
/*N*/ 	case RES_FMT_CHG:
/*N*/ 		pFmt = GetCharFmt();
/*N*/ 		break;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pFmt && !pFmt->GetDoc()->IsInDtor() )
/*N*/ 		UpdateNumNodes( (SwDoc*)pFmt->GetDoc() );
/*N*/ 	else
/*N*/ 		SwClient::Modify( pOld, pNew );
/*N*/ }
/* -----------------------------23.02.01 11:08--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void SwNumFmt::SetCharFmtName(const String& rSet)
/*N*/ {
/*N*/ 	SvxNumberFormat::SetCharFmtName(rSet);
/*N*/ }
/* -----------------------------22.02.01 13:47--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ const String&	SwNumFmt::GetCharFmtName() const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 		return aEmptyStr;
/*N*/ }
/* -----------------------------22.02.01 16:05--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void	SwNumFmt::SetGraphicBrush( const SvxBrushItem* pBrushItem, const Size* pSize,
/*N*/ 	const SvxFrameVertOrient* pOrient)
/*N*/ {
/*N*/     if(pOrient)
/*N*/         pVertOrient->SetVertOrient( (SwVertOrient)*pOrient );
/*N*/ 	SvxNumberFormat::SetGraphicBrush( pBrushItem, pSize, pOrient);
/*N*/ }
/* -----------------------------22.02.01 16:05--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void	SwNumFmt::SetVertOrient(SvxFrameVertOrient eSet)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }
/* -----------------------------22.02.01 16:05--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ SvxFrameVertOrient 	SwNumFmt::GetVertOrient() const
/*N*/ {
/*N*/     return SvxNumberFormat::GetVertOrient();
/*N*/ }
/* -----------------------------22.02.01 13:54--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ void SwNumFmt::UpdateNumNodes( SwDoc* pDoc )
/*N*/ {
/*N*/ 	BOOL bDocIsModified = pDoc->IsModified();
/*N*/ 	BOOL bFnd = FALSE;
/*N*/ 	const SwNumRule* pRule;
/*N*/ 	for( USHORT n = pDoc->GetNumRuleTbl().Count(); !bFnd && n; )
/*N*/ 	{
/*N*/ 		pRule = pDoc->GetNumRuleTbl()[ --n ];
/*N*/ 		for( BYTE i = 0; i < MAXLEVEL; ++i )
/*N*/ 			if( pRule->GetNumFmt( i ) == this )
/*N*/ 			{
/*N*/ 				const String& rRuleNm = pRule->GetName();
/*N*/ 
/*N*/ 				SwModify* pMod;
/*N*/ 				const SfxPoolItem* pItem;
/*N*/ 				USHORT k, nMaxItems = pDoc->GetAttrPool().GetItemCount(
/*N*/ 													RES_PARATR_NUMRULE );
/*N*/ 				for( k = 0; k < nMaxItems; ++k )
/*N*/ 					if( 0 != (pItem = pDoc->GetAttrPool().GetItem(
/*N*/ 						RES_PARATR_NUMRULE, k ) ) &&
/*N*/ 						0 != ( pMod = (SwModify*)((SwNumRuleItem*)pItem)->
/*N*/ 								GetDefinedIn()) &&
/*N*/ 						((SwNumRuleItem*)pItem)->GetValue() == rRuleNm )
/*N*/ 					{
/*N*/ 						if( pMod->IsA( TYPE( SwFmt )) )
/*N*/ 						{
/*?*/ 							DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwNumRuleInfo aInfo( rRuleNm );
/*?*/ 						}
/*N*/ 						else if( ((SwTxtNode*)pMod)->GetNodes().IsDocNodes() )
/*N*/ 							lcl_SetRuleChgd( *(SwTxtNode*)pMod, i );
/*N*/ 					}
/*N*/ 				bFnd = TRUE;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !bFnd )
/*N*/ 	{
/*?*/ 		pRule = pDoc->GetOutlineNumRule();
/*?*/ 		for( BYTE i = 0; i < MAXLEVEL; ++i )
/*?*/ 			if( pRule->GetNumFmt( i ) == this )
/*?*/ 			{
/*?*/ 				ULONG nStt = pDoc->GetNodes().GetEndOfContent().StartOfSectionIndex();
/*?*/ 				const SwTxtFmtColls& rColls = *pDoc->GetTxtFmtColls();
/*?*/ 				for( USHORT n = 1; n < rColls.Count(); ++n )
/*?*/ 				{
/*?*/ 					const SwTxtFmtColl* pColl = rColls[ n ];
/*?*/ 					if( i == pColl->GetOutlineLevel() )
/*?*/ 					{
/*?*/ 						SwClientIter aIter( *(SwTxtFmtColl*)pColl );
/*?*/ 						for( SwTxtNode* pNd = (SwTxtNode*)aIter.First( TYPE( SwTxtNode ));
/*?*/ 								pNd; pNd = (SwTxtNode*)aIter.Next() )
/*?*/ 							if( pNd->GetNodes().IsDocNodes() &&
/*?*/ 													nStt < pNd->GetIndex() &&
/*?*/ 								pNd->GetOutlineNum() && i == (~NO_NUMLEVEL &
/*?*/ 								pNd->GetOutlineNum()->GetLevel() ) )
/*?*/ 									pNd->NumRuleChgd();
/*?*/ 						break;
/*?*/ 					}
/*?*/ 				}
/*?*/ 
/*?*/ 				bFnd = TRUE;
/*?*/ 				break;
/*?*/ 			}
/*?*/ 	}
/*N*/ 
/*N*/ 	if( bFnd && !bDocIsModified )
/*?*/ 		pDoc->ResetModified();
/*N*/ }
/* -----------------------------31.05.01 16:08--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ const SwFmtVertOrient*      SwNumFmt::GetGraphicOrientation() const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 
/*N*/ }

/*N*/ BOOL SwNodeNum::operator==( const SwNodeNum& rNum ) const
/*N*/ {
/*N*/ 	return nMyLevel == rNum.nMyLevel &&
/*N*/ 		   nSetValue == rNum.nSetValue &&
/*N*/ 		   bStartNum == rNum.bStartNum &&
/*N*/ 		   ( nMyLevel >= MAXLEVEL ||
/*N*/ 			 0 == memcmp( nLevelVal, rNum.nLevelVal,
/*N*/ 						sizeof( USHORT ) * (nMyLevel+1) ));
/*N*/ }

/*N*/ SwNumRule::SwNumRule( const String& rNm, SwNumRuleType eType, BOOL bAutoFlg )
/*N*/ 	: eRuleType( eType ),
/*N*/ 	sName( rNm ),
/*N*/ 	bAutoRuleFlag( bAutoFlg ),
/*N*/ 	bInvalidRuleFlag( TRUE ),
/*N*/ 	bContinusNum( FALSE ),
/*N*/ 	bAbsSpaces( FALSE ),
/*N*/ 	nPoolFmtId( USHRT_MAX ),
/*N*/ 	nPoolHelpId( USHRT_MAX ),
/*N*/ 	nPoolHlpFileId( UCHAR_MAX )
/*N*/ {
/*N*/ 	if( !nRefCount++ )			// zum erstmal, also initialisiern
/*N*/ 	{
/*N*/ 		SwNumFmt* pFmt;
/*N*/ 		// Nummerierung:
            int n=0;
/*N*/ 		for( n = 0; n < MAXLEVEL; ++n )
/*N*/ 		{
/*N*/ 			pFmt = new SwNumFmt;
/*N*/ 			pFmt->SetIncludeUpperLevels( 1 );
/*N*/ 			pFmt->SetStart( 1 );
/*N*/ 			pFmt->SetLSpace( lNumIndent );
/*N*/ 			pFmt->SetAbsLSpace( SwNumRule::GetNumIndent( n ) );
/*N*/ 			pFmt->SetFirstLineOffset( lNumFirstLineOffset );
/*N*/ 			pFmt->SetSuffix( aDotStr );
/*N*/ 			SwNumRule::aBaseFmts[ NUM_RULE ][ n ] = pFmt;
/*N*/ 		}
/*N*/ 
/*N*/ 		// Gliederung:
/*N*/ 		for( n = 0; n < MAXLEVEL; ++n )
/*N*/ 		{
/*N*/ 			pFmt = new SwNumFmt;
/*N*/ //JP 18.01.96: heute soll es mal wieder vollstaendig numeriert werden
/*N*/ //JP 10.03.96: und nun mal wieder nicht
/*N*/ 			pFmt->SetNumberingType(SVX_NUM_NUMBER_NONE);
/*N*/ 			pFmt->SetIncludeUpperLevels( MAXLEVEL );
/*N*/ //            pFmt->eType = ARABIC;
/*N*/ 			pFmt->SetStart( 1 );
/*N*/ 			SwNumRule::aBaseFmts[ OUTLINE_RULE ][ n ] = pFmt;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	memset( aFmts, 0, sizeof( aFmts ));
/*N*/ 	ASSERT( sName.Len(), "NumRule ohne Namen!" );
/*N*/ }

/*N*/ SwNumRule::SwNumRule( const SwNumRule& rNumRule )
/*N*/ 	: eRuleType( rNumRule.eRuleType ),
/*N*/ 	sName( rNumRule.sName ),
/*N*/ 	bAutoRuleFlag( rNumRule.bAutoRuleFlag ),
/*N*/ 	bInvalidRuleFlag( TRUE ),
/*N*/ 	bContinusNum( rNumRule.bContinusNum ),
/*N*/ 	bAbsSpaces( rNumRule.bAbsSpaces ),
/*N*/ 	nPoolFmtId( rNumRule.GetPoolFmtId() ),
/*N*/ 	nPoolHelpId( rNumRule.GetPoolHelpId() ),
/*N*/ 	nPoolHlpFileId( rNumRule.GetPoolHlpFileId() )
/*N*/ {
/*N*/ 	++nRefCount;
/*N*/ 	memset( aFmts, 0, sizeof( aFmts ));
/*N*/ 	for( USHORT n = 0; n < MAXLEVEL; ++n )
/*N*/ 		if( rNumRule.aFmts[ n ] )
/*N*/ 			Set( n, *rNumRule.aFmts[ n ] );
/*N*/ }
/*N*/ 
/*N*/ SwNumRule::~SwNumRule()
/*N*/ {
/*N*/ 	for( USHORT n = 0; n < MAXLEVEL; ++n )
/*N*/ 		delete aFmts[ n ];
/*N*/ 
/*N*/ 	if( !--nRefCount )			// der letzte macht die Tuer zu
/*N*/ 	{
/*N*/ 			// Nummerierung:
/*N*/ 			SwNumFmt** ppFmts = (SwNumFmt**)SwNumRule::aBaseFmts;
                int n=0;
/*N*/ 			for( n = 0; n < MAXLEVEL; ++n, ++ppFmts )
/*N*/ 				delete *ppFmts, *ppFmts = 0;
/*N*/ 
/*N*/ 			// Gliederung:
/*N*/ 			for( n = 0; n < MAXLEVEL; ++n, ++ppFmts )
/*N*/ 				delete *ppFmts, *ppFmts = 0;
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ 
/*N*/ void SwNumRule::_MakeDefBulletFont()
/*N*/ {
/*N*/ 	pDefBulletFont = new Font( String::CreateFromAscii( sBulletFntName ),
/*N*/ 								aEmptyStr, Size( 0, 14 ) );
/*N*/ 	pDefBulletFont->SetCharSet( RTL_TEXTENCODING_SYMBOL );
/*N*/ 	pDefBulletFont->SetFamily( FAMILY_DONTKNOW );
/*N*/ 	pDefBulletFont->SetPitch( PITCH_DONTKNOW );
/*N*/ 	pDefBulletFont->SetWeight( WEIGHT_DONTKNOW );
/*N*/ 	pDefBulletFont->SetTransparent( TRUE );
/*N*/ }
/*N*/ 
/*N*/ 
/*N*/ void SwNumRule::CheckCharFmts( SwDoc* pDoc )
/*N*/ {
/*N*/ 	SwCharFmt* pFmt;
/*N*/ 	for( BYTE n = 0; n < MAXLEVEL; ++n )
/*N*/ 		if( aFmts[ n ] && 0 != ( pFmt = aFmts[ n ]->GetCharFmt() ) &&
/*N*/ 			pFmt->GetDoc() != pDoc )
/*N*/ 		{
/*N*/ 			// dann kopieren!
/*N*/ 			SwNumFmt* pNew = new SwNumFmt( *aFmts[ n ] );
/*N*/ 			pNew->SetCharFmt( pDoc->CopyCharFmt( *pFmt ) );
/*N*/ 			delete aFmts[ n ];
/*N*/ 			aFmts[ n ] = pNew;
/*N*/ 		}
/*N*/ }

    // setzt Num oder NoNum fuer den Level am TextNode UND setzt die
    // richtige Attributierung


/*N*/ SwNumRule& SwNumRule::operator=( const SwNumRule& rNumRule )
/*N*/ {
/*N*/ 	if( this != &rNumRule )
/*N*/ 	{
/*N*/ 		for( USHORT n = 0; n < MAXLEVEL; ++n )
/*N*/ 			Set( n, rNumRule.aFmts[ n ] );
/*N*/ 
/*N*/ 		eRuleType = rNumRule.eRuleType;
/*N*/ 		sName = rNumRule.sName;
/*N*/ 		bAutoRuleFlag = rNumRule.bAutoRuleFlag;
/*N*/ 		bInvalidRuleFlag = TRUE;
/*N*/ 		bContinusNum = rNumRule.bContinusNum;
/*N*/ 		bAbsSpaces = rNumRule.bAbsSpaces;
/*N*/ 		nPoolFmtId = rNumRule.GetPoolFmtId();
/*N*/ 		nPoolHelpId = rNumRule.GetPoolHelpId();
/*N*/ 		nPoolHlpFileId = rNumRule.GetPoolHlpFileId();
/*N*/ 	}
/*N*/ 	return *this;
/*N*/ }


/*N*/ BOOL SwNumRule::operator==( const SwNumRule& rRule ) const
/*N*/ {
/*N*/ 	BOOL bRet = eRuleType == rRule.eRuleType &&
/*N*/ 				sName == rRule.sName &&
/*N*/ 				bAutoRuleFlag == rRule.bAutoRuleFlag &&
/*N*/ 				bContinusNum == rRule.bContinusNum &&
/*N*/ 				bAbsSpaces == rRule.bAbsSpaces &&
/*N*/ 				nPoolFmtId == rRule.GetPoolFmtId() &&
/*N*/ 				nPoolHelpId == rRule.GetPoolHelpId() &&
/*N*/ 				nPoolHlpFileId == rRule.GetPoolHlpFileId();
/*N*/ 	if( bRet )
/*N*/ 	{
/*N*/ 		for( BYTE n = 0; n < MAXLEVEL; ++n )
/*N*/ 			if( !( rRule.Get( n ) == Get( n ) ))
/*N*/ 			{
/*N*/ 				bRet = FALSE;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*N*/ void SwNumRule::Set( USHORT i, const SwNumFmt& rNumFmt )
/*N*/ {
/*N*/ 	if( !aFmts[ i ] || !(rNumFmt == Get( i )) )
/*N*/ 	{
/*N*/ 		delete aFmts[ i ];
/*N*/ 		aFmts[ i ] = new SwNumFmt( rNumFmt );
/*N*/ 		bInvalidRuleFlag = TRUE;
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ void SwNumRule::Set( USHORT i, const SwNumFmt* pNumFmt )
/*N*/ {
/*N*/ 	SwNumFmt* pOld = aFmts[ i ];
/*N*/ 	if( !pOld )
/*N*/ 	{
/*N*/ 		if( pNumFmt )
/*N*/ 		{
/*N*/ 			aFmts[ i ] = new SwNumFmt( *pNumFmt );
/*N*/ 			bInvalidRuleFlag = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if( !pNumFmt )
/*?*/ 		delete pOld, aFmts[ i ] = 0, bInvalidRuleFlag = TRUE;
/*N*/ 	else if( *pOld != *pNumFmt )
/*N*/ 		*pOld = *pNumFmt, bInvalidRuleFlag = TRUE;
/*N*/ }
/*N*/ 
/*N*/ 
/*N*/ String SwNumRule::MakeNumString( const SwNodeNum& rNum, BOOL bInclStrings,
/*N*/ 								BOOL bOnlyArabic ) const
/*N*/ {
/*N*/ 	String aStr;
/*N*/ 	if( NO_NUM > rNum.GetLevel() && !( NO_NUMLEVEL & rNum.GetLevel() ) )
/*N*/ 	{
/*N*/ 		const SwNumFmt& rMyNFmt = Get( rNum.GetLevel() );
/*N*/ 		if( SVX_NUM_NUMBER_NONE != rMyNFmt.GetNumberingType() )
/*N*/ 		{
/*N*/ 			BYTE i = rNum.GetLevel();
/*N*/ 
/*N*/ 			if( !IsContinusNum() &&
/*N*/ 				rMyNFmt.GetIncludeUpperLevels() )		// nur der eigene Level ?
/*N*/ 			{
/*N*/ 				BYTE n = rMyNFmt.GetIncludeUpperLevels();
/*N*/ 				if( 1 < n )
/*N*/ 				{
/*N*/ 					if( i+1 >= n )
/*N*/ 						i -= n - 1;
/*N*/ 					else
/*N*/ 						i = 0;
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			for( ; i <= rNum.GetLevel(); ++i )
/*N*/ 			{
/*N*/ 				const SwNumFmt& rNFmt = Get( i );
/*N*/ 				if( SVX_NUM_NUMBER_NONE == rNFmt.GetNumberingType() )
/*N*/ 				{
/*N*/ 	// Soll aus 1.1.1 --> 2. NoNum --> 1..1 oder 1.1 ??
/*N*/ 	//                 if( i != rNum.nMyLevel )
/*N*/ 	//                    aStr += aDotStr;
/*N*/ 					continue;
/*N*/ 				}
/*N*/ 
/*N*/ 				if( rNum.GetLevelVal()[ i ] )
/*N*/ 				{
/*N*/ 					if( bOnlyArabic )
/*?*/ 						aStr += String::CreateFromInt32( rNum.GetLevelVal()[ i ] );
/*N*/ 					else
/*N*/ 						aStr += rNFmt.GetNumStr( rNum.GetLevelVal()[ i ] );
/*N*/ 				}
/*N*/ 				else
/*N*/ 					aStr += '0';		// alle 0-Level sind eine 0
/*N*/ 				if( i != rNum.GetLevel() && aStr.Len() )
/*N*/ 					aStr += aDotStr;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		//JP 14.12.99: the type dont have any number, so dont append
/*N*/ 		//				the Post-/Prefix String
/*N*/ 		if( bInclStrings && !bOnlyArabic &&
/*N*/ 			SVX_NUM_CHAR_SPECIAL != rMyNFmt.GetNumberingType() &&
/*N*/ 			SVX_NUM_BITMAP != rMyNFmt.GetNumberingType() )
/*N*/ 		{
/*N*/ 			aStr.Insert( rMyNFmt.GetPrefix(), 0 );
/*N*/ 			aStr += rMyNFmt.GetSuffix();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return aStr;
/*N*/ }

//  ----- Copy-Methode vom SwNumRule ------

    // eine Art Copy-Constructor, damit die Num-Formate auch an den
    // richtigen CharFormaten eines Dokumentes haengen !!
    // (Kopiert die NumFormate und returnt sich selbst)

/* -----------------30.10.98 08:33-------------------
 *
 * --------------------------------------------------*/
/*N*/ void SwNumRule::SetSvxRule(const SvxNumRule& rNumRule, SwDoc* pDoc)
/*N*/ {
/*N*/ 	for( USHORT n = 0; n < MAXLEVEL; ++n )
/*N*/ 	{
/*N*/ 		const SvxNumberFormat* pSvxFmt = rNumRule.Get(n);
/*N*/ 		delete aFmts[n];
/*N*/ 		aFmts[n] = pSvxFmt ? new SwNumFmt(*pSvxFmt, pDoc) : 0;
/*N*/ 	}

//	eRuleType = rNumRule.eRuleType;
//	sName = rNumRule.sName;
//	bAutoRuleFlag = rNumRule.bAutoRuleFlag;
/*N*/ 	bInvalidRuleFlag = TRUE;
/*N*/ 	bContinusNum = rNumRule.IsContinuousNumbering();
//!!!	bAbsSpaces = rNumRule.IsAbsSpaces();
/*N*/ }
/* -----------------30.10.98 08:33-------------------
 *
 * --------------------------------------------------*/
/*N*/ SvxNumRule SwNumRule::MakeSvxNumRule() const
/*N*/ {
/*N*/ 	SvxNumRule aRule(NUM_CONTINUOUS|NUM_CHAR_TEXT_DISTANCE|NUM_CHAR_STYLE|
/*N*/ 						NUM_ENABLE_LINKED_BMP|NUM_ENABLE_EMBEDDED_BMP,
/*N*/ 						MAXLEVEL,
/*N*/ 						eRuleType ==
/*N*/ 							NUM_RULE ?
/*N*/ 								SVX_RULETYPE_NUMBERING :
/*N*/ 									SVX_RULETYPE_OUTLINE_NUMBERING );
/*N*/ 	aRule.SetContinuousNumbering(bContinusNum);
/*N*/ //!!!	aRule.SetAbsSpaces( bAbsSpaces );
/*N*/ 	for( USHORT n = 0; n < MAXLEVEL; ++n )
/*N*/ 	{
/*N*/         SwNumFmt aNumFmt = Get(n);
/*N*/         if(aNumFmt.GetCharFmt())
/*N*/             aNumFmt.SetCharFmtName(aNumFmt.GetCharFmt()->GetName());
/*N*/         aRule.SetLevel(n, aNumFmt, aFmts[n] != 0);
/*N*/ 	}
/*N*/ 	return aRule;
/*N*/ }

/* #109308# */
void SwNumRule::SetNumAdjust(SvxAdjust eNumAdjust)
{
    for (int i = 0; i < MAXLEVEL; i++)
        aFmts[i]->SetNumAdjust(eNumAdjust);
}

}
