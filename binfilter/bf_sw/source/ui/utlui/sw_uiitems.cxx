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


#include "cmdid.h"
#include "uiitems.hxx"

#include "utlui.hrc"
#include "attrdesc.hrc"
#ifndef _UNOMID_H
#include <unomid.h>
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

// Breitenangaben der Fussnotenlinien, mit TabPage abstimmen
static const USHORT __FAR_DATA nFtnLines[] = {
    0,
    10,
    50,
    80,
    100,
    150
};

#define FTN_LINE_STYLE_COUNT 5


/*N*/ SwPageFtnInfoItem::SwPageFtnInfoItem( const USHORT nId, SwPageFtnInfo& rInfo) :
/*N*/ 	SfxPoolItem( nId ),
/*N*/ 	aFtnInfo(rInfo)
/*N*/ {
/*N*/ }


/*N*/ SwPageFtnInfoItem::SwPageFtnInfoItem( const SwPageFtnInfoItem& rItem ) :
/*N*/ 	SfxPoolItem( rItem ),
/*N*/ 	aFtnInfo(rItem.GetPageFtnInfo())
/*N*/ {
/*N*/ }


/*N*/  SwPageFtnInfoItem::~SwPageFtnInfoItem()
/*N*/ {
/*N*/ }


/*N*/ SfxPoolItem*  SwPageFtnInfoItem::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SwPageFtnInfoItem( *this );
/*N*/ }


/*N*/ int  SwPageFtnInfoItem::operator==( const SfxPoolItem& rAttr ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( Which() == rAttr.Which(), "keine gleichen Attribute" );
/*N*/ 	return ( aFtnInfo == ((SwPageFtnInfoItem&)rAttr).GetPageFtnInfo());
/*N*/ }


/* -----------------------------26.04.01 12:25--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ BOOL SwPageFtnInfoItem::QueryValue( Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bRet = sal_True;
/*N*/     switch(nMemberId & ~CONVERT_TWIPS)
/*N*/     {
/*N*/         case MID_FTN_HEIGHT        :     rVal <<= (sal_Int32)TWIP_TO_MM100(aFtnInfo.GetHeight());break;
/*N*/         case MID_LINE_WEIGHT       :     rVal <<= (sal_Int16)TWIP_TO_MM100(aFtnInfo.GetLineWidth());break;
/*N*/         case MID_LINE_COLOR        :     rVal <<= (sal_Int32)aFtnInfo.GetLineColor().GetColor();break;
/*N*/         case MID_LINE_RELWIDTH     :
/*N*/         {
/*N*/             Fraction aTmp( 100, 1 );
/*N*/             aTmp *= aFtnInfo.GetWidth();
/*N*/             rVal <<= (sal_Int8)(long)aTmp;
/*N*/         }
/*N*/         break;
/*N*/         case MID_LINE_ADJUST       :     rVal <<= (sal_Int16)aFtnInfo.GetAdj();break;//com::sun::star::text::HorizontalAdjust
/*N*/         case MID_LINE_TEXT_DIST    :     rVal <<= (sal_Int32)TWIP_TO_MM100(aFtnInfo.GetTopDist());break;
/*N*/         case MID_LINE_FOOTNOTE_DIST:     rVal <<= (sal_Int32)TWIP_TO_MM100(aFtnInfo.GetBottomDist());break;
/*N*/         default:
/*N*/             bRet = sal_False;
/*N*/     }
/*N*/     return bRet;
/*N*/ }
/* -----------------------------26.04.01 12:26--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ BOOL SwPageFtnInfoItem::PutValue(const Any& rVal, BYTE nMemberId)
/*N*/ {
/*N*/     sal_Int32 nSet32;
/*N*/     sal_Bool bRet = sal_True;
/*N*/     switch(nMemberId  & ~CONVERT_TWIPS)
/*N*/     {
/*N*/         case MID_LINE_COLOR        :
/*N*/             rVal >>= nSet32;
/*N*/             aFtnInfo.SetLineColor(nSet32);
/*N*/         break;
/*N*/         case MID_FTN_HEIGHT:
/*N*/         case MID_LINE_TEXT_DIST    :
/*N*/         case MID_LINE_FOOTNOTE_DIST:
/*N*/                 rVal >>= nSet32;
/*N*/                 if(nSet32 < 0)
/*N*/                     bRet = sal_False;
/*N*/                 else
/*N*/                 {
/*N*/                     nSet32 = MM100_TO_TWIP(nSet32);
/*N*/                     switch(nMemberId & ~CONVERT_TWIPS)
/*N*/                     {
/*N*/                         case MID_FTN_HEIGHT:            aFtnInfo.SetHeight(nSet32);    break;
/*N*/                         case MID_LINE_TEXT_DIST:        aFtnInfo.SetTopDist(nSet32);break;
/*N*/                         case MID_LINE_FOOTNOTE_DIST:    aFtnInfo.SetBottomDist(nSet32);break;
/*N*/                     }
/*N*/                 }
/*N*/         break;
/*N*/         case MID_LINE_WEIGHT       :
/*N*/         {
/*N*/             sal_Int16 nSet; rVal >>= nSet;
/*N*/             if(nSet >= 0)
/*N*/                 aFtnInfo.SetLineWidth(MM100_TO_TWIP(nSet));
/*N*/             else
/*N*/                 bRet = sal_False;
/*N*/         }
/*N*/         break;
/*N*/         case MID_LINE_RELWIDTH     :
/*N*/         {
/*N*/             sal_Int8 nSet; rVal >>= nSet;
/*N*/             if(nSet < 0)
/*N*/                 bRet = sal_False;
/*N*/             else
/*N*/                 aFtnInfo.SetWidth(Fraction(nSet, 100));
/*N*/         }
/*N*/         break;
/*N*/         case MID_LINE_ADJUST       :
/*N*/         {
/*N*/             sal_Int16 nSet; rVal >>= nSet;
/*N*/             if(nSet >= 0 && nSet < 3) //com::sun::star::text::HorizontalAdjust
/*N*/                 aFtnInfo.SetAdj((SwFtnAdj)nSet);
/*N*/             else
/*N*/                 bRet = sal_False;
/*N*/         }
/*N*/         break;
/*N*/         default:
/*N*/             bRet = sal_False;
/*N*/     }
/*N*/     return bRet;
/*N*/ }


/*--------------------------------------------------------------------
    Beschreibung: Copy-Konstruktor
 --------------------------------------------------------------------*/



/*--------------------------------------------------------------------
    Beschreibung: Clonen
 --------------------------------------------------------------------*/



/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/




/*-----------------12.11.97 12:55-------------------------------
 SwUINumRuleItem fuer die NumTabPages der FormatNumRule/Stylisten
---------------------------------------------------------------*/







/* -----------------17.06.98 17:43-------------------
 *
 * --------------------------------------------------*/
/* -----------------17.06.98 17:44-------------------
 *
 * --------------------------------------------------*/



}
