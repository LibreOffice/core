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

#include <bf_svtools/macitem.hxx>

#include <fchrfmt.hxx>
#include <fmtinfmt.hxx>
#include <txtatr.hxx>
#include <fmtruby.hxx>
#include <charfmt.hxx>

#include <horiornt.hxx>


#include <cppuhelper/implbase4.hxx>

#include <unoevent.hxx>		// SwHyperlinkEventDescriptor
#include <com/sun/star/text/RubyAdjust.hdl>

#include <cmdid.h>
#include <com/sun/star/uno/Any.h>
#include <SwStyleNameMapper.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::rtl;

/*************************************************************************
|*
|*    class SwFmtCharFmt
|*    Beschreibung
|*    Ersterstellung    JP 23.11.90
|*    Letzte Aenderung  JP 09.08.94
|*
*************************************************************************/

/*N*/ TYPEINIT1_AUTOFACTORY(SwFmtINetFmt, SfxPoolItem);

/*N*/ SwFmtCharFmt::SwFmtCharFmt( SwCharFmt *pFmt )
/*N*/ 	: SfxPoolItem( RES_TXTATR_CHARFMT ),
/*N*/ 	SwClient(pFmt),
/*N*/ 	pTxtAttr( 0 )
/*N*/ {
/*N*/ }



/*N*/ SwFmtCharFmt::SwFmtCharFmt( const SwFmtCharFmt& rAttr )
/*N*/ 	: SfxPoolItem( RES_TXTATR_CHARFMT ),
/*N*/ 	SwClient( rAttr.GetCharFmt() ),
/*N*/ 	pTxtAttr( 0 )
/*N*/ {
/*N*/ }



/*N*/ SwFmtCharFmt::~SwFmtCharFmt() {}



/*N*/ int SwFmtCharFmt::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
/*N*/ 	return GetCharFmt() == ((SwFmtCharFmt&)rAttr).GetCharFmt();
/*N*/ }



/*N*/ SfxPoolItem* SwFmtCharFmt::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SwFmtCharFmt( *this );
/*N*/ }






/*N*/ // weiterleiten an das TextAttribut
/*N*/ BOOL SwFmtCharFmt::GetInfo( SfxPoolItem& rInfo ) const
/*N*/ {
/*N*/ 	return pTxtAttr ? pTxtAttr->GetInfo( rInfo ) : FALSE;
/*N*/ }
/*N*/ bool SwFmtCharFmt::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	String sCharFmtName;
/*N*/ 	if(GetCharFmt())
/*N*/ 		SwStyleNameMapper::FillProgName(GetCharFmt()->GetName(), sCharFmtName,  GET_POOLID_CHRFMT, sal_True );
/*N*/ 	rVal <<= OUString( sCharFmtName );
/*N*/ 	return true;
/*N*/ }

/*************************************************************************
|*
|*    class SwFmtINetFmt
|*    Beschreibung
|*    Ersterstellung    AMA 02.08.96
|*    Letzte Aenderung  AMA 02.08.96
|*
*************************************************************************/

/*N*/ SwFmtINetFmt::SwFmtINetFmt()
/*N*/ 	: SfxPoolItem( RES_TXTATR_INETFMT ),
/*N*/ 	pTxtAttr( 0 ),
/*N*/ 	pMacroTbl( 0 ),
/*N*/ 	nINetId( 0 ),
/*N*/ 	nVisitedId( 0 )
/*N*/ {}

/*N*/ SwFmtINetFmt::SwFmtINetFmt( const XubString& rURL, const XubString& rTarget )
/*N*/ 	: SfxPoolItem( RES_TXTATR_INETFMT ),
/*N*/ 	aURL( rURL ),
/*N*/ 	aTargetFrame( rTarget ),
/*N*/ 	pTxtAttr( 0 ),
/*N*/ 	pMacroTbl( 0 ),
/*N*/ 	nINetId( 0 ),
/*N*/ 	nVisitedId( 0 )
/*N*/ {
/*N*/ }



/*N*/ SwFmtINetFmt::SwFmtINetFmt( const SwFmtINetFmt& rAttr )
/*N*/ 	: SfxPoolItem( RES_TXTATR_INETFMT ),
/*N*/ 	aURL( rAttr.GetValue() ),
/*N*/ 	aName( rAttr.aName ),
/*N*/ 	aTargetFrame( rAttr.aTargetFrame ),
/*N*/ 	aINetFmt( rAttr.aINetFmt ),
/*N*/ 	aVisitedFmt( rAttr.aVisitedFmt ),
/*N*/ 	pTxtAttr( 0 ),
/*N*/ 	pMacroTbl( 0 ),
/*N*/ 	nINetId( rAttr.nINetId ),
/*N*/ 	nVisitedId( rAttr.nVisitedId )
/*N*/ {
/*N*/ 	if( rAttr.GetMacroTbl() )
/*?*/ 		pMacroTbl = new SvxMacroTableDtor( *rAttr.GetMacroTbl() );
/*N*/ }



/*N*/ SwFmtINetFmt::~SwFmtINetFmt()
/*N*/ {
/*N*/ 	delete pMacroTbl;
/*N*/ }



/*N*/ int SwFmtINetFmt::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
/*N*/ 	BOOL bRet = SfxPoolItem::operator==( (SfxPoolItem&) rAttr )
/*N*/ 				&& aURL == ((SwFmtINetFmt&)rAttr).aURL
/*N*/ 				&& aName == ((SwFmtINetFmt&)rAttr).aName
/*N*/ 				&& aTargetFrame == ((SwFmtINetFmt&)rAttr).aTargetFrame
/*N*/ 				&& aINetFmt == ((SwFmtINetFmt&)rAttr).aINetFmt
/*N*/ 				&& aVisitedFmt == ((SwFmtINetFmt&)rAttr).aVisitedFmt
/*N*/ 				&& nINetId == ((SwFmtINetFmt&)rAttr).nINetId
/*N*/ 				&& nVisitedId == ((SwFmtINetFmt&)rAttr).nVisitedId;
/*N*/ 
/*N*/ 	if( !bRet )
/*N*/ 		return FALSE;
/*N*/ 
/*N*/ 	const SvxMacroTableDtor* pOther = ((SwFmtINetFmt&)rAttr).pMacroTbl;
/*N*/ 	if( !pMacroTbl )
/*N*/ 		return ( !pOther || !pOther->Count() );
/*N*/ 	if( !pOther )
/*?*/ 		return 0 == pMacroTbl->Count();
/*?*/ 
/*?*/ 	const SvxMacroTableDtor& rOwn = *pMacroTbl;
/*?*/ 	const SvxMacroTableDtor& rOther = *pOther;
/*?*/ 
/*?*/ 	// Anzahl unterschiedlich => auf jeden Fall ungleich
/*?*/ 	if( rOwn.Count() != rOther.Count() )
/*?*/ 		return FALSE;
/*?*/ 
/*?*/ 	// einzeln vergleichen; wegen Performance ist die Reihenfolge wichtig
/*?*/ 	for( USHORT nNo = 0; nNo < rOwn.Count(); ++nNo )
/*?*/ 	{
/*?*/ 		const SvxMacro *pOwnMac = rOwn.GetObject(nNo);
/*?*/ 		const SvxMacro *pOtherMac = rOther.GetObject(nNo);
/*?*/ 		if ( 	rOwn.GetKey(pOwnMac) != rOther.GetKey(pOtherMac)  ||
/*?*/ 				pOwnMac->GetLibName() != pOtherMac->GetLibName() ||
/*?*/ 				pOwnMac->GetMacName() != pOtherMac->GetMacName() )
/*?*/ 			return FALSE;
/*?*/ 	}
/*?*/ 	return TRUE;
/*N*/ }



/*N*/ SfxPoolItem* SwFmtINetFmt::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SwFmtINetFmt( *this );
/*N*/ }






/*N #i27164#*/ void SwFmtINetFmt::SetMacro( USHORT nEvent, const SvxMacro& rMacro )
/*N #i27164#*/ {
/*N #i27164#*/	if( !pMacroTbl )
/*N #i27164#*/	pMacroTbl = new SvxMacroTableDtor;
/*N #i27164#*/ 
/*N #i27164#*/	SvxMacro *pOldMacro;
/*N #i27164#*/	if( 0 != ( pOldMacro = pMacroTbl->Get( nEvent )) )
/*N #i27164#*/	{
/*N #i27164#*/	delete pOldMacro;
/*N #i27164#*/	pMacroTbl->Replace( nEvent, new SvxMacro( rMacro ) );
/*N #i27164#*/	}
/*N #i27164#*/	else
/*N #i27164#*/	pMacroTbl->Insert( nEvent, new SvxMacro( rMacro ) );
/*N #i27164#*/ }



/*N*/ const SvxMacro* SwFmtINetFmt::GetMacro( USHORT nEvent ) const
/*N*/ {
/*N*/ 	const SvxMacro* pRet = 0;
/*N*/ 	if( pMacroTbl && pMacroTbl->IsKeyValid( nEvent ) )
/*?*/ 		pRet = pMacroTbl->Get( nEvent );
/*N*/ 	return pRet;
/*N*/ }



/*N*/ bool SwFmtINetFmt::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	bool bRet = true;
/*N*/ 	XubString sVal;
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch(nMemberId)
/*N*/ 	{
/*N*/ 		case MID_URL_URL:
/*N*/ 			sVal = aURL;
/*N*/ 		break;
/*N*/ 		case MID_URL_TARGET:
/*N*/ 			sVal = aTargetFrame;
/*N*/ 		break;
/*N*/ 		case MID_URL_HYPERLINKNAME:
/*N*/ 			sVal = aName;
/*N*/ 		break;
/*N*/ 		case MID_URL_VISITED_FMT:
/*N*/ 			sVal = aVisitedFmt;
/*N*/ 			if( !sVal.Len() && nVisitedId != 0 )
/*?*/ 				SwStyleNameMapper::FillUIName( nVisitedId, sVal );
/*N*/ 			if( sVal.Len() )
/*?*/ 				SwStyleNameMapper::FillProgName( sVal, sVal, GET_POOLID_CHRFMT, sal_True );
/*N*/ 		break;
/*N*/ 		case MID_URL_UNVISITED_FMT:
/*N*/ 			sVal = aINetFmt;
/*N*/ 			if( !sVal.Len() && nINetId != 0 )
/*?*/ 				SwStyleNameMapper::FillUIName( nINetId, sVal );
/*N*/ 			if( sVal.Len() )
/*?*/ 				SwStyleNameMapper::FillProgName( sVal, sVal, GET_POOLID_CHRFMT, sal_True );
/*N*/ 		break;
/*N*/ 		case MID_URL_HYPERLINKEVENTS:
/*N*/ 		{
/*N*/ 			// create (and return) event descriptor
/*N*/ 			SwHyperlinkEventDescriptor* pEvents =
/*N*/ 				new SwHyperlinkEventDescriptor();
/*N*/ 			pEvents->copyMacrosFromINetFmt(*this);
/*N*/ 			uno::Reference<container::XNameReplace> xNameReplace(pEvents);
/*N*/
/*N*/ 			// all others return a string; so we just set rVal here and exit
/*N*/ 			rVal <<= xNameReplace;
/*N*/ 			return bRet;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		default:
/*N*/ 			bRet = false;
/*N*/ 	}
/*N*/ 	rVal <<= OUString(sVal);
/*N*/ 	return bRet;
/*N*/ }
/*N*/ bool SwFmtINetFmt::PutValue( const uno::Any& rVal, BYTE nMemberId  )
/*N*/ {
/*N*/ 	bool bRet = TRUE;
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/
/*N*/ 	// all properties except HyperlinkEvents are of type string, hence
/*N*/ 	// we treat HyperlinkEvents specially
/*N*/ 	if (MID_URL_HYPERLINKEVENTS == nMemberId)
/*N*/ 	{
/*?*/ 		uno::Reference<container::XNameReplace> xReplace;
/*?*/ 		rVal >>= xReplace;
/*?*/ 		if (xReplace.is())
/*?*/ 		{
/*?*/ 			// Create hyperlink event descriptor. Then copy events
/*?*/ 			// from argument into descriptor. Then copy events from
/*?*/ 			// the descriptor into the format.
/*?*/ 			SwHyperlinkEventDescriptor* pEvents = new SwHyperlinkEventDescriptor();
/*?*/ 			uno::Reference< ::com::sun::star::lang::XServiceInfo> xHold = pEvents;
/*?*/ 			pEvents->copyMacrosFromNameReplace(xReplace);
/*?*/ 			pEvents->copyMacrosIntoINetFmt(*this);
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			// wrong type!
/*?*/ 			bRet = false;
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// all string properties:
/*N*/ 		if(rVal.getValueType() != ::getCppuType((::rtl::OUString*)0))
/*N*/ 			return false;
/*N*/ 		XubString sVal = *(::rtl::OUString*)rVal.getValue();
/*N*/ 		switch(nMemberId)
/*N*/ 		{
/*N*/ 			case MID_URL_URL:
/*N*/ 				aURL = sVal;
/*N*/ 				break;
/*N*/ 			case MID_URL_TARGET:
/*N*/ 				aTargetFrame = sVal;
/*N*/ 				break;
/*N*/ 			case MID_URL_HYPERLINKNAME:
/*N*/ 				aName = sVal;
/*N*/ 				break;
/*?*/ 			case MID_URL_VISITED_FMT:
/*?*/ 			{
/*?*/ 				String aString;
/*?*/ 				SwStyleNameMapper::FillUIName( sVal, aString, GET_POOLID_CHRFMT, sal_True );
/*?*/ 				aVisitedFmt = OUString ( aString );
/*?*/ 				nVisitedId = SwStyleNameMapper::GetPoolIdFromUIName( aVisitedFmt,
/*?*/ 											   GET_POOLID_CHRFMT );
/*?*/ 			}
/*?*/ 			break;
/*?*/ 			case MID_URL_UNVISITED_FMT:
/*?*/ 			{
/*?*/ 				String aString;
/*?*/ 				SwStyleNameMapper::FillUIName( sVal, aString, GET_POOLID_CHRFMT, sal_True );
/*?*/ 				aINetFmt = OUString ( aString );
/*?*/ 				nINetId = SwStyleNameMapper::GetPoolIdFromUIName( aINetFmt,	GET_POOLID_CHRFMT );
/*?*/ 			}
/*?*/ 			break;
/*?*/ 			default:
/*?*/ 				bRet = false;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*************************************************************************
|*    class SwFmtRuby
*************************************************************************/

/*N*/ SwFmtRuby::SwFmtRuby( const String& rRubyTxt )
/*N*/ 	: SfxPoolItem( RES_TXTATR_CJK_RUBY ),
/*N*/ 	sRubyTxt( rRubyTxt ),
/*N*/ 	nCharFmtId( 0 ),
/*N*/ 	nPosition( 0 ), nAdjustment( 0 ),
/*N*/ 	pTxtAttr( 0 )
/*N*/ {
/*N*/ }

/*?*/ SwFmtRuby::SwFmtRuby( const SwFmtRuby& rAttr )
/*?*/ 	: SfxPoolItem( RES_TXTATR_CJK_RUBY ),
/*?*/ 	sRubyTxt( rAttr.sRubyTxt ),
/*?*/ 	sCharFmtName( rAttr.sCharFmtName ),
/*?*/ 	nCharFmtId( rAttr.nCharFmtId),
/*?*/ 	nPosition( rAttr.nPosition ), nAdjustment( rAttr.nAdjustment ),
/*?*/ 	pTxtAttr( 0 )
/*?*/ {
/*?*/ }

/*N*/ SwFmtRuby::~SwFmtRuby()
/*N*/ {
/*N*/ }

/*N*/  SwFmtRuby& SwFmtRuby::operator=( const SwFmtRuby& rAttr )
/*N*/  {
/*N*/  	sRubyTxt = rAttr.sRubyTxt;
/*N*/  	sCharFmtName = rAttr.sCharFmtName;
/*N*/  	nCharFmtId = rAttr.nCharFmtId;
/*N*/  	nPosition = rAttr.nPosition;
/*N*/  	nAdjustment = rAttr.nAdjustment;
/*N*/  	pTxtAttr =  0;
/*N*/  	return *this;
/*N*/  }

int SwFmtRuby::operator==( const SfxPoolItem& rAttr ) const
{
    ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
    return sRubyTxt == ((SwFmtRuby&)rAttr).sRubyTxt &&
           sCharFmtName == ((SwFmtRuby&)rAttr).sCharFmtName &&
           nCharFmtId == ((SwFmtRuby&)rAttr).nCharFmtId &&
           nPosition == ((SwFmtRuby&)rAttr).nPosition &&
           nAdjustment == ((SwFmtRuby&)rAttr).nAdjustment;
}

SfxPoolItem* SwFmtRuby::Clone( SfxItemPool* ) const
{
    return new SwFmtRuby( *this );
}

bool SwFmtRuby::QueryValue( ::com::sun::star::uno::Any& rVal,
                            BYTE nMemberId ) const
{
    bool bRet = true;
    nMemberId &= ~CONVERT_TWIPS;
    switch( nMemberId )
    {
        case MID_RUBY_TEXT: rVal <<= (OUString)sRubyTxt;                    break;
        case MID_RUBY_ADJUST:   rVal <<= (sal_Int16)nAdjustment;    break;
        case MID_RUBY_CHARSTYLE:
        {
            String aString;
            SwStyleNameMapper::FillProgName(sCharFmtName, aString, GET_POOLID_CHRFMT, sal_True );
            rVal <<= OUString ( aString );
        }
        break;
        case MID_RUBY_ABOVE:
        {
            sal_Bool bAbove = !nPosition;
            rVal.setValue(&bAbove, ::getBooleanCppuType());
        }
        break;
        default:
            bRet = false;
    }
    return bRet;
}
/*N*/ bool SwFmtRuby::PutValue( const ::com::sun::star::uno::Any& rVal,
/*N*/ 							BYTE nMemberId  )
/*N*/ {
/*N*/ 	bool bRet = TRUE;
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_RUBY_TEXT:
/*N*/ 		{
/*N*/ 			OUString sTmp;
/*N*/ 			bRet = rVal >>= sTmp;
/*N*/ 			sRubyTxt = sTmp;
/*N*/ 		}
/*N*/ 		break;
/*N*/  		case MID_RUBY_ADJUST:
/*N*/ 		{
/*N*/ 			sal_Int16 nSet; rVal >>= nSet;
/*N*/ 			if(nSet >= 0 && nSet <= ::com::sun::star::text::RubyAdjust_INDENT_BLOCK)
/*N*/ 				nAdjustment = nSet;
/*N*/ 			else
/*N*/ 				bRet = sal_False;
/*N*/ 		}
/*N*/ 		break;
/*N*/         case MID_RUBY_ABOVE:
/*N*/         {
/*N*/             const uno::Type& rType = ::getBooleanCppuType();
/*N*/             if(rVal.hasValue() && rVal.getValueType() == rType)
/*N*/             {
/*N*/                 sal_Bool bAbove = *(sal_Bool*)rVal.getValue();
/*N*/                 nPosition = bAbove ? 0 : 1;
/*N*/             }
/*N*/         }
/*N*/         break;
/*N*/         case MID_RUBY_CHARSTYLE:
/*N*/         {
/*N*/             OUString sTmp;
/*N*/             bRet = rVal >>= sTmp;
/*N*/             if(bRet)
/*N*/                 sCharFmtName = SwStyleNameMapper::GetUIName(sTmp, GET_POOLID_CHRFMT );
/*N*/         }
/*N*/         break;
/*N*/ 		default:
/*N*/           bRet = false;
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
