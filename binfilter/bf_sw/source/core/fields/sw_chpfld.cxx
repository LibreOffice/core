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

#include <com/sun/star/text/ChapterFormat.hpp>

#include <horiornt.hxx>

#include <doc.hxx>
#include <frame.hxx>		// SwChapterFieldType::ChangeExpansion()
#include <ndtxt.hxx>
#include <chpfld.hxx>
#include <expfld.hxx>		// fuer GetBodyTxtNode
#include <unofldmid.h>
namespace binfilter {

using namespace ::com::sun::star;

/*--------------------------------------------------------------------
    Beschreibung: SwChapterFieldType
 --------------------------------------------------------------------*/


/*N*/ SwChapterFieldType::SwChapterFieldType()
/*N*/ 	: SwFieldType( RES_CHAPTERFLD )
/*N*/ {
/*N*/ }


SwFieldType* SwChapterFieldType::Copy() const
{
    return new SwChapterFieldType();
}


/*--------------------------------------------------------------------
    Beschreibung: Kapittelfeld
 --------------------------------------------------------------------*/


/*N*/ SwChapterField::SwChapterField(SwChapterFieldType* pTyp, sal_uInt32 nFmt)
/*N*/ 	: SwField(pTyp, nFmt), nLevel( 0 )
/*N*/ {}


/*N*/ String SwChapterField::Expand() const
/*N*/ {
/*N*/ 	String sStr( sNumber );
/*N*/ 	switch( GetFormat() )
/*N*/ 	{
/*N*/ 		case CF_TITLE:		sStr = sTitle;	break;
/*N*/ 
/*N*/ 		case CF_NUMBER:
/*N*/ 		case CF_NUM_TITLE:  sStr.Insert( sPre, 0 );
/*N*/ 							sStr += sPost;
/*N*/ 							if( CF_NUM_TITLE == GetFormat() )
/*N*/ 								sStr += sTitle;
/*N*/ 							break;
/*N*/ 
/*?*/ 		case CF_NUM_NOPREPST_TITLE:	sStr += sTitle;	break;
/*N*/ 	}
/*N*/ 	return sStr;
/*N*/ }


/*N*/ SwField* SwChapterField::Copy() const
/*N*/ {
/*N*/ 	SwChapterField *pTmp =
/*N*/ 		new SwChapterField((SwChapterFieldType*)GetTyp(), GetFormat());
/*N*/ 	pTmp->nLevel = nLevel;
/*N*/ 	pTmp->sTitle = sTitle;
/*N*/ 	pTmp->sNumber = sNumber;
/*N*/ 	pTmp->sPost = sPost;
/*N*/ 	pTmp->sPre = sPre;
/*N*/ 
/*N*/ 	return pTmp;
/*N*/ }


/*N*/ void SwChapterField::ChangeExpansion( const SwFrm* pFrm,
/*N*/ 									  const SwTxtNode* pTxtNd,
/*N*/ 									  sal_Bool bSrchNum )
/*N*/ {
/*N*/ 	ASSERT( pFrm, "in welchem Frame stehe ich denn?" )
/*N*/ 	SwDoc* pDoc = (SwDoc*)pTxtNd->GetDoc();
/*N*/ 	SwPosition aPos( pDoc->GetNodes().GetEndOfContent() );
/*N*/ 
/*N*/ 	if( pFrm->IsInDocBody() )
/*N*/ 		aPos.nNode = *pTxtNd;
/*N*/ 	else if( 0 == (pTxtNd = GetBodyTxtNode( *pDoc, aPos, *pFrm )) )
/*N*/ 		// kein TxtNode (Formatierung Kopf/Fusszeile)
/*?*/ 		return;
/*N*/ 
/*N*/ 	ASSERT( pTxtNd, "Wo steht das Feld" );
/*N*/ 	pTxtNd = pTxtNd->FindOutlineNodeOfLevel( nLevel );
/*N*/ 	if( pTxtNd )
/*N*/ 	{
/*N*/ 		if( bSrchNum )
/*N*/ 		{
/*N*/ 			const SwTxtNode* pONd = pTxtNd;
/*N*/ 			do {
/*N*/ 				if( pONd && pONd->GetTxtColl() )
/*N*/ 				{
/*N*/ 					BYTE nPrevLvl = nLevel;
/*N*/ 					nLevel = GetRealLevel( pONd->GetTxtColl()->
/*N*/ 											GetOutlineLevel() );
/*N*/ 					if( nPrevLvl < nLevel )
/*?*/ 						nLevel = nPrevLvl;
/*N*/ 					else if( SVX_NUM_NUMBER_NONE != pDoc->GetOutlineNumRule()
/*N*/ 							->Get( nLevel ).GetNumberingType() )
/*N*/ 					{
/*?*/ 						pTxtNd = pONd;
/*?*/ 						break;
/*N*/ 					}
/*N*/ 
/*N*/ 					if( !nLevel-- )
/*N*/ 						break;
/*N*/ 					pONd = pTxtNd->FindOutlineNodeOfLevel( nLevel );
/*N*/ 				}
/*N*/ 				else
/*?*/ 					break;
/*N*/ 			} while( sal_True );
/*N*/ 		}
/*N*/ 
/*N*/ 		const SwNodeNum& rNum = *pTxtNd->GetOutlineNum();
/*N*/ 		// nur die Nummer besorgen, ohne Pre-/Post-fixstrings
/*N*/ 		sNumber = pDoc->GetOutlineNumRule()->MakeNumString( rNum, sal_False );
/*N*/ 
/*N*/ 		if( NO_NUM > rNum.GetLevel() && !( NO_NUMLEVEL & rNum.GetLevel() ) )
/*N*/ 		{
/*N*/ 			const SwNumFmt& rNFmt = pDoc->GetOutlineNumRule()->Get( rNum.GetLevel() );
/*N*/ 			sPost = rNFmt.GetSuffix();
/*N*/ 			sPre = rNFmt.GetPrefix();
/*N*/ 		}
/*N*/ 		else
/*?*/ 			sPost = aEmptyStr, sPre = aEmptyStr;
/*N*/ 
/*N*/ 		sTitle = pTxtNd->GetExpandTxt();
/*N*/ 
/*N*/ 		for( xub_StrLen i = 0; i < sTitle.Len(); ++i )
/*N*/ 			if( ' ' > sTitle.GetChar( i ) )
/*?*/ 				sTitle.Erase( i--, 1 );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		sNumber = aEmptyStr;
/*?*/ 		sTitle = aEmptyStr;
/*?*/ 		sPost = aEmptyStr;
/*?*/ 		sPre = aEmptyStr;
/*N*/ 	}
/*N*/ }

/*-----------------05.03.98 16:19-------------------

--------------------------------------------------*/
/*N*/ BOOL SwChapterField::QueryValue( ::com::sun::star::uno::Any& rAny, BYTE nMId ) const
/*N*/ {
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_BYTE1:
/*N*/ 		rAny <<= (sal_Int8)nLevel;
/*N*/ 		break;
/*N*/ 
/*N*/ 	case FIELD_PROP_USHORT1:
/*N*/ 		{
/*N*/ 			sal_Int16 nRet;
/*N*/ 			switch( GetFormat() )
/*N*/ 			{
/*N*/ 				case CF_NUMBER:	nRet = text::ChapterFormat::NUMBER; break;
/*N*/ 				case CF_TITLE:  nRet = text::ChapterFormat::NAME; break;
/*N*/ 				case CF_NUMBER_NOPREPST:
/*N*/ 					nRet = text::ChapterFormat::DIGIT;
/*N*/ 				break;
/*N*/ 				case CF_NUM_NOPREPST_TITLE:
/*?*/ 					nRet = text::ChapterFormat::NO_PREFIX_SUFFIX;
/*?*/ 				break;
/*N*/ 				case CF_NUM_TITLE:
/*N*/ 				default:		nRet = text::ChapterFormat::NAME_NUMBER;
/*N*/ 			}
/*N*/ 			rAny <<= nRet;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*N*/ 	}
/*N*/ 	return sal_True;
/*N*/ }
/*-----------------05.03.98 16:19-------------------

--------------------------------------------------*/
/*N*/ BOOL SwChapterField::PutValue( const ::com::sun::star::uno::Any& rAny, BYTE nMId )
/*N*/ {
/*N*/ 	BOOL bRet = TRUE;
/*N*/     nMId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMId )
/*N*/ 	{
/*N*/ 	case FIELD_PROP_BYTE1:
/*?*/ 		sal_Int8 nTmp;
/*?*/ 		rAny >>= nTmp;
/*?*/ 		if(nTmp >= 0 && nTmp < MAXLEVEL)
/*?*/ 			nLevel = nTmp;
/*?*/ 		else
/*?*/ 			bRet = FALSE;
/*?*/ 		break;
/*N*/ 
/*N*/ 	case FIELD_PROP_USHORT1:
/*N*/ 		{
/*N*/ 			sal_Int16 nVal;
/*N*/ 			rAny >>= nVal;
/*N*/ 			switch( nVal )
/*N*/ 			{
/*N*/ 				case text::ChapterFormat::NAME:	SetFormat(CF_TITLE); break;
/*N*/ 				case text::ChapterFormat::NUMBER:  SetFormat(CF_NUMBER); break;
/*N*/ 				case text::ChapterFormat::NO_PREFIX_SUFFIX:
/*?*/ 							SetFormat(CF_NUM_NOPREPST_TITLE);
/*?*/ 				break;
/*N*/ 				case text::ChapterFormat::DIGIT:
/*N*/ 						SetFormat(CF_NUMBER_NOPREPST);
/*N*/ 				break;
/*N*/ 				//case text::ChapterFormat::NAME_NUMBER:
/*N*/ 				default:		SetFormat(CF_NUM_TITLE);
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 	default:
/*?*/ 		DBG_ERROR("illegal property");
/*?*/ 		bRet = FALSE;
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
