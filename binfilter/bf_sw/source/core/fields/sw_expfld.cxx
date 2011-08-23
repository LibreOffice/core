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

#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <bf_svx/unolingu.hxx>
#endif
#ifndef _SVX_PAGEITEM_HXX
#include <bf_svx/pageitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <bf_svx/langitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <bf_svx/fontitem.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_SETVARIABLETYPE_HPP_
#include <com/sun/star/text/SetVariableType.hpp>
#endif


#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _UNOFIELD_HXX
#include <unofield.hxx>
#endif
#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX
#include <txtfld.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _TXTFTN_HXX
#include <txtftn.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _FTNFRM_HXX
#include <ftnfrm.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _USRFLD_HXX
#include <usrfld.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _DOCFLD_HXX
#include <docfld.hxx>
#endif
#ifndef _SWCACHE_HXX
#include <swcache.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#ifndef _UNOFLDMID_H
#include <unofldmid.h>
#endif
namespace binfilter {
extern String& GetString( const ::com::sun::star::uno::Any& rAny, String& rStr ); //STRIP008
using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::rtl;

/*N*/ void InsertSort( SvUShorts& rArr, USHORT nIdx )
/*N*/ {
/*N*/ 	register USHORT nO	= rArr.Count(), nM, nU = 0;
/*N*/ 	if( nO > 0 )
/*N*/ 	{
/*N*/ 		nO--;
/*N*/ 		while( nU <= nO )
/*N*/ 		{
/*N*/ 			nM = nU + ( nO - nU ) / 2;
/*N*/ 			if( *(rArr.GetData() + nM) == nIdx )
/*N*/ 			{
/*N*/ 				ASSERT( FALSE, "Index ist schon vorhanden, darf nie sein!" );
/*N*/ 				return;
/*N*/ 			}
/*N*/ 			if( *(rArr.GetData() + nM) < nIdx )
/*N*/ 				nU = nM + 1;
/*N*/ 			else if( nM == 0 )
/*N*/ 				break;
/*N*/ 			else
/*N*/ 				nO = nM - 1;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	rArr.Insert( nIdx, nU );
/*N*/ }

//-----------------------------------------------------------------------------
/*N*/ sal_Int16 lcl_SubTypeToAPI(USHORT nSubType)
/*N*/ {
/*N*/ 		sal_Int16 nRet = -1;
/*N*/ 		switch(nSubType)
/*N*/ 		{
/*N*/ 			case GSE_EXPR	:  	nRet = SetVariableType::VAR /*0*/; break;
/*N*/ 			case GSE_SEQ	:	nRet = SetVariableType::SEQUENCE /*1*/; break;
/*N*/ 			case GSE_FORMULA:	nRet = SetVariableType::FORMULA /*2*/; break;
/*N*/ 			case GSE_STRING : 	nRet = SetVariableType::STRING /*3*/; break;
/*N*/ 		}
/*N*/ 		return nRet;
/*N*/ }
//-----------------------------------------------------------------------------
/*N*/ sal_Int32 lcl_APIToSubType(const uno::Any& rAny)
/*N*/ {
/*N*/ 		sal_Int16 nVal;
/*N*/ 		rAny >>= nVal;
/*N*/ 		sal_Int32 nSet = 0;
/*N*/ 		switch(nVal)
/*N*/ 		{
/*N*/ 			case SetVariableType::VAR:		nSet = GSE_EXPR;  break;
/*N*/ 			case SetVariableType::SEQUENCE: nSet = GSE_SEQ;  break;
/*?*/ 			case SetVariableType::FORMULA:  nSet = GSE_FORMULA; break;
/*N*/ 			case SetVariableType::STRING:	nSet = GSE_STRING;	break;
/*N*/ 			default:
/*?*/ 				DBG_ERROR("wrong value");
/*?*/ 				nSet = -1;
/*N*/ 		}
/*N*/ 		return nSet;
/*N*/ }
//-----------------------------------------------------------------------------

/*N*/ void ReplacePoint( String& rTmpName, BOOL bWithCommandType )
/*N*/ {
/*N*/     // replace first and last (if bWithCommandType: last two) dot Ersten und letzten Punkt ersetzen, da in Tabellennamen Punkte erlaubt sind
/*N*/     // since table names may contain dots
/*N*/
/*N*/ 	xub_StrLen nLen = rTmpName.Len();
/*N*/ 	sal_Unicode *pStr = rTmpName.GetBufferAccess(), *pBackStr = pStr + nLen;
/*N*/
/*N*/     long nBackCount = bWithCommandType ? 2 : 1;
            xub_StrLen i;
/*N*/     for( i = nLen; i; --i, pBackStr-- )
/*N*/ 		if( '.' == *pBackStr )
/*N*/ 		{
/*?*/ 			*pBackStr = DB_DELIM;
/*?*/             if(!--nBackCount)
/*?*/                 break;
/*N*/ 		}
/*N*/ 	for( i = 0; i < nLen; ++i, ++pStr )
/*N*/ 		if( '.' == *pStr )
/*N*/ 		{
/*?*/ 			*pStr = DB_DELIM;
/*?*/ 			break;
/*N*/ 		}
/*N*/ }

/*N*/ SwTxtNode* GetFirstTxtNode( const SwDoc& rDoc, SwPosition& rPos,
/*N*/ 							const SwCntntFrm *pCFrm, Point &rPt )
/*N*/ {
/*N*/ 	SwTxtNode* pTxtNode;
/*N*/ 	if ( !pCFrm )
/*N*/ 	{
/*?*/ 		rPos.nNode = *rDoc.GetNodes().GetEndOfContent().StartOfSectionNode();
/*?*/ 		SwCntntNode* pCNd;
/*?*/ 		while( 0 != (pCNd = rDoc.GetNodes().GoNext( &rPos.nNode ) ) &&
/*?*/ 				0 == ( pTxtNode = pCNd->GetTxtNode() ) )
/*?*/ 						;
/*?*/ 		ASSERT( pTxtNode, "wo ist der 1.TextNode" );
/*?*/ 		rPos.nContent.Assign( pTxtNode, 0 );
/*N*/ 	}
/*N*/ 	else if ( !pCFrm->IsValid() )
/*N*/ 	{
/*N*/ 		pTxtNode = (SwTxtNode*)pCFrm->GetNode();
/*N*/ 		rPos.nNode = *pTxtNode;
/*N*/ 		rPos.nContent.Assign( pTxtNode, 0 );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pCFrm->GetCrsrOfst( &rPos, rPt );
/*N*/ 		pTxtNode = rPos.nNode.GetNode().GetTxtNode();
/*N*/ 	}
/*N*/ 	return pTxtNode;
/*N*/ }

/*N*/ const SwTxtNode* GetBodyTxtNode( const SwDoc& rDoc, SwPosition& rPos,
/*N*/ 								const SwFrm& rFrm )
/*N*/ {
/*N*/ 	const SwLayoutFrm* pLayout = (SwLayoutFrm*)rFrm.GetUpper();
/*N*/ 	const SwTxtNode* pTxtNode = 0;
/*N*/
/*N*/ 	while( pLayout )
/*N*/ 	{
/*N*/ 		if( pLayout->IsFlyFrm() )
/*N*/ 		{
/*N*/ 			// hole das FlyFormat
/*N*/ 			SwFrmFmt* pFlyFmt = ((SwFlyFrm*)pLayout)->GetFmt();
/*N*/ 			ASSERT( pFlyFmt, "kein FlyFormat gefunden, wo steht das Feld" );
/*N*/
/*N*/ 			const SwFmtAnchor &rAnchor = pFlyFmt->GetAnchor();
/*N*/
/*N*/ 			if( FLY_AT_FLY == rAnchor.GetAnchorId() )
/*N*/ 			{
/*N*/ 				// und der Fly muss irgendwo angehaengt sein, also
/*N*/ 				// den befragen
/*?*/ 				pLayout = (SwLayoutFrm*)((SwFlyFrm*)pLayout)->GetAnchor();
/*?*/ 				continue;
/*N*/ 			}
/*N*/ 			else if( FLY_AT_CNTNT == rAnchor.GetAnchorId() ||
/*N*/ 					 FLY_AUTO_CNTNT == rAnchor.GetAnchorId() ||
/*N*/ 					 FLY_IN_CNTNT == rAnchor.GetAnchorId() )
/*N*/ 			{
/*N*/ 				ASSERT( rAnchor.GetCntntAnchor(), "keine gueltige Position" );
/*N*/ 				rPos = *rAnchor.GetCntntAnchor();
/*N*/ 				pTxtNode = rPos.nNode.GetNode().GetTxtNode();
/*N*/ 				if( FLY_AT_CNTNT == rAnchor.GetAnchorId() )
/*?*/               ((SwTxtNode*)pTxtNode)->MakeStartIndex( &rPos.nContent );
/*N*/ // oder doch besser das Ende vom (Anker-)TextNode nehmen ??
/*N*/ //					((SwTxtNode*)pTxtNode)->MakeEndIndex( &rPos.nContent );
/*N*/
/*N*/ 				// noch nicht abbrechen, kann ja auch noch im
/*N*/ 				// Header/Footer/Footnote/Fly stehen !!
/*N*/ 				pLayout = ((SwFlyFrm*)pLayout)->GetAnchor()
/*N*/ 							? ((SwFlyFrm*)pLayout)->GetAnchor()->GetUpper() : 0;
/*N*/ 				continue;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pLayout->FindPageFrm()->GetCntntPosition(
/*N*/ 												pLayout->Frm().Pos(), rPos );
/*N*/ 				pTxtNode = rPos.nNode.GetNode().GetTxtNode();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if( pLayout->IsFtnFrm() )
/*N*/ 		{
/*?*/ 			// hole den Node vom Anker
/*?*/ 			const SwTxtFtn* pFtn = ((SwFtnFrm*)pLayout)->GetAttr();
/*?*/ 			pTxtNode = &pFtn->GetTxtNode();
/*?*/ 			rPos.nNode = *pTxtNode;
/*?*/ 			rPos.nContent = *pFtn->GetStart();
/*N*/ 		}
/*N*/ 		else if( pLayout->IsHeaderFrm() || pLayout->IsFooterFrm() )
/*N*/ 		{
/*N*/ 			const SwCntntFrm* pCntFrm;
/*N*/ 			const SwPageFrm* pPgFrm = pLayout->FindPageFrm();
/*N*/ 			if( pLayout->IsHeaderFrm() )
/*N*/ 			{
/*N*/ 				const SwTabFrm *pTab;
/*N*/ 				if( 0 != ( pCntFrm = pPgFrm->FindFirstBodyCntnt()) &&
/*N*/ 					0 != (pTab = pCntFrm->FindTabFrm()) && pTab->IsFollow() &&
/*N*/ 		   			pTab->GetTable()->IsHeadlineRepeat() &&
/*N*/ 		   			((SwLayoutFrm*)pTab->Lower())->IsAnLower( pCntFrm ))
/*N*/ 				{
/*N*/ 					// take the next line
/*N*/ 					const SwLayoutFrm* pRow = (SwLayoutFrm*)pTab->Lower();
/*N*/ 					pRow = (SwLayoutFrm*)pRow->GetNext();
/*N*/ 					pCntFrm = pRow->ContainsCntnt();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*?*/ 				pCntFrm = pPgFrm->FindLastBodyCntnt();
/*N*/
/*N*/ 			if( pCntFrm )
/*N*/ 			{
/*N*/ 				pTxtNode = pCntFrm->GetNode()->GetTxtNode();
/*N*/ 				rPos.nNode = *pTxtNode;
/*N*/ 				((SwTxtNode*)pTxtNode)->MakeEndIndex( &rPos.nContent );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				Point aPt( pLayout->Frm().Pos() );
/*N*/ 				aPt.Y()++;		// aus dem Header raus
/*N*/ 				pCntFrm = pPgFrm->GetCntntPos( aPt, FALSE, TRUE, FALSE );
/*N*/ 				pTxtNode = GetFirstTxtNode( rDoc, rPos, pCntFrm, aPt );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			pLayout = pLayout->GetUpper();
/*N*/ 			continue;
/*N*/ 		}
/*N*/ 		break;		// gefunden und beende die Schleife
/*N*/ 	}
/*N*/ 	return pTxtNode;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: SwSetExpFieldType by JP
 --------------------------------------------------------------------*/

/*N*/ SwGetExpFieldType::SwGetExpFieldType(SwDoc* pDoc)
/*N*/ 	: SwValueFieldType( pDoc, RES_GETEXPFLD )
/*N*/ {
/*N*/ }

/*N*/ SwFieldType* SwGetExpFieldType::Copy() const
/*N*/ {
            return new SwGetExpFieldType(GetDoc());
/*N*/ }

/*N*/ void SwGetExpFieldType::Modify( SfxPoolItem*, SfxPoolItem* pNew )
/*N*/ {
/*N*/ 	if( pNew && RES_DOCPOS_UPDATE == pNew->Which() )
/*N*/ 		SwModify::Modify( 0, pNew );
/*N*/ 	// sonst nichts weiter expandieren
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: SwGetExpField by JP
 --------------------------------------------------------------------*/

/*N*/ SwGetExpField::SwGetExpField(SwGetExpFieldType* pTyp, const String& rFormel,
/*N*/ 							USHORT nSub, ULONG nFmt)
/*N*/ 	: SwFormulaField( pTyp, nFmt, 0.0 ),
/*N*/ 	nSubType(nSub),
/*N*/ 	bIsInBodyTxt( TRUE )
/*N*/ {
/*N*/ 	SetFormula( rFormel );
/*N*/ }

/*N*/ String SwGetExpField::Expand() const
/*N*/ {
/*N*/ 	if(nSubType & SUB_CMD)
/*N*/ 		return GetFormula();
/*N*/ 	else
/*N*/ 		return sExpand;
/*N*/ }

/*N*/ String SwGetExpField::GetCntnt(BOOL bName) const
/*N*/ {
/*N*/ 	if ( bName )
/*N*/ 	{
/*?*/ 		String aStr( SwFieldType::GetTypeStr( GSE_FORMULA & nSubType
/*?*/ 												? TYP_FORMELFLD
/*?*/ 												: TYP_GETFLD ) );
/*?*/ 		aStr += ' ';
/*?*/ 		aStr += GetFormula();
/*?*/ 		return aStr;
/*N*/ 	}
/*N*/ 	return Expand();
/*N*/ }

/*N*/ SwField* SwGetExpField::Copy() const
/*N*/ {
/*N*/ 	SwGetExpField *pTmp = new SwGetExpField((SwGetExpFieldType*)GetTyp(),
/*N*/ 											GetFormula(), nSubType, GetFormat());
/*N*/ 	pTmp->SetLanguage(GetLanguage());
/*N*/ 	pTmp->SwValueField::SetValue(GetValue());
/*N*/ 	pTmp->sExpand 		= sExpand;
/*N*/ 	pTmp->bIsInBodyTxt 	= bIsInBodyTxt;
/*N*/     pTmp->SetAutomaticLanguage(IsAutomaticLanguage());
/*N*/
/*N*/ 	return pTmp;
/*N*/ }


String SwGetExpField::GetPar2() const
{
    return GetFormula();
}

void SwGetExpField::SetPar2(const String& rStr)
{
    SetFormula(rStr);
}

/*N*/ USHORT SwGetExpField::GetSubType() const
/*N*/ {
/*N*/ 	return nSubType;
/*N*/ }

/*N*/ void SwGetExpField::SetSubType(USHORT nType)
/*N*/ {
/*N*/ 	nSubType = nType;
/*N*/ }

/*N*/ void SwGetExpField::SetLanguage(USHORT nLng)
/*N*/ {
/*N*/ 	if (nSubType & SUB_CMD)
/*N*/ 		SwField::SetLanguage(nLng);
/*N*/ 	else
/*N*/ 		SwValueField::SetLanguage(nLng);
/*N*/ }

/*-----------------07.03.98 16:08-------------------

--------------------------------------------------*/
/*N*/ BOOL SwGetExpField::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_DOUBLE:
/*N*/ 		rAny <<= GetValue();
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		rAny <<= (sal_Int32)GetFormat();
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_USHORT1:
/*?*/ 		 rAny <<= (sal_Int16)nSubType;
/*?*/ 		break;
/*N*/ 	case FIELD_PROP_PAR1:
/*N*/ 	 	rAny <<= OUString( GetFormula() );
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_SUBTYPE:
/*N*/ 		{
/*N*/ 			sal_Int16 nRet = lcl_SubTypeToAPI(GetSubType() & 0xff);
/*N*/ 			rAny <<= nRet;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_BOOL2:
/*N*/ 		{
/*N*/ 			BOOL bTmp = 0 != (nSubType & SUB_CMD);
/*N*/ 			rAny.setValue(&bTmp, ::getBooleanCppuType());
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR4:
/*?*/ 		rAny <<= ::rtl::OUString(GetExpStr());
/*?*/ 		break;
/*?*/ 	default:
/*?*/         return SwField::QueryValue(rAny, nMId);
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }
/*-----------------07.03.98 16:08-------------------

--------------------------------------------------*/
BOOL SwGetExpField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    sal_Int32 nTmp;
    String sTmp;
    switch( nMId )
    {
    case FIELD_PROP_DOUBLE:
        SwValueField::SetValue(*(double*) rAny.getValue());
        break;
    case FIELD_PROP_FORMAT:
        rAny >>= nTmp;
        SetFormat(nTmp);
        break;
    case FIELD_PROP_USHORT1:
         rAny >>= nTmp;
         nSubType = nTmp;
        break;
    case FIELD_PROP_PAR1:
        SetFormula( binfilter::GetString( rAny, sTmp ));
        break;
    case FIELD_PROP_SUBTYPE:
        nTmp = lcl_APIToSubType(rAny);
        if( nTmp >=0 )
            SetSubType( (GetSubType() & 0xff00) | nTmp);
        break;
    case FIELD_PROP_BOOL2:
        if(*(sal_Bool*) rAny.getValue())
            nSubType |= SUB_CMD;
        else
            nSubType &= (~SUB_CMD);
        break;
    case FIELD_PROP_PAR4:
        ChgExpStr(binfilter::GetString( rAny, sTmp ));
        break;
    default:
        return SwField::PutValue(rAny, nMId);
    }
    return TRUE;
}

/*-----------------JP: 17.06.93 -------------------
 Set-Expression-Type
 --------------------------------------------------*/

/*N*/ SwSetExpFieldType::SwSetExpFieldType( SwDoc* pDoc, const String& rName, USHORT nTyp )
/*N*/ 	: SwValueFieldType( pDoc, RES_SETEXPFLD ),
/*N*/ 	sName( rName ),
/*N*/ 	nType(nTyp),
/*N*/ 	cDelim( '.' ), nLevel( UCHAR_MAX ),
/*N*/ 	bDeleted( FALSE ),
/*N*/ 	pOutlChgNd( 0 )
/*N*/ {
/*N*/ 	if( ( GSE_SEQ | GSE_STRING ) & nType )
/*N*/ 		EnableFormat(FALSE);	// Numberformatter nicht einsetzen
/*N*/ }

/*N*/ SwFieldType* SwSetExpFieldType::Copy() const
/*N*/ {
/*N*/ 	SwSetExpFieldType* pNew = new SwSetExpFieldType(GetDoc(), sName, nType);
/*N*/ 	pNew->bDeleted = bDeleted;
/*N*/ 	pNew->cDelim = cDelim;
/*N*/ 	pNew->nLevel = nLevel;
/*N*/
/*N*/ 	return pNew;
/*N*/ }

/*N*/ const String& SwSetExpFieldType::GetName() const
/*N*/ {
/*N*/ 	return sName;
/*N*/ }

void SwSetExpFieldType::Modify( SfxPoolItem*, SfxPoolItem* )
{
    return;     // nicht weiter expandieren
}



/*N*/ USHORT SwSetExpFieldType::SetSeqRefNo( SwSetExpField& rFld )
/*N*/ {
/*N*/ 	if( !GetDepends() || !(GSE_SEQ & nType) )
/*?*/ 		return USHRT_MAX;
/*N*/
/*N*/ 	SvUShorts aArr( 64 );
/*N*/
/*N*/ 	USHORT n;
/*N*/
/*N*/ 	// dann testmal, ob die Nummer schon vergeben ist oder ob eine neue
/*N*/ 	// bestimmt werden muss.
/*N*/ 	SwClientIter aIter( *this );
/*N*/ 	const SwTxtNode* pNd;
/*N*/ 	for( SwFmtFld* pF = (SwFmtFld*)aIter.First( TYPE( SwFmtFld )); pF;
/*N*/ 			pF = (SwFmtFld*)aIter.Next() )
/*N*/ 		if( pF->GetFld() != &rFld && pF->GetTxtFld() &&
/*N*/ 			0 != ( pNd = pF->GetTxtFld()->GetpTxtNode() ) &&
/*N*/ 			pNd->GetNodes().IsDocNodes() )
/*N*/ 			::binfilter::InsertSort( aArr, ((SwSetExpField*)pF->GetFld())->GetSeqNumber() );
/*N*/
/*N*/
/*N*/ 	// teste erstmal ob die Nummer schon vorhanden ist:
/*N*/ 	USHORT nNum = rFld.GetSeqNumber();
/*N*/ 	if( USHRT_MAX != nNum )
/*N*/ 	{
/*N*/ 		for( n = 0; n < aArr.Count(); ++n )
/*N*/ 			if( aArr[ n ] > nNum )
/*?*/ 				return nNum;			// nicht vorhanden -> also benutzen
/*N*/ 			else if( aArr[ n ] == nNum )
/*N*/ 				break;					// schon vorhanden -> neue erzeugen
/*N*/
/*N*/ 		if( n == aArr.Count() )
/*N*/ 			return nNum;			// nicht vorhanden -> also benutzen
/*N*/ 	}
/*N*/
/*N*/ 	// alle Nummern entsprechend geflag, also bestimme die richtige Nummer
/*N*/ 	for( n = 0; n < aArr.Count(); ++n )
/*N*/ 		if( n != aArr[ n ] )
/*?*/ 			break;
/*N*/
/*N*/ 	rFld.SetSeqNumber( n );
/*N*/ 	return n;
/*N*/ }




/* -----------------24.03.99 09:44-------------------
 *
 * --------------------------------------------------*/
/*N*/ BOOL SwSetExpFieldType::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_SUBTYPE:
/*N*/ 		{
/*N*/ 			sal_Int16 nRet = lcl_SubTypeToAPI(GetType());
/*N*/ 			rAny <<= nRet;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR2:
/*?*/ 		rAny <<= OUString(GetDelimiter());
/*?*/ 		break;
/*N*/ 	case FIELD_PROP_SHORT1:
/*N*/ 		{
/*N*/ 			sal_Int8 nRet = nLevel < MAXLEVEL? nLevel : -1;
/*N*/ 			rAny <<= nRet;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL SwSetExpFieldType::PutValue( const uno::Any& rAny, BYTE nMId )
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_SUBTYPE:
/*N*/ 		{
/*N*/ 			sal_Int32 nSet = lcl_APIToSubType(rAny);
/*N*/ 			if(nSet >=0)
/*N*/ 				SetType(nSet);
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR2:
/*N*/ 		{
/*?*/ 			String sTmp;
/*?*/ 			if( ::binfilter::GetString( rAny, sTmp ).Len() )
/*?*/ 				SetDelimiter( sTmp.GetChar( 0 ));
/*?*/ 			else
/*?*/ 				SetDelimiter(' ');
/*?*/ 		}
/*?*/ 		break;
/*N*/ 	case FIELD_PROP_SHORT1:
/*N*/ 		{
/*N*/ 			sal_Int8 nLvl;
/*N*/ 			rAny >>= nLvl;
/*N*/ 			if(nLvl < 0 || nLvl >= MAXLEVEL)
/*N*/ 				SetOutlineLvl(UCHAR_MAX);
/*N*/ 			else
/*?*/ 				SetOutlineLvl(nLvl);
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }



/*--------------------------------------------------------------------
    Beschreibung: SwSetExpField by JP
 --------------------------------------------------------------------*/

/*N*/ SwSetExpField::SwSetExpField(SwSetExpFieldType* pTyp, const String& rFormel,
/*N*/ 										ULONG nFmt)
/*N*/ 	: SwFormulaField( pTyp, nFmt, 0.0 ), nSeqNo( USHRT_MAX ),
/*N*/ 	nSubType(0)
/*N*/ {
/*N*/ 	SetFormula(rFormel);
/*N*/ 	// SubType ignorieren !!!
/*N*/ 	bInput = FALSE;
/*N*/ 	if( IsSequenceFld() )
/*N*/ 	{
/*N*/ 		SwValueField::SetValue(1.0);
/*N*/ 		if( !rFormel.Len() )
/*N*/ 		{
/*N*/ 			String sFormel(rFormel);
/*N*/ 			sFormel += pTyp->GetName();
/*N*/ 			sFormel += '+';
/*N*/ 			sFormel += '1';
/*N*/ 			SetFormula(sFormel);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ String SwSetExpField::Expand() const
/*N*/ {
/*N*/ 	String aStr;
/*N*/ 	if (nSubType & SUB_CMD)
/*N*/ 	{	// Der CommandString ist gefragt
/*?*/ 		aStr = GetTyp()->GetName();
/*?*/ 		aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " = " ));
/*?*/ 		aStr += GetFormula();
/*N*/ 	}
/*N*/ 	else if(!(nSubType & SUB_INVISIBLE))
/*N*/ 	{   // Der Wert ist sichtbar
/*N*/ 		aStr = sExpand;
/*N*/ 	}
/*N*/ 	return aStr;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: liefert den Namen oder den Inhalt
 --------------------------------------------------------------------*/

/*N*/ String SwSetExpField::GetCntnt(BOOL bName) const
/*N*/ {
/*N*/ 	if( bName )
/*N*/ 	{
/*?*/ 		USHORT nStrType;
/*?*/
/*?*/ 		if( IsSequenceFld() )
/*?*/ 			nStrType = TYP_SEQFLD;
/*?*/ 		else if( bInput )
/*?*/ 			nStrType = TYP_SETINPFLD;
/*?*/ 		else
/*?*/ 			nStrType = TYP_SETFLD;
/*?*/
/*?*/ 		String aStr( SwFieldType::GetTypeStr( nStrType ) );
/*?*/ 		aStr += ' ';
/*?*/ 		aStr += GetTyp()->GetName();
/*?*/
/*?*/ 		if( TYP_SEQFLD != nStrType )
/*?*/ 		{
/*?*/ 			// Sequence nicht die Formel ausgeben
/*?*/ 			aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " = " ));
/*?*/ 			aStr += GetFormula();
/*?*/ 		}
/*?*/ 		return aStr;
/*N*/ 	}
/*N*/ 	return Expand();
/*N*/ }

/*N*/ SwField* SwSetExpField::Copy() const
/*N*/ {
/*N*/ 	SwSetExpField *pTmp = new SwSetExpField((SwSetExpFieldType*)GetTyp(),
/*N*/ 											GetFormula(), GetFormat());
/*N*/ 	pTmp->SwValueField::SetValue(GetValue());
/*N*/ 	pTmp->sExpand   	= sExpand;
/*N*/     pTmp->SetAutomaticLanguage(IsAutomaticLanguage());
/*N*/ 	pTmp->SetLanguage(GetLanguage());
/*N*/ 	pTmp->aPText		= aPText;
/*N*/ 	pTmp->bInput		= bInput;
/*N*/ 	pTmp->nSeqNo		= nSeqNo;
/*N*/ 	pTmp->SetSubType(GetSubType());
/*N*/
/*N*/ 	return pTmp;
/*N*/ }

/*N*/ void SwSetExpField::SetSubType(USHORT nSub)
/*N*/ {
/*N*/ 	((SwSetExpFieldType*)GetTyp())->SetType(nSub & 0xff);
/*N*/ 	nSubType = nSub & 0xff00;
/*N*/
/*N*/ 	DBG_ASSERT( (nSub & 0xff) != 3, "SubType ist illegal!" );
/*N*/ }

/*N*/ USHORT SwSetExpField::GetSubType() const
/*N*/ {
/*N*/ 	return ((SwSetExpFieldType*)GetTyp())->GetType() | nSubType;
/*N*/ }

/*N*/ void SwSetExpField::SetValue( const double& rAny )
/*N*/ {
/*N*/ 	SwValueField::SetValue(rAny);
/*N*/
/*N*/ 	if( IsSequenceFld() )
/*N*/ 		sExpand = FormatNumber( (USHORT)GetValue(), GetFormat() );
/*N*/ 	else
/*N*/ 		sExpand = ((SwValueFieldType*)GetTyp())->ExpandValue( rAny,
/*N*/ 												GetFormat(), GetLanguage());
/*N*/ }

/*N*/ void SwGetExpField::SetValue( const double& rAny )
/*N*/ {
/*N*/ 	SwValueField::SetValue(rAny);
/*N*/ 	sExpand = ((SwValueFieldType*)GetTyp())->ExpandValue( rAny, GetFormat(),
/*N*/ 															GetLanguage());
/*N*/ }
/* -----------------14.07.99 12:21-------------------
    Description: Find the index of the reference text
    following the current field
 --------------------------------------------------*/
/*N*/ xub_StrLen SwGetExpField::GetReferenceTextPos( const SwFmtFld& rFmt, SwDoc& rDoc)
/*N*/ {
/*N*/ 	//
/*N*/ 	const SwTxtFld* pTxtFld = rFmt.GetTxtFld();
/*N*/ 	const SwTxtNode& rTxtNode = pTxtFld->GetTxtNode();
/*N*/ 	//
/*N*/ 	xub_StrLen nRet = *pTxtFld->GetStart() + 1;
/*N*/ 	String sNodeText = rTxtNode.GetTxt();
/*N*/ 	sNodeText.Erase(0, nRet);
/*N*/ 	if(sNodeText.Len())
/*N*/ 	{
/*N*/ 		//now check if sNodeText starts with a non-alphanumeric character plus a blank
/*N*/ 		USHORT nSrcpt = pBreakIt->GetRealScriptOfText( sNodeText, 0 );
/*N*/
/*N*/ 		static USHORT nIds[] =
/*N*/ 		{
/*N*/ 			RES_CHRATR_LANGUAGE, RES_CHRATR_LANGUAGE,
/*N*/ 			RES_CHRATR_FONT, RES_CHRATR_FONT,
/*N*/ 			RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_LANGUAGE,
/*N*/ 			RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONT,
/*N*/ 			RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_LANGUAGE,
/*N*/ 			RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONT,
/*N*/ 			0, 0
/*N*/ 		};
/*N*/ 		SwAttrSet aSet(rDoc.GetAttrPool(), nIds);
/*N*/ 		rTxtNode.GetAttr(aSet, nRet, nRet+1);
/*N*/
/*N*/ 		if( RTL_TEXTENCODING_SYMBOL != ((SvxFontItem&)aSet.Get(
/*N*/ 				GetWhichOfScript( RES_CHRATR_FONT, nSrcpt )) ).GetCharSet() )
/*N*/ 		{
/*N*/ 			LanguageType eLang = ((SvxLanguageItem&)aSet.Get(
/*N*/ 				GetWhichOfScript( RES_CHRATR_LANGUAGE, nSrcpt )) ).GetLanguage();
/*N*/ 			CharClass aCC( SvxCreateLocale( eLang ));
/*N*/ 			sal_Unicode c0 = sNodeText.GetChar(0);
/*N*/ 			BOOL bIsAlphaNum = aCC.isAlphaNumeric( sNodeText, 0 );
/*N*/ 			if( !bIsAlphaNum ||
/*N*/ 				(c0 == ' ' || c0 == '\t'))
/*N*/ 			{
/*N*/ 				nRet++;
/*N*/ 				if( sNodeText.Len() > 1 &&
/*N*/ 					(sNodeText.GetChar(1) == ' ' ||
/*N*/ 					 sNodeText.GetChar(1) == '\t'))
/*N*/ 					nRet++;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }


/*--------------------------------------------------------------------
    Beschreibung: Parameter setzen
 --------------------------------------------------------------------*/

/*N*/ const String& SwSetExpField::GetPar1() const
/*N*/ {
/*N*/ 	return ((SwSetExpFieldType*)GetTyp())->GetName();
/*N*/ }

String SwSetExpField::GetPar2() const
{
    USHORT nType = ((SwSetExpFieldType*)GetTyp())->GetType();

    if (nType & GSE_STRING)
        return GetFormula();
    return GetExpandedFormula();
}

void SwSetExpField::SetPar2(const String& rStr)
{
    USHORT nType = ((SwSetExpFieldType*)GetTyp())->GetType();

    if( !(nType & GSE_SEQ) || rStr.Len() )
    {
        if (nType & GSE_STRING)
            SetFormula(rStr);
        else
            SetExpandedFormula(rStr);
    }
}

/*--------------------------------------------------------------------
    Beschreibung: Eingabefeld Type
 ---------------------------------------------------------------------*/

/*N*/ SwInputFieldType::SwInputFieldType( SwDoc* pD )
/*N*/ 	: SwFieldType( RES_INPUTFLD ), pDoc( pD )
/*N*/ {
/*N*/ }

/*N*/ SwFieldType* SwInputFieldType::Copy() const
/*N*/ {
            SwInputFieldType* pType = new SwInputFieldType( pDoc );
            return pType;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: Eingabefeld
 --------------------------------------------------------------------*/

/*N*/ SwInputField::SwInputField(SwInputFieldType* pType, const String& rContent,
/*N*/ 						   const String& rPrompt, USHORT nSub, ULONG nFmt) :
/*N*/ 	SwField(pType, nFmt), nSubType(nSub), aContent(rContent), aPText(rPrompt)
/*N*/ {
/*N*/ }

/*N*/ String SwInputField::GetCntnt(BOOL bName) const
/*N*/ {
/*N*/ 	if ( bName )
/*N*/ 	{
/*?*/ 		String aStr(SwField::GetCntnt(bName));
/*?*/ 		if ((nSubType & 0x00ff) == INP_USR)
/*?*/ 		{
/*?*/ 			aStr += GetTyp()->GetName();
/*?*/ 			aStr += ' ';
/*?*/ 			aStr += aContent;
/*?*/ 		}
/*?*/ 		return aStr;
/*N*/ 	}
/*N*/ 	return Expand();
/*N*/ }

/*N*/ SwField* SwInputField::Copy() const
/*N*/ {
/*N*/ 	SwInputField* pFld = new SwInputField((SwInputFieldType*)GetTyp(), aContent,
/*N*/ 										  aPText, GetSubType(), GetFormat());
/*N*/     pFld->SetAutomaticLanguage(IsAutomaticLanguage());
/*N*/ 	return pFld;
/*N*/ }

/*N*/ String SwInputField::Expand() const
/*N*/ {
/*N*/ 	String sRet;
/*N*/ 	if((nSubType & 0x00ff) == INP_TXT)
/*N*/ 		sRet = aContent;
/*N*/
/*N*/ 	else if( (nSubType & 0x00ff) == INP_USR )
/*N*/ 	{
/*?*/ 		SwUserFieldType* pUserTyp = (SwUserFieldType*)
/*?*/ 							((SwInputFieldType*)GetTyp())->GetDoc()->
/*?*/ 							GetFldType( RES_USERFLD, aContent );
/*?*/ 		if( pUserTyp )
/*?*/ 			sRet = pUserTyp->GetContent();
/*N*/ 	}
/*N*/ 	return sRet;
/*N*/ }

/*-----------------06.03.98 11:12-------------------

--------------------------------------------------*/
/*N*/ BOOL SwInputField::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_PAR1:
/*?*/ 		 rAny <<= OUString( aContent );
/*?*/ 		break;
/*N*/ 	case FIELD_PROP_PAR2:
/*N*/ 		rAny <<= OUString( aPText );
/*N*/ 		break;
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }
/*-----------------06.03.98 11:12-------------------

--------------------------------------------------*/
BOOL SwInputField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        binfilter::GetString( rAny, aContent );
        break;
    case FIELD_PROP_PAR2:
        binfilter::GetString( rAny, aPText );
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return TRUE;
}
/*--------------------------------------------------------------------
    Beschreibung: Bedingung setzen
 --------------------------------------------------------------------*/

void SwInputField::SetPar1(const String& rStr)
{
    aContent = rStr;
}

/*N*/ const String& SwInputField::GetPar1() const
/*N*/ {
/*N*/ 	return aContent;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung: True/False Text
 --------------------------------------------------------------------*/

void SwInputField::SetPar2(const String& rStr)
{
    aPText = rStr;
}

/*N*/ String SwInputField::GetPar2() const
/*N*/ {
/*N*/ 	return aPText;
/*N*/ }

/*N*/ USHORT SwInputField::GetSubType() const
/*N*/ {
/*N*/ 	return nSubType;
/*N*/ }

void SwInputField::SetSubType(USHORT nSub)
{
    nSubType = nSub;
}
/*-----------------05.03.98 17:22-------------------

--------------------------------------------------*/
/*N*/ BOOL SwSetExpField::QueryValue( uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_BOOL2:
/*N*/ 		{
/*N*/ 			sal_Bool bVal = 0 == (nSubType & SUB_INVISIBLE);
/*N*/ 			rAny.setValue(&bVal, ::getBooleanCppuType());
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		rAny <<= (sal_Int32)GetFormat();
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_USHORT2:
/*N*/ 		rAny <<= (sal_Int16)GetFormat();
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_USHORT1:
/*N*/ 		rAny <<= (sal_Int16)nSeqNo;
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR1:
/*N*/ 		rAny <<= OUString ( SwStyleNameMapper::GetProgName(GetPar1(), GET_POOLID_TXTCOLL ) );
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR2:
/*N*/ 		{
/*N*/ 			//I18N - if the formula contains only "TypeName+1"
/*N*/ 			//and it's one of the initially created sequence fields
/*N*/ 			//then the localized names has to be replaced by a programmatic name
/*N*/ 			OUString sFormula = SwXFieldMaster::LocalizeFormula(*this, GetFormula(), TRUE);
/*N*/ 			rAny <<= OUString( sFormula );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_DOUBLE:
/*N*/ 		rAny <<= (double)GetValue();
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_SUBTYPE:
/*N*/ 		{
/*N*/ 			sal_Int16 nRet = 0;
/*N*/ 				nRet = lcl_SubTypeToAPI(GetSubType() & 0xff);
/*N*/ 			rAny <<= nRet;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR3:
/*?*/ 		rAny <<= OUString( aPText );
/*?*/ 		break;
/*?*/ 	case FIELD_PROP_BOOL3:
/*?*/ 		{
/*?*/ 			BOOL bTmp = 0 != (nSubType & SUB_CMD);
/*?*/ 			rAny.setValue(&bTmp, ::getBooleanCppuType());
/*?*/ 		}
/*?*/ 		break;
/*N*/ 	case FIELD_PROP_BOOL1:
/*N*/ 		{
/*N*/ 			BOOL bTmp = GetInputFlag();
/*N*/ 			rAny.setValue(&bTmp, ::getBooleanCppuType());
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR4:
/*?*/ 		rAny <<= ::rtl::OUString(GetExpStr());
/*?*/ 		break;
/*?*/ 	default:
/*?*/         return SwField::QueryValue(rAny, nMId);
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }
/*-----------------05.03.98 17:22-------------------

--------------------------------------------------*/
/*N*/ BOOL SwSetExpField::PutValue( const uno::Any& rAny, BYTE nMId )
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	sal_Int32 nTmp32;
/*N*/ 	sal_Int16 nTmp16;
/*N*/ 	String sTmp;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_BOOL2:
/*?*/ 		if(*(sal_Bool*)rAny.getValue())
/*?*/ 			nSubType &= ~SUB_INVISIBLE;
/*?*/ 		else
/*?*/ 			nSubType |= SUB_INVISIBLE;
/*?*/ 		break;
/*N*/ 	case FIELD_PROP_FORMAT:
/*N*/ 		rAny >>= nTmp32;
/*N*/ 		SetFormat(nTmp32);
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_USHORT2:
/*?*/ 		{
/*?*/ 			rAny >>= nTmp16;
/*?*/ 			if(nTmp16 <= SVX_NUMBER_NONE )
/*?*/ 				SetFormat(nTmp16);
/*?*/ 			else
/*?*/ 				//exception(wrong_value)
/*?*/ 				;
/*?*/ 		}
/*?*/ 		break;
/*?*/ 	case FIELD_PROP_USHORT1:
/*?*/ 		rAny >>= nTmp16;
/*?*/ 		nSeqNo = nTmp16;
/*?*/ 		break;
/*?*/ 	case FIELD_PROP_PAR1:
/*?*/       SetPar1( SwStyleNameMapper::GetUIName(
/*?*/                           ::binfilter::GetString( rAny, sTmp ), GET_POOLID_TXTCOLL ) );
/*?*/ 		break;
/*N*/ 	case FIELD_PROP_PAR2:
/*N*/ 		{
/*N*/ 			OUString uTmp;
/*N*/ 			rAny >>= uTmp;
/*N*/ 			//I18N - if the formula contains only "TypeName+1"
/*N*/ 			//and it's one of the initially created sequence fields
/*N*/ 			//then the localized names has to be replaced by a programmatic name
/*N*/ 			OUString sFormula = SwXFieldMaster::LocalizeFormula(*this, uTmp, FALSE);
/*N*/ 			SetFormula( sFormula );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_DOUBLE:
/*N*/ 		{
/*N*/ 		 	double fVal;
/*N*/ 		 	rAny >>= fVal;
/*N*/ 		 	SetValue(fVal);
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_SUBTYPE:
/*N*/ 		nTmp32 = lcl_APIToSubType(rAny);
/*N*/ 		if(nTmp32 >= 0)
/*N*/ 			SetSubType((GetSubType() & 0xff00) | nTmp32);
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_PAR3:
/*?*/ 		::binfilter::GetString( rAny, aPText );
/*?*/ 		break;
/*N*/ 	case FIELD_PROP_BOOL3:
/*N*/ 		if(*(sal_Bool*) rAny.getValue())
/*?*/ 			nSubType |= SUB_CMD;
/*N*/ 		else
/*N*/ 			nSubType &= (~SUB_CMD);
/*N*/ 		break;
/*N*/ 	case FIELD_PROP_BOOL1:
/*?*/ 		SetInputFlag(*(sal_Bool*) rAny.getValue());
/*?*/ 		break;
/*N*/ 	case FIELD_PROP_PAR4:
/*N*/ 		ChgExpStr( ::binfilter::GetString( rAny, sTmp ));
/*N*/ 		break;
/*N*/ 	default:
/*?*/         return SwField::PutValue(rAny, nMId);
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }



}
