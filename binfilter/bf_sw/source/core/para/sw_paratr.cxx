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

#include "unomid.h"
#include <com/sun/star/style/DropCapFormat.hpp>

#include <cppuhelper/implbase4.hxx>

#include <SwStyleNameMapper.hxx>
#include "errhdl.hxx"
#include "paratr.hxx"
#include "charfmt.hxx"
#include "cmdid.h"
namespace binfilter {

using namespace ::com::sun::star;

/*N*/ TYPEINIT2_AUTOFACTORY( SwFmtDrop, SfxPoolItem, SwClient);
/*N*/ TYPEINIT1_AUTOFACTORY( SwRegisterItem, SfxBoolItem);
/*N*/ TYPEINIT1_AUTOFACTORY( SwNumRuleItem, SfxStringItem);
/*N*/ TYPEINIT1_AUTOFACTORY( SwParaConnectBorderItem, SfxBoolItem);

/*************************************************************************
|*    Beschreibung		Methoden von SwFmtDrop
|*    Ersterstellung	MS  19.02.91
|*    Letzte Aenderung	JP 08.08.94
*************************************************************************/



/*N*/ SwFmtDrop::SwFmtDrop()
/*N*/ 	: SfxPoolItem( RES_PARATR_DROP ),
/*N*/ 	SwClient( 0 ),
/*N*/ 	nLines( 0 ),
/*N*/ 	nChars( 0 ),
/*N*/ 	nDistance( 0 ),
/*N*/ 	pDefinedIn( 0 ),
/*N*/ 	bWholeWord( sal_False ),
/*N*/ 	nReadFmt( USHRT_MAX )
/*N*/ {
/*N*/ }



/*N*/ SwFmtDrop::SwFmtDrop( const SwFmtDrop &rCpy )
/*N*/ 	: SfxPoolItem( RES_PARATR_DROP ),
/*N*/ 	SwClient( rCpy.pRegisteredIn ),
/*N*/ 	nLines( rCpy.GetLines() ),
/*N*/ 	nChars( rCpy.GetChars() ),
/*N*/ 	nDistance( rCpy.GetDistance() ),
/*N*/ 	bWholeWord( rCpy.GetWholeWord() ),
/*N*/ 	pDefinedIn( 0 ),
/*N*/ 	nReadFmt( rCpy.nReadFmt )
/*N*/ {
/*N*/ }



/*N*/ SwFmtDrop::~SwFmtDrop()
/*N*/ {
/*N*/ }



/*N*/ void SwFmtDrop::SetCharFmt( SwCharFmt *pNew )
/*N*/ {
/*N*/ 	//Ummelden
/*N*/ 	if ( pRegisteredIn )
/*?*/ 		pRegisteredIn->Remove( this );
/*N*/     if(pNew)
/*N*/         pNew->Add( this );
/*N*/ 	nReadFmt = USHRT_MAX;
/*N*/ }



/*N*/ void SwFmtDrop::Modify( SfxPoolItem *pA, SfxPoolItem *pB )
/*N*/ {
/*N*/ 	if( pDefinedIn )
/*N*/ 	{
/*N*/ 		if( !pDefinedIn->ISA( SwFmt ))
/*N*/ 			pDefinedIn->Modify( this, this );
/*N*/ 		else if( pDefinedIn->GetDepends() &&
/*N*/ 				!pDefinedIn->IsModifyLocked() )
/*N*/ 		{
/*?*/ 			// selbst den Abhaengigen vom Format bescheid sagen. Das
/*?*/ 			// Format selbst wuerde es nicht weitergeben, weil es ueber
/*?*/ 			// die Abpruefung nicht hinauskommt.
/*?*/ 			SwClientIter aIter( *pDefinedIn );
/*?*/ 			SwClient * pLast = aIter.GoStart();
/*?*/ 			if( pLast ) 	// konnte zum Anfang gesprungen werden ??
/*?*/ 				do {
/*?*/ 					pLast->Modify( this, this );
/*?*/ 					if( !pDefinedIn->GetDepends() )	// Baum schon Weg ??
/*?*/ 						break;
/*?*/ 				} while( 0 != ( pLast = aIter++ ));
/*?*/ 		}
/*N*/ 	}
/*N*/ }



/*N*/ sal_Bool SwFmtDrop::GetInfo( SfxPoolItem& rInfo ) const
/*N*/ {
/*N*/ 	// fuers UNDO: pruefe ob das Attribut wirklich in diesem Format steht
/*N*/ #ifdef USED_30
/*N*/ 	if( pDefinedIn )
/*N*/ 	{
/*N*/ 		if( IS_TYPE( SwTxtNode, pDefinedIn )
/*N*/ 			&& ((SwTxtNode*)pDefinedIn)->....... )
/*N*/ 			;
/*N*/ 		else if( IS_TYPE( SwTxtFmtColl, pDefinedIn )
/*N*/ 			&& ((SwTxtFmtColl*)pDefinedIn)->....... )
/*N*/ 			;
/*N*/ //	 this == pFmt->GetAttr( RES_PARATR_DROP, sal_False ))
/*N*/ //        return pFmt->GetInfo( rInfo );
/*N*/ 
/*N*/ 	}
/*N*/ #endif
/*N*/ 	return sal_True;    // weiter
/*N*/ }



/*N*/ int SwFmtDrop::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
/*N*/ 	return ( nLines == ((SwFmtDrop&)rAttr).GetLines() &&
/*N*/ 			 nChars == ((SwFmtDrop&)rAttr).GetChars() &&
/*N*/ 			 nDistance ==  ((SwFmtDrop&)rAttr).GetDistance() &&
/*N*/ 			 bWholeWord == ((SwFmtDrop&)rAttr).GetWholeWord() &&
/*N*/ 			 GetCharFmt() == ((SwFmtDrop&)rAttr).GetCharFmt() &&
/*N*/ 			 pDefinedIn == ((SwFmtDrop&)rAttr).pDefinedIn );
/*N*/ }



/*N*/ SfxPoolItem* SwFmtDrop::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SwFmtDrop( *this );
/*N*/ }




/*N*/ bool SwFmtDrop::QueryValue( uno::Any& rVal, sal_uInt8 nMemberId ) const
/*N*/ {
/*N*/ 	switch(nMemberId&~CONVERT_TWIPS)
/*N*/ 	{
/*N*/         case MID_DROPCAP_LINES : rVal <<= (sal_Int16)nLines; break;
/*N*/         case MID_DROPCAP_COUNT : rVal <<= (sal_Int16)nChars; break;
/*N*/         case MID_DROPCAP_DISTANCE : rVal <<= (sal_Int16) TWIP_TO_MM100(nDistance); break;
/*N*/ 		case MID_DROPCAP_FORMAT:
/*N*/ 		{
/*N*/ 		 	style::DropCapFormat aDrop;
/*N*/ 			aDrop.Lines	= nLines   ;
/*N*/ 			aDrop.Count	= nChars   ;
/*N*/ 			aDrop.Distance	= TWIP_TO_MM100(nDistance);
/*N*/ 			rVal.setValue(&aDrop, ::getCppuType((const style::DropCapFormat*)0));
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_DROPCAP_WHOLE_WORD:
/*N*/ 			rVal.setValue(&bWholeWord, ::getBooleanCppuType());
/*N*/ 		break;
/*N*/ 		case MID_DROPCAP_CHAR_STYLE_NAME :
/*N*/ 		{
/*N*/ 			::rtl::OUString sName;
/*N*/ 			if(GetCharFmt())
/*N*/ 				sName = SwStyleNameMapper::GetProgName(
/*N*/ 						GetCharFmt()->GetName(), GET_POOLID_CHRFMT );
/*N*/ 			rVal <<= sName;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return true;
/*N*/ }

/*N*/ bool SwFmtDrop::PutValue( const uno::Any& rVal, sal_uInt8 nMemberId )
/*N*/ {
/*N*/ 	switch(nMemberId&~CONVERT_TWIPS)
/*N*/ 	{
/*N*/       case MID_DROPCAP_LINES :
/*N*/       {
/*N*/           sal_Int8 nTemp;
/*N*/           rVal >>= nTemp;
/*N*/           if(nTemp >=1 && nTemp < 0x7f)
/*N*/               nLines = (BYTE)nTemp;
/*N*/       }
/*N*/       break;
/*N*/       case MID_DROPCAP_COUNT :
/*N*/       {
/*N*/           sal_Int16 nTemp;
/*N*/           rVal >>= nTemp;
/*N*/           if(nTemp >=1 && nTemp < 0x7f)
/*N*/               nChars = (BYTE)nTemp;
/*N*/       }
/*N*/       break;
/*N*/       case MID_DROPCAP_DISTANCE :
/*N*/       {
/*N*/           sal_Int16 nVal;
/*N*/           if ( rVal >>= nVal )
/*N*/               nDistance = (sal_Int16) MM100_TO_TWIP((sal_Int32)nVal);
/*N*/           else
/*N*/               return sal_False;
/*N*/           break;
/*N*/       }
/*N*/ 		case MID_DROPCAP_FORMAT:
/*N*/ 		{
/*N*/ 			if(rVal.getValueType()  == ::getCppuType((const style::DropCapFormat*)0))
/*N*/ 			{
/*N*/ 				const style::DropCapFormat* pDrop = (const style::DropCapFormat*)rVal.getValue();
/*N*/ 				nLines 		= pDrop->Lines;
/*N*/ 				nChars 		= pDrop->Count;
/*N*/ 				nDistance 	= MM100_TO_TWIP(pDrop->Distance);
/*N*/ 			}
/*N*/ 			else
/*N*/ 				//exception( wrong_type)
/*N*/ 				;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case MID_DROPCAP_WHOLE_WORD:
/*N*/ 			bWholeWord = *(sal_Bool*)rVal.getValue();
/*N*/ 		break;
/*N*/ 		case MID_DROPCAP_CHAR_STYLE_NAME :
/*N*/ 			DBG_ERROR("char format cannot be set in PutValue()!");
/*N*/ 		break;
/*N*/ 	}
/*N*/ 	return true;
/*N*/ }

// class SwRegisterItem -------------------------------------------------


/*N*/ SfxPoolItem* SwRegisterItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SwRegisterItem( *this );
/*N*/ }

// class SwNumRuleItem -------------------------------------------------
/*N*/ SfxPoolItem* SwNumRuleItem::Clone( SfxItemPool *pPool  ) const
/*N*/ {
/*N*/ 	return new SwNumRuleItem( *this );
/*N*/ }
/* -----------------------------27.06.00 11:05--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ bool SwNumRuleItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	::rtl::OUString sRet = SwStyleNameMapper::GetProgName(GetValue(), GET_POOLID_NUMRULE );
/*N*/ 	rVal <<= sRet;
/*N*/ 	return true;
/*N*/ }
/* -----------------------------27.06.00 11:05--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ bool SwNumRuleItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	::rtl::OUString uName;
/*N*/ 	rVal >>= uName;
/*N*/ 	SetValue(SwStyleNameMapper::GetUIName(uName, GET_POOLID_NUMRULE));
/*N*/ 	return true;
/*N*/ }
/* -----------------19.05.2003 10:44-----------------

 --------------------------------------------------*/
 SfxPoolItem* SwParaConnectBorderItem::Clone( SfxItemPool * ) const
{
    return new SwParaConnectBorderItem( *this );
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
