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

// include ---------------------------------------------------------------

#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>

#include <com/sun/star/drawing/Hatch.hpp>

#include <com/sun/star/drawing/LineStyle.hpp>

#include <com/sun/star/drawing/LineDash.hpp>

#include <com/sun/star/drawing/FillStyle.hpp>

#include <com/sun/star/awt/Gradient.hpp>

#include <bf_svtools/itempool.hxx>

#include <bf_svx/xdef.hxx>

#include "unoapi.hxx"

#include <bf_svtools/style.hxx>

#include "unopolyhelper.hxx"

#include <bf_svtools/itemset.hxx>
#include "dialogs.hrc"
#include "svdstr.hrc"
#include "xattr.hxx"
#include "xtable.hxx"
#include "itemtype.hxx"

#include "xftadit.hxx"
#include "xftdiit.hxx"
#include "xftmrit.hxx"
#include "xftouit.hxx"
#include "xftsfit.hxx"
#include "xftshcit.hxx"
#include "xftshit.hxx" 
#include "xftshxy.hxx"
#include "xftstit.hxx"
#include "xtxasit.hxx"

#include "svdmodel.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

#define GLOBALOVERFLOW

/************************************************************************/

#define VCLTOSVCOL( rCol ) (USHORT)((((USHORT)(rCol))<<8)|(rCol))

/************************************************************************/

XubString aNameOrIndexEmptyString;

/*N*/ TYPEINIT1_AUTOFACTORY(NameOrIndex, SfxStringItem);

/*************************************************************************
|*
|*	  NameOrIndex::NameOrIndex(USHORT nWhich, long nIndex)
|*
|*	  Beschreibung
|*	  Ersterstellung	14.11.94
|*	  Letzte Aenderung	14.11.94
|*
*************************************************************************/

/*N*/ NameOrIndex::NameOrIndex(USHORT nWhich, long nIndex) :
/*N*/ 	SfxStringItem(nWhich, aNameOrIndexEmptyString),
/*N*/ 	nPalIndex(nIndex)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  NameOrIndex::NameOrIndex(USHORT nWhich, const String& rName)
|*
|*	  Beschreibung
|*	  Ersterstellung	14.11.94
|*	  Letzte Aenderung	14.11.94
|*
*************************************************************************/

/*N*/ NameOrIndex::NameOrIndex(USHORT nWhich, const XubString& rName) :
/*N*/ 	SfxStringItem(nWhich, rName),
/*N*/ 	nPalIndex((long)-1)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  NameOrIndex::NameOrIndex(USHORT nWhich, SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	14.11.94
|*	  Letzte Aenderung	14.11.94
|*
*************************************************************************/

/*N*/ NameOrIndex::NameOrIndex(USHORT nWhich, SvStream& rIn) :
/*N*/ 	SfxStringItem(nWhich, rIn)
/*N*/ {
/*N*/ 	rIn >> nPalIndex;
/*N*/ }

/*************************************************************************
|*
|*	  NameOrIndex::NameOrIndex(const NameOrIndex& rNameOrIndex)
|*
|*	  Beschreibung
|*	  Ersterstellung	14.11.94
|*	  Letzte Aenderung	14.11.94
|*
*************************************************************************/

/*N*/ NameOrIndex::NameOrIndex(const NameOrIndex& rNameOrIndex) :
/*N*/ 	SfxStringItem(rNameOrIndex),
/*N*/ 	nPalIndex(rNameOrIndex.nPalIndex)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  int NameOrIndex::operator==(const SfxPoolItem& rItem) const
|*
|*	  Beschreibung
|*	  Ersterstellung	14.11.94
|*	  Letzte Aenderung	14.11.94
|*
*************************************************************************/

/*N*/ int NameOrIndex::operator==(const SfxPoolItem& rItem) const
/*N*/ {
/*N*/ 	return ( SfxStringItem::operator==(rItem) &&
/*N*/ 			((const NameOrIndex&) rItem).nPalIndex == nPalIndex );
/*N*/ }

/*************************************************************************
|*
|*	  SvStream* NameOrIndex::Store(SvStream& rIn) const
|*
|*	  Beschreibung
|*	  Ersterstellung	14.11.94
|*	  Letzte Aenderung	14.11.94
|*
*************************************************************************/

/*N*/ SvStream& NameOrIndex::Store( SvStream& rOut, USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	SfxStringItem::Store( rOut, nItemVersion );
/*N*/ 	rOut << (INT32)nPalIndex;
/*N*/ 	return rOut;
/*N*/ }

/** this static checks if the given NameOrIndex item has a unique name for its value.
    The returned String is a unique name for an item with this value in both given pools.
    Argument pPool2 can be null.
    If returned string equals NameOrIndex->GetName(), the name was already unique.
*/
/*N*/ String NameOrIndex::CheckNamedItem( const NameOrIndex* pCheckItem, const sal_uInt16 nWhich, const SfxItemPool* pPool1, const SfxItemPool* pPool2, SvxCompareValueFunc pCompareValueFunc, USHORT nPrefixResId, XPropertyList* pDefaults )
/*N*/ {
/*N*/ 	sal_Bool bForceNew = sal_False;
/*N*/ 
/*N*/ 	String aUniqueName;
/*N*/ 	SvxUnogetInternalNameForItem( nWhich, pCheckItem->GetName(), aUniqueName );
/*N*/ 
/*N*/ 	// 2. if we have a name check if there is already an item with the
/*N*/ 	// same name in the documents pool with a different line end or start
/*N*/ 
/*N*/ 	if( aUniqueName.Len() && pPool1 )
/*N*/ 	{
/*N*/ 		const sal_uInt16 nCount = pPool1->GetItemCount( nWhich );
/*N*/ 
/*N*/ 		const NameOrIndex *pItem;
/*N*/ 		for( sal_uInt16 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
/*N*/ 		{
/*N*/ 			pItem = (NameOrIndex*)pPool1->GetItem( nWhich, nSurrogate );
/*N*/ 
/*N*/ 			if( pItem && ( pItem->GetName() == pCheckItem->GetName() ) )
/*N*/ 			{
/*N*/ 				// if there is already an item with the same name and the same
/*N*/ 				// value its ok to set it
/*N*/ 				if( !pCompareValueFunc( pItem, pCheckItem ) )
/*N*/ 				{
/*N*/ 					// same name but different value, we need a new name for this item
/*N*/ 					aUniqueName = String();
/*N*/ 					bForceNew = sal_True;
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// if we have no name yet, find existing item with same conent or
/*N*/ 	// create a unique name
/*N*/ 	if( aUniqueName.Len() == 0 )
/*N*/ 	{
/*N*/ 		sal_Int32 nUserIndex = 1;
/*N*/ 		const ResId aRes(SVX_RES(nPrefixResId));
/*N*/ 		String aUser( aRes );
/*N*/ 		aUser += sal_Unicode( ' ' );
/*N*/ 
/*N*/ 		if( pDefaults )
/*N*/ 		{
/*N*/ 			const int nCount = pDefaults->Count();
/*N*/ 			int nIndex;
/*N*/ 			for( nIndex = 0; nIndex < nCount; nIndex++ )
/*N*/ 			{
/*N*/ 				XPropertyEntry* pEntry = pDefaults->Get( nIndex, 0 );
/*N*/ 				if( pEntry )
/*N*/ 				{
/*N*/ 					bool bFound = false;
/*N*/ 
/*N*/ 					switch( nWhich )
/*N*/ 					{
/*N*/ 					case XATTR_FILLBITMAP:
/*N*/ 						bFound =  (((XFillBitmapItem*)pCheckItem)->GetValue().GetGraphicObject().GetUniqueID() ==
/*N*/ 							((XBitmapEntry*)pEntry)->GetXBitmap().GetGraphicObject().GetUniqueID());
/*N*/ 						break;
/*N*/ 					case XATTR_LINEDASH:
/*N*/ 						bFound = (((XLineDashItem*)pCheckItem)->GetValue() == ((XDashEntry*)pEntry) ->GetDash());
/*N*/ 						break;
/*N*/ 					case XATTR_LINESTART:
/*?*/ 						bFound = (((XLineStartItem*)pCheckItem)->GetValue() == ((XLineEndEntry*)pEntry)->GetLineEnd());
/*N*/ 						break;
/*N*/ 					case XATTR_LINEEND:
/*?*/ 						bFound = (((XLineEndItem*)pCheckItem)->GetValue() == ((XLineEndEntry*)pEntry)->GetLineEnd());
/*N*/ 						break;
/*N*/ 					case XATTR_FILLGRADIENT:
/*N*/ 						bFound = (((XFillGradientItem*)pCheckItem)->GetValue() == ((XGradientEntry*)pEntry)->GetGradient());
/*N*/ 						break;
/*N*/ 					case XATTR_FILLHATCH:
/*N*/ 						bFound = (((XFillHatchItem*)pCheckItem)->GetValue() == ((XHatchEntry*)pEntry)->GetHatch());
/*N*/ 						break;
/*N*/ 					}
/*N*/ 
/*N*/ 					if( bFound )
/*N*/ 					{
/*?*/ 						aUniqueName = pEntry->GetName();
/*?*/ 						break;
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						sal_Int32 nThisIndex = pEntry->GetName().Copy( aUser.Len() ).ToInt32();
/*N*/ 						if( nThisIndex >= nUserIndex )
/*N*/ 							nUserIndex = nThisIndex + 1;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if( (aUniqueName.Len() == 0) && pPool1 )
/*N*/ 		{
/*N*/ 			const sal_uInt16 nCount = pPool1->GetItemCount( nWhich );
/*N*/ 			const NameOrIndex *pItem;
/*N*/ 			for( sal_uInt16 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
/*N*/ 			{
/*N*/ 				pItem = (NameOrIndex*)pPool1->GetItem( nWhich, nSurrogate );
/*N*/ 
/*N*/ 				if( pItem && pItem->GetName().Len() )
/*N*/ 				{
/*N*/ 					if( !bForceNew && pCompareValueFunc( pItem, pCheckItem ) )
/*N*/ 						return pItem->GetName();
/*N*/ 
/*N*/ 					if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
/*N*/ 					{
/*N*/ 						sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
/*N*/ 						if( nThisIndex >= nUserIndex )
/*N*/ 							nUserIndex = nThisIndex + 1;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		aUniqueName = aUser;
/*N*/ 		aUniqueName += String::CreateFromInt32( nUserIndex );
/*N*/ 	}
/*N*/ 
/*N*/ 	return aUniqueName;
/*N*/ }

//*************************************************************************

// -------------------
// class XColorItem
// -------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XColorItem, NameOrIndex);

/*************************************************************************
|*
|*	  XColorItem::XColorItem(USHORT nWhich, const String& rName, const Color& rTheColor)
|*
\************************************************************************/

/*N*/ XColorItem::XColorItem(USHORT nWhich, const XubString& rName, const Color& rTheColor) :
/*N*/ 	NameOrIndex(nWhich, rName),
/*N*/ 	aColor(rTheColor)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XColorItem::XColorItem(const XColorItem& rItem)
|*
\************************************************************************/

/*N*/ XColorItem::XColorItem(const XColorItem& rItem) :
/*N*/ 	NameOrIndex(rItem),
/*N*/ 	aColor(rItem.aColor)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XColorItem::XColorItem(USHORT nWhich, SvStream& rIn)
|*
\************************************************************************/

/*N*/ XColorItem::XColorItem(USHORT nWhich, SvStream& rIn) :
/*N*/ 	NameOrIndex(nWhich, rIn)
/*N*/ {
/*N*/ 	if (!IsIndex())
/*N*/ 	{
/*N*/ 		rIn >> aColor;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	  XColorItem::Clone(SfxItemPool* pPool) const
|*
\************************************************************************/

/*N*/ SfxPoolItem* XColorItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XColorItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  int XColorItem::operator==(const SfxPoolItem& rItem) const
|*
\************************************************************************/

/*N*/ int XColorItem::operator==(const SfxPoolItem& rItem) const
/*N*/ {
/*N*/ 	return ( NameOrIndex::operator==(rItem) &&
/*N*/ 			((const XColorItem&) rItem).aColor == aColor );
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XColorItem::Create(SvStream& rIn, USHORT nVer) const
|*
\************************************************************************/

/*N*/ SfxPoolItem* XColorItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XColorItem(Which(), rIn);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XColorItem::Store(SvStream& rOut) const
|*
\************************************************************************/

/*N*/ SvStream& XColorItem::Store( SvStream& rOut, USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	NameOrIndex::Store( rOut, nItemVersion );
/*N*/ 
/*N*/ 	if ( !IsIndex() )
/*N*/ 	{
/*N*/ 		rOut << aColor;
/*N*/ 	}
/*N*/ 
/*N*/ 	return rOut;
/*N*/ }

/*************************************************************************
|*
|*	  const XColor& XColorItem::GetValue(const XColorTable* pTable) const
|*
\************************************************************************/

/*N*/ const Color& XColorItem::GetValue(const XColorTable* pTable) const
/*N*/ {
/*N*/ 	if (!IsIndex())
/*N*/ 		return aColor;
/*N*/ 	else
/*?*/ 		return pTable->Get(GetIndex())->GetColor();
/*N*/ 
/*N*/ }

/*N*/ bool XColorItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (sal_Int32)GetValue().GetRGBColor();
/*N*/ 	return true;
/*N*/ }

/*N*/ bool XColorItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Int32 nValue;
/*N*/ 	rVal >>= nValue;
/*N*/ 	SetValue( nValue );
/*N*/
/*N*/ 	return true;
/*N*/ }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//							 Linienattribute
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------
// class XLineStyleItem
//---------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XLineStyleItem, SfxEnumItem);

/*************************************************************************
|*
|*	  XLineStyleItem::XLineStyleItem(XLineStyle eTheLineStyle)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ XLineStyleItem::XLineStyleItem(XLineStyle eTheLineStyle) :
/*N*/ 	SfxEnumItem(XATTR_LINESTYLE, eTheLineStyle)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineStyleItem::XLineStyleItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XLineStyleItem::XLineStyleItem(SvStream& rIn) :
/*N*/ 	SfxEnumItem(XATTR_LINESTYLE, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineStyleItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	09.11.94
|*	  Letzte Aenderung	09.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineStyleItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XLineStyleItem( *this );
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XLineStyleItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineStyleItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XLineStyleItem(rIn);
/*N*/ }

//------------------------------------------------------------------------


/*N*/ bool XLineStyleItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	::com::sun::star::drawing::LineStyle eLS = (::com::sun::star::drawing::LineStyle)GetValue();
/*N*/ 	rVal <<= eLS;
/*N*/ 	return true;
/*N*/ }

/*N*/ bool XLineStyleItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	::com::sun::star::drawing::LineStyle eLS;
/*N*/ 	if(!(rVal >>= eLS ))
/*N*/     {
/*?*/         // also try an int (for Basic)
/*?*/         sal_Int32 nLS;
/*?*/         if(!(rVal >>= nLS))
/*?*/             return sal_False;
/*?*/         eLS = (::com::sun::star::drawing::LineStyle)nLS;
/*N*/     }
/*N*/
/*N*/ 	SetValue( (XLineStyle)eLS );
/*N*/ 	return true;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ USHORT XLineStyleItem::GetValueCount() const
/*N*/ {
/*N*/ 	return 3;
/*N*/ }


// ------------
// class XDash
// ------------
/*************************************************************************
|*
|*	  XDash::XDash(XDashStyle, USHORT, ULONG, USHORT, ULONG, ULONG)
|*
|*	  Beschreibung
|*	  Ersterstellung	21.11.94
|*	  Letzte Aenderung	21.11.94
|*
*************************************************************************/

/*N*/ XDash::XDash(XDashStyle eTheDash, USHORT nTheDots, ULONG nTheDotLen,
/*N*/ 			 USHORT nTheDashes, ULONG nTheDashLen, ULONG nTheDistance) :
/*N*/ 	eDash(eTheDash),
/*N*/ 	nDots(nTheDots),
/*N*/ 	nDotLen(nTheDotLen),
/*N*/ 	nDashes(nTheDashes),
/*N*/ 	nDashLen(nTheDashLen),
/*N*/ 	nDistance(nTheDistance)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  int XDash::operator==(const SfxPoolItem& rItem) const
|*
|*	  Beschreibung
|*	  Ersterstellung	29.11.94
|*	  Letzte Aenderung	29.11.94
|*
*************************************************************************/

/*N*/ int XDash::operator==(const XDash& rDash) const
/*N*/ {
/*N*/ 	return ( eDash		== rDash.eDash		&&
/*N*/ 			 nDots		== rDash.nDots		&&
/*N*/ 			 nDotLen	== rDash.nDotLen	&&
/*N*/ 			 nDashes	== rDash.nDashes	&&
/*N*/ 			 nDashLen	== rDash.nDashLen	&&
/*N*/ 			 nDistance	== rDash.nDistance );
/*N*/ }


// -------------------
// class XLineDashItem
// -------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XLineDashItem, NameOrIndex);

/*************************************************************************
|*
|*	  XLineDashItem::XLineDashItem(const String& rName, const XDash& rTheDash)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XLineDashItem::XLineDashItem(const XubString& rName, const XDash& rTheDash) :
/*N*/ 	NameOrIndex(XATTR_LINEDASH, rName),
/*N*/ 	aDash(rTheDash)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineDashItem::XLineDashItem(const XLineDashItem& rItem)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XLineDashItem::XLineDashItem(const XLineDashItem& rItem) :
/*N*/ 	NameOrIndex(rItem),
/*N*/ 	aDash(rItem.aDash)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineDashItem::XLineDashItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XLineDashItem::XLineDashItem(SvStream& rIn) :
/*N*/ 	NameOrIndex(XATTR_LINEDASH, rIn)
/*N*/ {
/*N*/ 	if (!IsIndex())
/*N*/ 	{
/*N*/ 		USHORT	nSTemp;
/*N*/ 		UINT32	nLTemp;
/*N*/ 		INT32   nITemp;
/*N*/ 
/*N*/ 		rIn >> nITemp; aDash.SetDashStyle((XDashStyle)nITemp);
/*N*/ 		rIn >> nSTemp; aDash.SetDots(nSTemp);
/*N*/ 		rIn >> nLTemp; aDash.SetDotLen(nLTemp);
/*N*/ 		rIn >> nSTemp; aDash.SetDashes(nSTemp);
/*N*/ 		rIn >> nLTemp; aDash.SetDashLen(nLTemp);
/*N*/ 		rIn >> nLTemp; aDash.SetDistance(nLTemp);
/*N*/ 	}
/*N*/ }

//*************************************************************************

/*N*/ XLineDashItem::XLineDashItem(SfxItemPool* pPool, const XDash& rTheDash)
/*N*/ : 	NameOrIndex( XATTR_LINEDASH, -1 ),
/*N*/ 	aDash(rTheDash)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineDashItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineDashItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XLineDashItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  int XLineDashItem::operator==(const SfxPoolItem& rItem) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ int XLineDashItem::operator==(const SfxPoolItem& rItem) const
/*N*/ {
/*N*/ 	return ( NameOrIndex::operator==(rItem) &&
/*N*/ 			 aDash == ((const XLineDashItem&) rItem).aDash );
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XLineDashItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineDashItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XLineDashItem(rIn);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XLineDashItem::Store(SvStream& rOut) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SvStream& XLineDashItem::Store( SvStream& rOut, USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	NameOrIndex::Store( rOut, nItemVersion );
/*N*/ 
/*N*/ 	if (!IsIndex())
/*N*/ 	{
/*N*/ 		rOut << (INT32) aDash.GetDashStyle();
/*N*/ 		rOut << aDash.GetDots();
/*N*/ 		rOut << (UINT32) aDash.GetDotLen();
/*N*/ 		rOut << aDash.GetDashes();
/*N*/ 		rOut << (UINT32) aDash.GetDashLen();
/*N*/ 		rOut << (UINT32) aDash.GetDistance();
/*N*/ 	}
/*N*/ 
/*N*/ 	return rOut;
/*N*/ }

/*************************************************************************
|*
|*	  const XDash& XLineDashItem::GetValue(const XDashTable* pTable) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ const XDash& XLineDashItem::GetValue(const XDashTable* pTable) const
/*N*/ {
/*N*/ 	if (!IsIndex())
/*N*/ 		return aDash;
/*N*/ 	else
/*?*/ 		return pTable->Get(GetIndex())->GetDash();
/*N*/ }

//------------------------------------------------------------------------


/*N*/ bool XLineDashItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/
/*N*/ 	switch ( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_NAME:
/*N*/ 		{
/*N*/ 			::rtl::OUString aApiName;
/*N*/ 			SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
/*N*/ 			rVal <<= aApiName;
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_LINEDASH:
/*N*/ 		{
/*N*/ 			const XDash& rXD = GetValue();
/*N*/
/*N*/ 			::com::sun::star::drawing::LineDash aLineDash;
/*N*/
/*N*/ 			aLineDash.Style = (::com::sun::star::drawing::DashStyle)((UINT16)rXD.GetDashStyle());
/*N*/ 			aLineDash.Dots = rXD.GetDots();
/*N*/ 			aLineDash.DotLen = rXD.GetDotLen();
/*N*/ 			aLineDash.Dashes = rXD.GetDashes();
/*N*/ 			aLineDash.DashLen = rXD.GetDashLen();
/*N*/ 			aLineDash.Distance = rXD.GetDistance();
/*N*/
/*N*/ 			rVal <<= aLineDash;
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_LINEDASH_STYLE:
/*N*/ 		{
/*?*/ 			const XDash& rXD = GetValue();
/*?*/ 			rVal <<= (::com::sun::star::drawing::DashStyle)((sal_Int16)rXD.GetDashStyle());
/*?*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_LINEDASH_DOTS:
/*N*/ 		{
/*?*/ 			const XDash& rXD = GetValue();
/*?*/ 			rVal <<= rXD.GetDots();
/*?*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_LINEDASH_DOTLEN:
/*N*/ 		{
/*?*/ 			const XDash& rXD = GetValue();
/*?*/ 			rVal <<= rXD.GetDotLen();
/*?*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_LINEDASH_DASHES:
/*N*/ 		{
/*?*/ 			const XDash& rXD = GetValue();
/*?*/ 			rVal <<= rXD.GetDashes();
/*?*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_LINEDASH_DASHLEN:
/*N*/ 		{
/*?*/ 			const XDash& rXD = GetValue();
/*?*/ 			rVal <<= rXD.GetDashLen();
/*?*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_LINEDASH_DISTANCE:
/*N*/ 		{
/*?*/ 			const XDash& rXD = GetValue();
/*?*/ 			rVal <<= rXD.GetDistance();
/*?*/ 			break;
/*N*/ 		}
/*N*/
/*?*/ 		default: DBG_ERROR("Wrong MemberId!"); return false;
/*N*/ 	}
/*N*/
/*N*/ 	return true;
/*N*/ }

/*N*/ bool XLineDashItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/
/*N*/ 	switch ( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_NAME:
/*N*/ 		{
/*?*/ 			::rtl::OUString aName;
/*?*/ 			if (!(rVal >>= aName))
/*?*/ 				return false;
/*?*/ 			SetName( aName );
/*?*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_LINEDASH:
/*N*/ 		{
/*N*/ 			::com::sun::star::drawing::LineDash aLineDash;
/*N*/ 			if(!(rVal >>= aLineDash))
/*N*/ 				return false;
/*N*/
/*N*/ 			XDash aXDash;
/*N*/
/*N*/ 			aXDash.SetDashStyle((XDashStyle)((UINT16)(aLineDash.Style)));
/*N*/ 			aXDash.SetDots(aLineDash.Dots);
/*N*/ 			aXDash.SetDotLen(aLineDash.DotLen);
/*N*/ 			aXDash.SetDashes(aLineDash.Dashes);
/*N*/ 			aXDash.SetDashLen(aLineDash.DashLen);
/*N*/ 			aXDash.SetDistance(aLineDash.Distance);
/*N*/
/*N*/ 			if((0 == aXDash.GetDots()) && (0 == aXDash.GetDashes()))
/*?*/ 				aXDash.SetDots(1);
/*N*/
/*N*/ 			SetValue( aXDash );
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_LINEDASH_STYLE:
/*N*/ 		{
/*?*/ 			sal_Int16 nVal;
/*?*/ 			if(!(rVal >>= nVal))
/*?*/ 				return false;
/*?*/
/*?*/ 			XDash aXDash = GetValue();
/*?*/ 			aXDash.SetDashStyle((XDashStyle)((UINT16)(nVal)));
/*?*/
/*?*/ 			if((0 == aXDash.GetDots()) && (0 == aXDash.GetDashes()))
/*?*/ 				aXDash.SetDots(1);
/*?*/
/*?*/ 			SetValue( aXDash );
/*?*/
/*?*/ 			break;
/*N*/ 		}

/*N*/ 		case MID_LINEDASH_DOTS:
/*N*/ 		case MID_LINEDASH_DASHES:
/*N*/ 		{
/*?*/ 			sal_Int16 nVal;
/*?*/ 			if(!(rVal >>= nVal))
/*?*/ 				return false;
/*?*/
/*?*/ 			XDash aXDash = GetValue();
/*?*/ 			if ( nMemberId == MID_LINEDASH_DOTS )
/*?*/ 				aXDash.SetDots( nVal );
/*?*/ 			else
/*?*/ 				aXDash.SetDashes( nVal );
/*?*/
/*?*/ 			if((0 == aXDash.GetDots()) && (0 == aXDash.GetDashes()))
/*?*/ 				aXDash.SetDots(1);
/*?*/
/*?*/ 			SetValue( aXDash );
/*?*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_LINEDASH_DOTLEN:
/*N*/ 		case MID_LINEDASH_DASHLEN:
/*N*/ 		case MID_LINEDASH_DISTANCE:
/*N*/ 		{
/*?*/ 			sal_Int32 nVal;
/*?*/ 			if(!(rVal >>= nVal))
/*?*/ 				return false;
/*?*/
/*?*/ 			XDash aXDash = GetValue();
/*?*/ 			if ( nMemberId == MID_LINEDASH_DOTLEN )
/*?*/ 				aXDash.SetDotLen( nVal );
/*?*/ 			else if ( nMemberId == MID_LINEDASH_DASHLEN )
/*?*/ 				aXDash.SetDashLen( nVal );
/*?*/ 			else
/*?*/ 				aXDash.SetDistance( nVal );
/*?*/
/*?*/ 			if((0 == aXDash.GetDots()) && (0 == aXDash.GetDashes()))
/*?*/ 				aXDash.SetDots(1);
/*?*/
/*?*/ 			SetValue( aXDash );
/*?*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return true;
/*N*/ }

/*N*/ BOOL XLineDashItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
/*N*/ {
/*N*/ 	return ((XLineDashItem*)p1)->GetValue() == ((XLineDashItem*)p2)->GetValue();
/*N*/ }

/*N*/ XLineDashItem* XLineDashItem::checkForUniqueItem( SdrModel* pModel ) const
/*N*/ {
/*N*/ 	if( pModel )
/*N*/ 	{
/*N*/ 		const String aUniqueName = NameOrIndex::CheckNamedItem(	this,
/*N*/ 																XATTR_LINEDASH,
/*N*/ 																&pModel->GetItemPool(),
/*N*/ 																pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
/*N*/ 																XLineDashItem::CompareValueFunc,
/*N*/ 																RID_SVXSTR_DASH11,
/*N*/ 																pModel->GetDashList() );
/*N*/ 
/*N*/ 		// if the given name is not valid, replace it!
/*N*/ 		if( aUniqueName != GetName() )
/*N*/ 		{
/*N*/ 			return new XLineDashItem( aUniqueName, aDash );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return (XLineDashItem*)this;
/*N*/ }

// -------------------
// class XLineWidthItem
// -------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XLineWidthItem, SfxMetricItem);

/*************************************************************************
|*
|*	  XLineWidthItem::XLineWidthItem(long nWidth)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ XLineWidthItem::XLineWidthItem(long nWidth) :
/*N*/ 	SfxMetricItem(XATTR_LINEWIDTH, nWidth)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineWidthItem::XLineWidthItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XLineWidthItem::XLineWidthItem(SvStream& rIn) :
/*N*/ 	SfxMetricItem(XATTR_LINEWIDTH, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineWidthItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineWidthItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XLineWidthItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XLineWidthItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineWidthItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XLineWidthItem(rIn);
/*N*/ }

//------------------------------------------------------------------------


/*N*/ bool XLineWidthItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (sal_Int32)GetValue();
/*N*/ 	return true;
/*N*/ }

/*N*/ bool XLineWidthItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Int32 nValue;
/*N*/ 	rVal >>= nValue;
/*N*/
/*N*/ 	SetValue( nValue );
/*N*/ 	return true;
/*N*/ }

// -------------------
// class XLineColorItem
// -------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XLineColorItem, XColorItem);

/*************************************************************************
|*
|*	  XLineColorItem::XLineColorItem(const XubString& rName, const Color& rTheColor)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XLineColorItem::XLineColorItem(const XubString& rName, const Color& rTheColor) :
/*N*/ 	XColorItem(XATTR_LINECOLOR, rName, rTheColor)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineColorItem::XLineColorItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XLineColorItem::XLineColorItem(SvStream& rIn) :
/*N*/ 	XColorItem(XATTR_LINECOLOR, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineColorItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineColorItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XLineColorItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XLineColorItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineColorItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XLineColorItem(rIn);
/*N*/ }

//------------------------------------------------------------------------


/*N*/ bool XLineColorItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (sal_Int32)GetValue().GetRGBColor();
/*N*/ 	return true;
/*N*/ }

/*N*/ bool XLineColorItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Int32 nValue;
/*N*/ 	if(!(rVal >>= nValue))
/*N*/ 		return false;
/*N*/
/*N*/ 	SetValue( nValue );
/*N*/ 	return true;
/*N*/ }

// -----------------------
// class XLineStartItem
// -----------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XLineStartItem, NameOrIndex);

/*************************************************************************
|*
|*	  XLineStartItem::XLineStartItem(long nIndex)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ XLineStartItem::XLineStartItem(long nIndex) :
/*N*/ 	NameOrIndex(XATTR_LINESTART, nIndex)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineStartItem::XLineStartItem(const XubString& rName,
|*									 const XPolygon& rXPolygon)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ XLineStartItem::XLineStartItem(const XubString& rName,
/*N*/ 							   const XPolygon& rXPolygon):
/*N*/ 	NameOrIndex(XATTR_LINESTART, rName),
/*N*/ 	aXPolygon(rXPolygon)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineStartItem::XLineStartItem(const XLineStartItem& rItem)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	17.11.94
|*
*************************************************************************/

/*N*/ XLineStartItem::XLineStartItem(const XLineStartItem& rItem) :
/*N*/ 	NameOrIndex(rItem),
/*N*/ 	aXPolygon(rItem.aXPolygon)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineStartItem::XLineStartItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	17.11.94
|*
*************************************************************************/

/*N*/ XLineStartItem::XLineStartItem(SvStream& rIn) :
/*N*/ 	NameOrIndex(XATTR_LINESTART, rIn)
/*N*/ {
/*N*/ 	if (!IsIndex())
/*N*/ 	{
/*N*/ 		UINT32 nPoints;
/*N*/ 		INT32  nFlags;
/*N*/ 		rIn >> nPoints;
/*N*/ 		aXPolygon.SetSize((USHORT)nPoints);
/*N*/ 		for (USHORT nIndex = 0; nIndex < (USHORT)nPoints; nIndex++)
/*N*/ 		{
/*N*/ 			rIn >> aXPolygon[nIndex].X();
/*N*/ 			rIn >> aXPolygon[nIndex].Y();
/*N*/ 			rIn >> nFlags; aXPolygon.SetFlags(nIndex, (XPolyFlags)nFlags);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//*************************************************************************

/*N*/ XLineStartItem::XLineStartItem(SfxItemPool* pPool, const XPolygon& rXPolygon)
/*N*/ : 	NameOrIndex( XATTR_LINESTART, -1 ),
/*N*/ 	aXPolygon(rXPolygon)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineStartItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineStartItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XLineStartItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  int XLineStartItem::operator==(const SfxPoolItem& rItem) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ int XLineStartItem::operator==(const SfxPoolItem& rItem) const
/*N*/ {
/*N*/ 	return ( NameOrIndex::operator==(rItem) &&
/*N*/ 			((const XLineStartItem&) rItem).aXPolygon == aXPolygon );
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XLineStartItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineStartItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XLineStartItem(rIn);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XLineStartItem::Store(SvStream& rOut) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ SvStream& XLineStartItem::Store( SvStream& rOut, USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	NameOrIndex::Store( rOut, nItemVersion );
/*N*/ 
/*N*/ 	if (!IsIndex())
/*N*/ 	{
/*N*/ 		UINT32 nPoints = (UINT32)aXPolygon.GetPointCount();
/*N*/ 		rOut << nPoints;
/*N*/ 		for (USHORT nIndex = 0; nIndex < (USHORT)nPoints; nIndex++)
/*N*/ 		{
/*N*/ 			rOut << aXPolygon[nIndex].X();
/*N*/ 			rOut << aXPolygon[nIndex].Y();
/*N*/ 			rOut << (INT32)aXPolygon.GetFlags(nIndex);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return rOut;
/*N*/ }

/*************************************************************************
|*
|*	  const XPolygon& XLineStartItem::GetValue(const XLineEndTable* pTable)
|*											   const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ const XPolygon& XLineStartItem::GetValue(const XLineEndTable* pTable) const
/*N*/ {
/*N*/ 	if (!IsIndex())
/*N*/ 		return aXPolygon;
/*N*/ 	else
/*?*/ 		return pTable->Get(GetIndex())->GetLineEnd();
/*N*/ }

//------------------------------------------------------------------------


/*N*/ bool XLineStartItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	if( nMemberId == MID_NAME )
/*N*/ 	{
/*N*/ 		::rtl::OUString aApiName;
/*N*/ 		SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
/*N*/ 		rVal <<= aApiName;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		::com::sun::star::drawing::PolyPolygonBezierCoords aBezier;
/*N*/ 		SvxConvertXPolygonToPolyPolygonBezier( aXPolygon, aBezier );
/*N*/ 		rVal <<= aBezier;
/*N*/ 	}
/*N*/
/*N*/ 	return true;
/*N*/ }

/*N*/ bool XLineStartItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	if( nMemberId == MID_NAME )
/*N*/ 	{
/*?*/ 		return false;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aXPolygon.SetSize( 0 );
/*N*/ 		if( rVal.hasValue() && rVal.getValue() )
/*N*/ 		{
/*N*/ 			if( rVal.getValueType() != ::getCppuType((const ::com::sun::star::drawing::PolyPolygonBezierCoords*)0) )
/*?*/ 				return false;
/*N*/
/*N*/ 			aXPolygon.SetSize(0);
/*N*/ 			::com::sun::star::drawing::PolyPolygonBezierCoords* pCoords = (::com::sun::star::drawing::PolyPolygonBezierCoords*)rVal.getValue();
/*N*/ 			if( pCoords->Coordinates.getLength() > 0 )
/*N*/ 				SvxConvertPolyPolygonBezierToXPolygon( pCoords, aXPolygon );
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return true;
/*N*/ }

/** this function searches in both the models pool and the styles pool for XLineStartItem
    and XLineEndItem with the same value or name and returns an item with the value of
    this item and a unique name for an item with this value. */
/*N*/ XLineStartItem* XLineStartItem::checkForUniqueItem( SdrModel* pModel ) const
/*N*/ {
/*N*/ 	if( pModel )
/*N*/ 	{
/*N*/ 		XLineStartItem* pTempItem = NULL;
/*N*/ 		const XLineStartItem* pLineStartItem = this;
/*N*/ 
/*N*/ 		String aUniqueName( GetName() );
/*N*/ 
/*N*/ 		if( aXPolygon.GetPointCount() == 0 )
/*N*/ 		{
/*N*/ 			// if the polygon is empty, check if the name is empty
/*N*/ 			if( aUniqueName.Len() == 0 )
/*N*/ 				return (XLineStartItem*)this;
/*N*/ 
/*N*/ 			// force empty name for empty polygons
/*N*/ 			return new XLineStartItem( String(), aXPolygon );
/*N*/ 		}
/*N*/ 
/*N*/ 		if( aXPolygon.GetPointCount() > 1 )
/*N*/ 		{
/*N*/ 			// check if the polygon is closed
/*N*/ 			if( aXPolygon[0] != aXPolygon[aXPolygon.GetPointCount() - 1] )
/*N*/ 			{
/*N*/ 				// force a closed polygon
/*N*/ 				XPolygon aNewPolygon( aXPolygon );
/*N*/ 				aNewPolygon[ aXPolygon.GetPointCount() ] = aXPolygon[0];
/*N*/ 				pTempItem = new XLineStartItem( aUniqueName, aNewPolygon );
/*N*/ 				pLineStartItem = pTempItem;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		sal_Bool bForceNew = sal_False;
/*N*/ 
/*N*/ 		// 2. if we have a name check if there is already an item with the
/*N*/ 		// same name in the documents pool with a different line end or start
/*N*/ 
/*N*/ 		sal_uInt16 nCount, nSurrogate;
/*N*/ 
/*N*/ 		const SfxItemPool* pPool1 = &pModel->GetItemPool();
/*N*/ 		if( aUniqueName.Len() && pPool1 )
/*N*/ 		{
/*N*/ 			nCount = pPool1->GetItemCount( XATTR_LINESTART );
/*N*/ 
/*N*/ 			for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
/*N*/ 			{
/*N*/ 				const XLineStartItem* pItem = (const XLineStartItem*)pPool1->GetItem( XATTR_LINESTART, nSurrogate );
/*N*/ 
/*N*/ 				if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
/*N*/ 				{
/*N*/ 					// if there is already an item with the same name and the same
/*N*/ 					// value its ok to set it
/*N*/ 					if( pItem->GetValue() != pLineStartItem->GetValue() )
/*N*/ 					{
/*N*/ 						// same name but different value, we need a new name for this item
/*N*/ 						aUniqueName = String();
/*N*/ 						bForceNew = sal_True;
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if( !bForceNew )
/*N*/ 			{
/*N*/ 				nCount = pPool1->GetItemCount( XATTR_LINEEND );
/*N*/ 
/*N*/ 				for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
/*N*/ 				{
/*N*/ 					const XLineEndItem* pItem = (const XLineEndItem*)pPool1->GetItem( XATTR_LINEEND, nSurrogate );
/*N*/ 
/*N*/ 					if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
/*N*/ 					{
/*N*/ 						// if there is already an item with the same name and the same
/*N*/ 						// value its ok to set it
/*N*/ 						if( pItem->GetValue() != pLineStartItem->GetValue() )
/*N*/ 						{
/*N*/ 							// same name but different value, we need a new name for this item
/*?*/ 							aUniqueName = String();
/*?*/ 							bForceNew = sal_True;
/*N*/ 						}
/*N*/ 						break;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		const SfxItemPool* pPool2 = pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL;
/*N*/ 		if( aUniqueName.Len() && pPool2)
/*N*/ 		{
/*N*/ 			nCount = pPool2->GetItemCount( XATTR_LINESTART );
/*N*/ 			for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
/*N*/ 			{
/*N*/ 				const XLineStartItem* pItem = (const XLineStartItem*)pPool2->GetItem( XATTR_LINESTART, nSurrogate );
/*N*/ 
/*N*/ 				if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
/*N*/ 				{
/*N*/ 					// if there is already an item with the same name and the same
/*N*/ 					// value its ok to set it
/*N*/ 					if( pItem->GetValue() != pLineStartItem->GetValue() )
/*N*/ 					{
/*N*/ 						// same name but different value, we need a new name for this item
/*?*/ 						aUniqueName = String();
/*?*/ 						bForceNew = sal_True;
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if( !bForceNew )
/*N*/ 			{
/*N*/ 				nCount = pPool2->GetItemCount( XATTR_LINEEND );
/*N*/ 				for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
/*N*/ 				{
/*N*/ 					const XLineEndItem* pItem = (const XLineEndItem*)pPool2->GetItem( XATTR_LINEEND, nSurrogate );
/*N*/ 
/*N*/ 					if( pItem && ( pItem->GetName() == pLineStartItem->GetName() ) )
/*N*/ 					{
/*N*/ 						// if there is already an item with the same name and the same
/*N*/ 						// value its ok to set it
/*N*/ 						if( pItem->GetValue() != pLineStartItem->GetValue() )
/*N*/ 						{
/*N*/ 							// same name but different value, we need a new name for this item
/*N*/ 							aUniqueName = String();
/*N*/ 							bForceNew = sal_True;
/*N*/ 						}
/*N*/ 						break;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// if we have no name yet, find existing item with same conent or
/*N*/ 		// create a unique name
/*N*/ 		if( aUniqueName.Len() == 0 )
/*N*/ 		{
/*N*/ 			sal_Bool bFoundExisting = sal_False;
/*N*/ 
/*N*/ 			sal_Int32 nUserIndex = 1;
/*N*/ 			const ResId aRes(SVX_RES(RID_SVXSTR_LINEEND));
/*N*/ 			const String aUser( aRes );
/*N*/ 
/*N*/ 			if( pPool1 )
/*N*/ 			{
/*N*/ 				nCount = pPool1->GetItemCount( XATTR_LINESTART );
/*N*/ 				for( sal_uInt16 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
/*N*/ 				{
/*N*/ 					const XLineStartItem* pItem = (const XLineStartItem*)pPool1->GetItem( XATTR_LINESTART, nSurrogate );
/*N*/ 
/*N*/ 					if( pItem && pItem->GetName().Len() )
/*N*/ 					{
/*N*/ 						if( !bForceNew && pItem->GetValue() == pLineStartItem->GetValue() )
/*N*/ 						{
/*N*/ 							aUniqueName = pItem->GetName();
/*N*/ 							bFoundExisting = sal_True;
/*N*/ 							break;
/*N*/ 						}
/*N*/ 
/*N*/ 						if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
/*N*/ 						{
/*N*/ 							sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
/*N*/ 							if( nThisIndex >= nUserIndex )
/*N*/ 								nUserIndex = nThisIndex + 1;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				nCount = pPool1->GetItemCount( XATTR_LINEEND );
/*N*/ 				for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
/*N*/ 				{
/*N*/ 					const XLineEndItem* pItem = (const XLineEndItem*)pPool1->GetItem( XATTR_LINEEND, nSurrogate );
/*N*/ 
/*N*/ 					if( pItem && pItem->GetName().Len() )
/*N*/ 					{
/*N*/ 						if( !bForceNew && pItem->GetValue() == pLineStartItem->GetValue() )
/*N*/ 						{
/*N*/ 							aUniqueName = pItem->GetName();
/*N*/ 							bFoundExisting = sal_True;
/*N*/ 							break;
/*N*/ 						}
/*N*/ 
/*N*/ 						if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
/*N*/ 						{
/*N*/ 							sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
/*N*/ 							if( nThisIndex >= nUserIndex )
/*N*/ 								nUserIndex = nThisIndex + 1;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if( !bFoundExisting )
/*N*/ 			{
/*N*/ 				aUniqueName = aUser;
/*N*/ 				aUniqueName += sal_Unicode(' ');
/*N*/ 				aUniqueName += String::CreateFromInt32( nUserIndex );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// if the given name is not valid, replace it!
/*N*/ 		if( aUniqueName != GetName() || pTempItem )
/*N*/ 		{
/*N*/ 			if( pTempItem )
/*N*/ 			{
/*N*/ 				pTempItem->SetName( aUniqueName );
/*N*/ 				return pTempItem;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				return new XLineStartItem( aUniqueName, aXPolygon );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return (XLineStartItem*)this;
/*N*/ }

// ---------------------
// class XLineEndItem
// ---------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XLineEndItem, NameOrIndex);

/*************************************************************************
|*
|*	  XLineEndItem::XLineEndItem(long nIndex)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ XLineEndItem::XLineEndItem(long nIndex) :
/*N*/ 	NameOrIndex(XATTR_LINEEND, nIndex)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineEndItem::XLineEndItem(const XubString& rName,
|*								 const XPolygon& rXPolygon)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ XLineEndItem::XLineEndItem(const XubString& rName, const XPolygon& rPolygon) :
/*N*/ 	NameOrIndex(XATTR_LINEEND, rName),
/*N*/ 	aXPolygon(rPolygon)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineEndItem::XLineEndItem(const XLineEndItem& rItem)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ XLineEndItem::XLineEndItem(const XLineEndItem& rItem) :
/*N*/ 	NameOrIndex(rItem),
/*N*/ 	aXPolygon(rItem.aXPolygon)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineEndItem::XLineEndItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	17.11.94
|*
*************************************************************************/

/*N*/ XLineEndItem::XLineEndItem(SvStream& rIn) :
/*N*/ 	NameOrIndex(XATTR_LINEEND, rIn)
/*N*/ {
/*N*/ 	if (!IsIndex())
/*N*/ 	{
/*N*/ 		UINT32 nPoints;
/*N*/ 		INT32  nFlags;
/*N*/ 		rIn >> nPoints;
/*N*/ 		aXPolygon.SetSize((USHORT)nPoints);
/*N*/ 		for (USHORT nIndex = 0; nIndex < (USHORT)nPoints; nIndex++)
/*N*/ 		{
/*N*/ 			rIn >> aXPolygon[nIndex].X();
/*N*/ 			rIn >> aXPolygon[nIndex].Y();
/*N*/ 			rIn >> nFlags; aXPolygon.SetFlags(nIndex, (XPolyFlags)nFlags);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//*************************************************************************

/*N*/ XLineEndItem::XLineEndItem(SfxItemPool* pPool, const XPolygon& rXPolygon)
/*N*/ : 	NameOrIndex( XATTR_LINEEND, -1 ),
/*N*/ 	aXPolygon(rXPolygon)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineEndItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineEndItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XLineEndItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  int XLineEndItem::operator==(const SfxPoolItem& rItem) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	17.11.94
|*
*************************************************************************/

/*N*/ int XLineEndItem::operator==(const SfxPoolItem& rItem) const
/*N*/ {
/*N*/ 	return ( NameOrIndex::operator==(rItem) &&
/*N*/ 			((const XLineEndItem&) rItem).aXPolygon == aXPolygon );
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XLineEndItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineEndItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XLineEndItem(rIn);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XLineEndItem::Store(SvStream& rOut) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	17.11.94
|*
*************************************************************************/

/*N*/ SvStream& XLineEndItem::Store( SvStream& rOut, USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	NameOrIndex::Store( rOut, nItemVersion );
/*N*/ 
/*N*/ 	if (!IsIndex())
/*N*/ 	{
/*N*/ 		UINT32 nPoints = (UINT32)aXPolygon.GetPointCount();
/*N*/ 		rOut << nPoints;
/*N*/ 		for (USHORT nIndex = 0; nIndex < (USHORT)nPoints; nIndex++)
/*N*/ 		{
/*N*/ 			rOut << aXPolygon[nIndex].X();
/*N*/ 			rOut << aXPolygon[nIndex].Y();
/*N*/ 			rOut << (INT32)aXPolygon.GetFlags(nIndex);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return rOut;
/*N*/ }

/*************************************************************************
|*
|*	  const XPolygon& XLineEndItem::GetValue(const XLineEndTable* pTable) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ const XPolygon& XLineEndItem::GetValue(const XLineEndTable* pTable) const
/*N*/ {
/*N*/ 	if (!IsIndex())
/*N*/ 		return aXPolygon;
/*N*/ 	else
/*?*/ 		return pTable->Get(GetIndex())->GetLineEnd();
/*N*/ }


/** this function searches in both the models pool and the styles pool for XLineStartItem
    and XLineEndItem with the same value or name and returns an item with the value of
    this item and a unique name for an item with this value. */
/*N*/ XLineEndItem* XLineEndItem::checkForUniqueItem( SdrModel* pModel ) const
/*N*/ {
/*N*/ 	if( pModel )
/*N*/ 	{
/*N*/ 		XLineEndItem* pTempItem = NULL;
/*N*/ 		const XLineEndItem* pLineEndItem = this;
/*N*/ 
/*N*/ 		String aUniqueName( GetName() );
/*N*/ 
/*N*/ 		if( aXPolygon.GetPointCount() == 0 )
/*N*/ 		{
/*N*/ 			// if the polygon is empty, check if the name is empty
/*N*/ 			if( aUniqueName.Len() == 0 )
/*N*/ 				return (XLineEndItem*)this;
/*N*/ 
/*N*/ 			// force empty name for empty polygons
/*N*/ 			return new XLineEndItem( String(), aXPolygon );
/*N*/ 		}
/*N*/ 
/*N*/ 		if( aXPolygon.GetPointCount() > 1 )
/*N*/ 		{
/*N*/ 			// check if the polygon is closed
/*N*/ 			if( aXPolygon[0] != aXPolygon[aXPolygon.GetPointCount() - 1] )
/*N*/ 			{
/*N*/ 				// force a closed polygon
/*N*/ 				XPolygon aNewPolygon( aXPolygon );
/*N*/ 				aNewPolygon[ aXPolygon.GetPointCount() ] = aXPolygon[0];
/*N*/ 				pTempItem = new XLineEndItem( aUniqueName, aNewPolygon );
/*N*/ 				pLineEndItem = pTempItem;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		sal_Bool bForceNew = sal_False;
/*N*/ 
/*N*/ 		// 2. if we have a name check if there is already an item with the
/*N*/ 		// same name in the documents pool with a different line end or start
/*N*/ 
/*N*/ 		sal_uInt16 nCount, nSurrogate;
/*N*/ 
/*N*/ 		const SfxItemPool* pPool1 = &pModel->GetItemPool();
/*N*/ 		if( aUniqueName.Len() && pPool1 )
/*N*/ 		{
/*N*/ 			nCount = pPool1->GetItemCount( XATTR_LINESTART );
/*N*/ 
/*N*/ 			for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
/*N*/ 			{
/*N*/ 				const XLineStartItem* pItem = (const XLineStartItem*)pPool1->GetItem( XATTR_LINESTART, nSurrogate );
/*N*/ 
/*N*/ 				if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
/*N*/ 				{
/*N*/ 					// if there is already an item with the same name and the same
/*N*/ 					// value its ok to set it
/*N*/ 					if( pItem->GetValue() != pLineEndItem->GetValue() )
/*N*/ 					{
/*N*/ 						// same name but different value, we need a new name for this item
/*N*/ 						aUniqueName = String();
/*N*/ 						bForceNew = sal_True;
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if( !bForceNew )
/*N*/ 			{
/*N*/ 				nCount = pPool1->GetItemCount( XATTR_LINEEND );
/*N*/ 
/*N*/ 				for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
/*N*/ 				{
/*N*/ 					const XLineEndItem* pItem = (const XLineEndItem*)pPool1->GetItem( XATTR_LINEEND, nSurrogate );
/*N*/ 
/*N*/ 					if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
/*N*/ 					{
/*N*/ 						// if there is already an item with the same name and the same
/*N*/ 						// value its ok to set it
/*N*/ 						if( pItem->GetValue() != pLineEndItem->GetValue() )
/*N*/ 						{
/*N*/ 							// same name but different value, we need a new name for this item
/*N*/ 							aUniqueName = String();
/*N*/ 							bForceNew = sal_True;
/*N*/ 						}
/*N*/ 						break;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		const SfxItemPool* pPool2 = pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL;
/*N*/ 		if( aUniqueName.Len() && pPool2)
/*N*/ 		{
/*N*/ 			nCount = pPool2->GetItemCount( XATTR_LINESTART );
/*N*/ 			for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
/*N*/ 			{
/*N*/ 				const XLineStartItem* pItem = (const XLineStartItem*)pPool2->GetItem( XATTR_LINESTART, nSurrogate );
/*N*/ 
/*N*/ 				if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
/*N*/ 				{
/*N*/ 					// if there is already an item with the same name and the same
/*N*/ 					// value its ok to set it
/*N*/ 					if( pItem->GetValue() != pLineEndItem->GetValue() )
/*N*/ 					{
/*N*/ 						// same name but different value, we need a new name for this item
/*N*/ 						aUniqueName = String();
/*N*/ 						bForceNew = sal_True;
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if( !bForceNew )
/*N*/ 			{
/*N*/ 				nCount = pPool2->GetItemCount( XATTR_LINEEND );
/*N*/ 				for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
/*N*/ 				{
/*N*/ 					const XLineEndItem* pItem = (const XLineEndItem*)pPool2->GetItem( XATTR_LINEEND, nSurrogate );
/*N*/ 
/*N*/ 					if( pItem && ( pItem->GetName() == pLineEndItem->GetName() ) )
/*N*/ 					{
/*N*/ 						// if there is already an item with the same name and the same
/*N*/ 						// value its ok to set it
/*N*/ 						if( pItem->GetValue() != pLineEndItem->GetValue() )
/*N*/ 						{
/*N*/ 							// same name but different value, we need a new name for this item
/*N*/ 							aUniqueName = String();
/*N*/ 							bForceNew = sal_True;
/*N*/ 						}
/*N*/ 						break;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// if we have no name yet, find existing item with same conent or
/*N*/ 		// create a unique name
/*N*/ 		if( aUniqueName.Len() == 0 )
/*N*/ 		{
/*N*/ 			sal_Bool bFoundExisting = sal_False;
/*N*/ 
/*N*/ 			sal_Int32 nUserIndex = 1;
/*N*/ 			const ResId aRes(SVX_RES(RID_SVXSTR_LINEEND));
/*N*/ 			const String aUser( aRes );
/*N*/ 
/*N*/ 			if( pPool1 )
/*N*/ 			{
/*N*/ 				nCount = pPool1->GetItemCount( XATTR_LINESTART );
/*N*/ 				for( sal_uInt16 nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
/*N*/ 				{
/*N*/ 					const XLineStartItem* pItem = (const XLineStartItem*)pPool1->GetItem( XATTR_LINESTART, nSurrogate );
/*N*/ 
/*N*/ 					if( pItem && pItem->GetName().Len() )
/*N*/ 					{
/*N*/ 						if( !bForceNew && pItem->GetValue() == pLineEndItem->GetValue() )
/*N*/ 						{
/*N*/ 							aUniqueName = pItem->GetName();
/*N*/ 							bFoundExisting = sal_True;
/*N*/ 							break;
/*N*/ 						}
/*N*/ 
/*N*/ 						if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
/*N*/ 						{
/*N*/ 							sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
/*N*/ 							if( nThisIndex >= nUserIndex )
/*N*/ 								nUserIndex = nThisIndex + 1;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				nCount = pPool1->GetItemCount( XATTR_LINEEND );
/*N*/ 				for( nSurrogate = 0; nSurrogate < nCount; nSurrogate++ )
/*N*/ 				{
/*N*/ 					const XLineEndItem* pItem = (const XLineEndItem*)pPool1->GetItem( XATTR_LINEEND, nSurrogate );
/*N*/ 
/*N*/ 					if( pItem && pItem->GetName().Len() )
/*N*/ 					{
/*N*/ 						if( !bForceNew && pItem->GetValue() == pLineEndItem->GetValue() )
/*N*/ 						{
/*N*/ 							aUniqueName = pItem->GetName();
/*N*/ 							bFoundExisting = sal_True;
/*N*/ 							break;
/*N*/ 						}
/*N*/ 
/*N*/ 						if( pItem->GetName().CompareTo( aUser, aUser.Len() ) == 0 )
/*N*/ 						{
/*N*/ 							sal_Int32 nThisIndex = pItem->GetName().Copy( aUser.Len() ).ToInt32();
/*N*/ 							if( nThisIndex >= nUserIndex )
/*N*/ 								nUserIndex = nThisIndex + 1;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			if( !bFoundExisting )
/*N*/ 			{
/*N*/ 				aUniqueName = aUser;
/*N*/ 				aUniqueName += sal_Unicode(' ');
/*N*/ 				aUniqueName += String::CreateFromInt32( nUserIndex );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// if the given name is not valid, replace it!
/*N*/ 		if( aUniqueName != GetName() || pTempItem )
/*N*/ 		{
/*N*/ 			if( pTempItem )
/*N*/ 			{
/*N*/ 				pTempItem->SetName( aUniqueName );
/*N*/ 				return pTempItem;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				return new XLineEndItem( aUniqueName, aXPolygon );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return (XLineEndItem*)this;
/*N*/ }


//------------------------------------------------------------------------


/*N*/ bool XLineEndItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	if( nMemberId == MID_NAME )
/*N*/ 	{
/*N*/ 		::rtl::OUString aApiName;
/*N*/ 		SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
/*N*/ 		rVal <<= aApiName;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		::com::sun::star::drawing::PolyPolygonBezierCoords aBezier;
/*N*/ 		SvxConvertXPolygonToPolyPolygonBezier( aXPolygon, aBezier );
/*N*/ 		rVal <<= aBezier;
/*N*/ 	}
/*N*/ #endif
/*N*/ 	return true;
/*N*/ }

/*N*/ bool XLineEndItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	if( nMemberId == MID_NAME )
/*N*/ 	{
/*N*/ 		return false;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		aXPolygon.SetSize( 0 );
/*N*/ 		if( rVal.hasValue() && rVal.getValue() )
/*N*/ 		{
/*N*/ 			if( rVal.getValueType() != ::getCppuType((const ::com::sun::star::drawing::PolyPolygonBezierCoords*)0) )
/*N*/ 				return false;
/*N*/
/*N*/ 			aXPolygon.SetSize(0);
/*N*/ 			::com::sun::star::drawing::PolyPolygonBezierCoords* pCoords = (::com::sun::star::drawing::PolyPolygonBezierCoords*)rVal.getValue();
/*N*/ 			if( pCoords->Coordinates.getLength() > 0 )
/*N*/ 				SvxConvertPolyPolygonBezierToXPolygon( pCoords, aXPolygon );
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return true;
/*N*/ }

// ----------------------------
// class XLineStartWidthItem
// ----------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XLineStartWidthItem, SfxMetricItem);

/*************************************************************************
|*
|*	  XLineStartWidthItem::XLineStartWidthItem(long nWidth)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ XLineStartWidthItem::XLineStartWidthItem(long nWidth) :
/*N*/ 	SfxMetricItem(XATTR_LINESTARTWIDTH, nWidth)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineStartWidthItem::XLineStartWidthItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XLineStartWidthItem::XLineStartWidthItem(SvStream& rIn) :
/*N*/ 	SfxMetricItem(XATTR_LINESTARTWIDTH, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineStartWidthItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineStartWidthItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XLineStartWidthItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XLineStartWidthItem::Create(SvStream& rIn, USHORT nVer)
|*												const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineStartWidthItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XLineStartWidthItem(rIn);
/*N*/ }

//------------------------------------------------------------------------


/*N*/ bool XLineStartWidthItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (sal_Int32)GetValue();
/*N*/ 	return true;
/*N*/ }

/*N*/ bool XLineStartWidthItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Int32 nValue;
/*N*/ 	rVal >>= nValue;
/*N*/ 	SetValue( nValue );
/*N*/ 	return true;
/*N*/ }



// --------------------------
// class XLineEndWidthItem
// --------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XLineEndWidthItem, SfxMetricItem);

/*************************************************************************
|*
|*	  XLineEndWidthItem::XLineEndWidthItem(long nWidth)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XLineEndWidthItem::XLineEndWidthItem(long nWidth) :
/*N*/    SfxMetricItem(XATTR_LINEENDWIDTH, nWidth)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineEndWidthItem::XLineEndWidthItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XLineEndWidthItem::XLineEndWidthItem(SvStream& rIn) :
/*N*/ 	SfxMetricItem(XATTR_LINEENDWIDTH, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineEndWidthItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineEndWidthItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XLineEndWidthItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XLineEndWidthItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineEndWidthItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XLineEndWidthItem(rIn);
/*N*/ }

//------------------------------------------------------------------------


/*N*/ bool XLineEndWidthItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (sal_Int32)GetValue();
/*N*/ 	return true;
/*N*/ }

/*N*/ bool XLineEndWidthItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Int32 nValue;
/*N*/ 	rVal >>= nValue;
/*N*/ 	SetValue( nValue );
/*N*/ 	return true;
/*N*/ }


// -----------------------------
// class XLineStartCenterItem
// -----------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XLineStartCenterItem, SfxBoolItem);

/*************************************************************************
|*
|*	  XLineStartCenterItem::XLineStartCenterItem(BOOL bStartCenter)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XLineStartCenterItem::XLineStartCenterItem(BOOL bStartCenter) :
/*N*/ 	SfxBoolItem(XATTR_LINESTARTCENTER, bStartCenter)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineStartCenterItem::XLineStartCenterItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XLineStartCenterItem::XLineStartCenterItem(SvStream& rIn) :
/*N*/ 	SfxBoolItem(XATTR_LINESTARTCENTER, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineStartCenterItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineStartCenterItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XLineStartCenterItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XLineStartCenterItem::Create(SvStream& rIn, USHORT nVer)
|*												const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineStartCenterItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XLineStartCenterItem(rIn);
/*N*/ }

//------------------------------------------------------------------------


/*N*/ bool XLineStartCenterItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	sal_Bool bValue = GetValue();
/*N*/ 	rVal.setValue( &bValue, ::getCppuBooleanType()  );
/*N*/ 	return true;
/*N*/ }

/*N*/ bool XLineStartCenterItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	if( !rVal.hasValue() || rVal.getValueType() != ::getCppuBooleanType() )
/*N*/ 		return false;
/*N*/
/*N*/ 	SetValue( *(sal_Bool*)rVal.getValue() );
/*N*/ 	return true;
/*N*/ }


// ---------------------------
// class XLineEndCenterItem
// ---------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XLineEndCenterItem, SfxBoolItem);

/*************************************************************************
|*
|*	  XLineEndCenterItem::XLineEndCenterItem(BOOL bEndCenter)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XLineEndCenterItem::XLineEndCenterItem(BOOL bEndCenter) :
/*N*/ 	SfxBoolItem(XATTR_LINEENDCENTER, bEndCenter)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineEndCenterItem::XLineEndCenterItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XLineEndCenterItem::XLineEndCenterItem(SvStream& rIn) :
/*N*/ 	SfxBoolItem(XATTR_LINEENDCENTER, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XLineEndCenterItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineEndCenterItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XLineEndCenterItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XLineEndCenterItem::Create(SvStream& rIn, USHORT nVer)
|*												const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XLineEndCenterItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XLineEndCenterItem(rIn);
/*N*/ }

//------------------------------------------------------------------------


/*N*/ bool XLineEndCenterItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	sal_Bool bValue = GetValue();
/*N*/ 	rVal.setValue( &bValue, ::getCppuBooleanType()  );
/*N*/ 	return true;
/*N*/ }

/*N*/ bool XLineEndCenterItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	if( !rVal.hasValue() || rVal.getValueType() != ::getCppuBooleanType() )
/*N*/ 		return false;
/*N*/
/*N*/ 	SetValue( *(sal_Bool*)rVal.getValue() );
/*N*/ 	return true;
/*N*/ }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//							 Fuellattribute
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// --------------------
// class XFillStyleItem
// --------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFillStyleItem, SfxEnumItem);

/*************************************************************************
|*
|*	  XFillStyleItem::XFillStyleItem(XFillStyle eFillStyle)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XFillStyleItem::XFillStyleItem(XFillStyle eFillStyle) :
/*N*/ 	SfxEnumItem(XATTR_FILLSTYLE, eFillStyle)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillStyleItem::XFillStyleItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XFillStyleItem::XFillStyleItem(SvStream& rIn) :
/*N*/ 	SfxEnumItem(XATTR_FILLSTYLE, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillStyleItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	09.11.94
|*	  Letzte Aenderung	09.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillStyleItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFillStyleItem( *this );
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFillStyleItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillStyleItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFillStyleItem(rIn);
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------

/*N*/ USHORT XFillStyleItem::GetValueCount() const
/*N*/ {
/*N*/ 	return 5;
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ bool XFillStyleItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	::com::sun::star::drawing::FillStyle eFS = (::com::sun::star::drawing::FillStyle)GetValue();
/*N*/
/*N*/ 	rVal <<= eFS;
/*N*/
/*N*/ 	return true;
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ bool XFillStyleItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     ::com::sun::star::drawing::FillStyle eFS;
/*N*/ 	if(!(rVal >>= eFS))
/*N*/     {
/*?*/         // also try an int (for Basic)
/*?*/         sal_Int32 nFS;
/*?*/         if(!(rVal >>= nFS))
/*?*/             return false;
/*?*/         eFS = (::com::sun::star::drawing::FillStyle)nFS;
/*N*/     }
/*N*/
/*N*/ 	SetValue( (XFillStyle)eFS );
/*N*/
/*N*/ 	return true;
/*N*/ }


// -------------------
// class XFillColorItem
// -------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFillColorItem, XColorItem);

/*************************************************************************
|*
|*	  XFillColorItem::XFillColorItem(const XubString& rName, const Color& rTheColor)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XFillColorItem::XFillColorItem(const XubString& rName, const Color& rTheColor) :
/*N*/ 	XColorItem(XATTR_FILLCOLOR, rName, rTheColor)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillColorItem::XFillColorItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XFillColorItem::XFillColorItem(SvStream& rIn) :
/*N*/ 	XColorItem(XATTR_FILLCOLOR, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillColorItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillColorItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFillColorItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFillColorItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillColorItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFillColorItem(rIn);
/*N*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ bool XFillColorItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (sal_Int32)GetValue().GetRGBColor();
/*N*/
/*N*/ 	return true;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ bool XFillColorItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Int32 nValue;
/*N*/ 	if(!(rVal >>= nValue ))
/*N*/ 		return false;
/*N*/
/*N*/ 	SetValue( nValue );
/*N*/ 	return true;
/*N*/ }



// ----------------
// class XGradient
// ----------------

/*************************************************************************
|*
|*	  XGradient::XGradient(XGradientStyle, const Color&, const Color&,
|*						   long, USHORT, USHORT, USHORT)
|*
|*	  Beschreibung
|*	  Ersterstellung	21.11.94
|*	  Letzte Aenderung	21.11.94
|*
*************************************************************************/

/*N*/ XGradient::XGradient(const Color& rStart, const Color& rEnd,
/*N*/ 					 XGradientStyle eTheStyle, long nTheAngle, USHORT nXOfs,
/*N*/ 					 USHORT nYOfs, USHORT nTheBorder,
/*N*/ 					 USHORT nStartIntens, USHORT nEndIntens,
/*N*/ 					 USHORT nSteps) :
/*N*/ 	aStartColor(rStart),
/*N*/ 	aEndColor(rEnd),
/*N*/ 	eStyle(eTheStyle),
/*N*/ 	nAngle(nTheAngle),
/*N*/ 	nBorder(nTheBorder),
/*N*/ 	nOfsX(nXOfs),
/*N*/ 	nOfsY(nYOfs),
/*N*/ 	nIntensStart(nStartIntens),
/*N*/ 	nIntensEnd(nEndIntens),
/*N*/ 	nStepCount(nSteps)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  int XGradient::operator==(const SfxPoolItem& rItem) const
|*
|*	  Beschreibung
|*	  Ersterstellung	29.11.94
|*	  Letzte Aenderung	29.11.94
|*
*************************************************************************/

/*N*/ int XGradient::operator==(const XGradient& rGradient) const
/*N*/ {
/*N*/ 	return ( eStyle 		== rGradient.eStyle 		&&
/*N*/ 			 aStartColor	== rGradient.aStartColor	&&
/*N*/ 			 aEndColor		== rGradient.aEndColor		&&
/*N*/ 			 nAngle 		== rGradient.nAngle 		&&
/*N*/ 			 nBorder		== rGradient.nBorder		&&
/*N*/ 			 nOfsX			== rGradient.nOfsX			&&
/*N*/ 			 nOfsY			== rGradient.nOfsY			&&
/*N*/ 			 nIntensStart	== rGradient.nIntensStart	&&
/*N*/ 			 nIntensEnd		== rGradient.nIntensEnd		&&
/*N*/ 			 nStepCount		== rGradient.nStepCount );
/*N*/ }


// -----------------------
// class XFillGradientItem
// -----------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFillGradientItem, NameOrIndex);

/*************************************************************************
|*
|*	  XFillGradientItem::XFillGradientItem(long nIndex,
|*										 const XGradient& rTheGradient)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XFillGradientItem::XFillGradientItem(long nIndex,
/*N*/ 								   const XGradient& rTheGradient) :
/*N*/ 	NameOrIndex(XATTR_FILLGRADIENT, nIndex),
/*N*/ 	aGradient(rTheGradient)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillGradientItem::XFillGradientItem(const XubString& rName,
|*										 const XGradient& rTheGradient)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XFillGradientItem::XFillGradientItem(const XubString& rName,
/*N*/ 								   const XGradient& rTheGradient) :
/*N*/ 	NameOrIndex(XATTR_FILLGRADIENT, rName),
/*N*/ 	aGradient(rTheGradient)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillGradientItem::XFillGradientItem(const XFillGradientItem& rItem)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XFillGradientItem::XFillGradientItem(const XFillGradientItem& rItem) :
/*N*/ 	NameOrIndex(rItem),
/*N*/ 	aGradient(rItem.aGradient)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillGradientItem::XFillGradientItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XFillGradientItem::XFillGradientItem(SvStream& rIn, USHORT nVer) :
/*N*/ 	NameOrIndex(XATTR_FILLGRADIENT, rIn),
/*N*/ 	aGradient(COL_BLACK, COL_WHITE)
/*N*/ {
/*N*/ 	if (!IsIndex())
/*N*/ 	{
/*N*/ 		USHORT nUSTemp;
/*N*/ 		USHORT nRed;
/*N*/ 		USHORT nGreen;
/*N*/ 		USHORT nBlue;
/*N*/ 		INT16  nITemp;
/*N*/ 		INT32  nLTemp;
/*N*/ 
/*N*/ 		rIn >> nITemp; aGradient.SetGradientStyle((XGradientStyle)nITemp);
/*N*/ 		rIn >> nRed;
/*N*/ 		rIn >> nGreen;
/*N*/ 		rIn >> nBlue;
/*N*/ 		Color aCol;
/*N*/ 		aCol = Color( (BYTE)( nRed >> 8 ), (BYTE)( nGreen >> 8 ), (BYTE)( nBlue >> 8 ) );
/*N*/ 		aGradient.SetStartColor( aCol );
/*N*/ 
/*N*/ 		rIn >> nRed;
/*N*/ 		rIn >> nGreen;
/*N*/ 		rIn >> nBlue;
/*N*/ 		aCol = Color( (BYTE)( nRed >> 8 ), (BYTE)( nGreen >> 8 ), (BYTE)( nBlue >> 8 ) );
/*N*/ 		aGradient.SetEndColor(aCol);
/*N*/ 		rIn >> nLTemp; aGradient.SetAngle(nLTemp);
/*N*/ 		rIn >> nUSTemp; aGradient.SetBorder(nUSTemp);
/*N*/ 		rIn >> nUSTemp; aGradient.SetXOffset(nUSTemp);
/*N*/ 		rIn >> nUSTemp; aGradient.SetYOffset(nUSTemp);
/*N*/ 		rIn >> nUSTemp; aGradient.SetStartIntens(nUSTemp);
/*N*/ 		rIn >> nUSTemp; aGradient.SetEndIntens(nUSTemp);
/*N*/ 
/*N*/ 		// bei neueren Versionen wird zusaetzlich
/*N*/ 		// die Schrittweite mit eingelesen
/*N*/ 		if (nVer >= 1)
/*N*/ 		{
/*N*/ 			rIn >> nUSTemp; aGradient.SetSteps(nUSTemp);
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//*************************************************************************

/*N*/ XFillGradientItem::XFillGradientItem(SfxItemPool* pPool, const XGradient& rTheGradient)
/*N*/ : 	NameOrIndex( XATTR_FILLGRADIENT, -1 ),
/*N*/ 	aGradient(rTheGradient)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillGradientItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillGradientItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFillGradientItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  int XFillGradientItem::operator==(const SfxPoolItem& rItem) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ int XFillGradientItem::operator==(const SfxPoolItem& rItem) const
/*N*/ {
/*N*/ 	return ( NameOrIndex::operator==(rItem) &&
/*N*/ 			 aGradient == ((const XFillGradientItem&) rItem).aGradient );
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFillGradientItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillGradientItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFillGradientItem(rIn, nVer);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFillGradientItem::Store(SvStream& rOut) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SvStream& XFillGradientItem::Store( SvStream& rOut, USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	NameOrIndex::Store( rOut, nItemVersion );
/*N*/ 
/*N*/ 	if (!IsIndex())
/*N*/ 	{
/*N*/ 		rOut << (INT16)aGradient.GetGradientStyle();
/*N*/ 
/*N*/ 		USHORT nTmp;
/*N*/ 
/*N*/ 		nTmp = VCLTOSVCOL( aGradient.GetStartColor().GetRed() ); rOut << nTmp;
/*N*/ 		nTmp = VCLTOSVCOL( aGradient.GetStartColor().GetGreen() ); rOut << nTmp;
/*N*/ 		nTmp = VCLTOSVCOL( aGradient.GetStartColor().GetBlue() ); rOut << nTmp;
/*N*/ 		nTmp = VCLTOSVCOL( aGradient.GetEndColor().GetRed() ); rOut << nTmp;
/*N*/ 		nTmp = VCLTOSVCOL( aGradient.GetEndColor().GetGreen() ); rOut << nTmp;
/*N*/ 		nTmp = VCLTOSVCOL( aGradient.GetEndColor().GetBlue() ); rOut << nTmp;
/*N*/ 
/*N*/ 		rOut << (INT32) aGradient.GetAngle();
/*N*/ 		rOut << aGradient.GetBorder();
/*N*/ 		rOut << aGradient.GetXOffset();
/*N*/ 		rOut << aGradient.GetYOffset();
/*N*/ 		rOut << aGradient.GetStartIntens();
/*N*/ 		rOut << aGradient.GetEndIntens();
/*N*/ 		rOut << aGradient.GetSteps();
/*N*/ 	}
/*N*/ 
/*N*/ 	return rOut;
/*N*/ }

/*************************************************************************
|*
|*	  const XGradient& XFillGradientItem::GetValue(const XGradientTable* pTable)
|*																   const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ const XGradient& XFillGradientItem::GetValue(const XGradientTable* pTable) const
/*N*/ {
/*N*/ 	if (!IsIndex())
/*N*/ 		return aGradient;
/*N*/ 	else
/*?*/ 		return pTable->Get(GetIndex())->GetGradient();
/*N*/ }


/*************************************************************************
|*
|*	  USHORT XFillGradientItem::GetVersion() const
|*
|*	  Beschreibung
|*	  Ersterstellung	01.11.95
|*	  Letzte Aenderung	01.11.95
|*
*************************************************************************/

/*N*/ USHORT XFillGradientItem::GetVersion( USHORT nFileFormatVersion ) const
/*N*/ {
/*N*/ 	// !!! this version number also represents the version number of superclasses
/*N*/ 	// !!! (e.g. XFillFloatTransparenceItem); if you make any changes here,
/*N*/ 	// !!! the superclass is also affected
/*N*/ 	return 1;
/*N*/ }

//------------------------------------------------------------------------


// -----------------------------------------------------------------------
/*N*/ bool XFillGradientItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/ 	switch ( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_FILLGRADIENT:
/*N*/ 		{
/*N*/ 			const XGradient& aXGradient = GetValue();
/*N*/ 			::com::sun::star::awt::Gradient aGradient;
/*N*/
/*N*/ 			aGradient.Style = (::com::sun::star::awt::GradientStyle) aXGradient.GetGradientStyle();
/*N*/ 			aGradient.StartColor = (INT32)aXGradient.GetStartColor().GetColor();
/*N*/ 			aGradient.EndColor = (INT32)aXGradient.GetEndColor().GetColor();
/*N*/ 			aGradient.Angle = (short)aXGradient.GetAngle();
/*N*/ 			aGradient.Border = aXGradient.GetBorder();
/*N*/ 			aGradient.XOffset = aXGradient.GetXOffset();
/*N*/ 			aGradient.YOffset = aXGradient.GetYOffset();
/*N*/ 			aGradient.StartIntensity = aXGradient.GetStartIntens();
/*N*/ 			aGradient.EndIntensity = aXGradient.GetEndIntens();
/*N*/ 			aGradient.StepCount = aXGradient.GetSteps();
/*N*/
/*N*/ 			rVal <<= aGradient;
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_NAME:
/*N*/ 		{
/*N*/ 			::rtl::OUString aApiName;
/*N*/ 			SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
/*N*/ 			rVal <<= aApiName;
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_GRADIENT_STYLE: rVal <<= (sal_Int16)GetValue().GetGradientStyle(); break;
/*N*/ 		case MID_GRADIENT_STARTCOLOR: rVal <<= (sal_Int32)GetValue().GetStartColor().GetColor(); break;
/*N*/ 		case MID_GRADIENT_ENDCOLOR: rVal <<= (sal_Int32)GetValue().GetEndColor().GetColor(); break;
/*N*/ 		case MID_GRADIENT_ANGLE: rVal <<= (sal_Int16)GetValue().GetAngle(); break;
/*N*/ 		case MID_GRADIENT_BORDER: rVal <<= GetValue().GetBorder(); break;
/*N*/ 		case MID_GRADIENT_XOFFSET: rVal <<= GetValue().GetXOffset(); break;
/*N*/ 		case MID_GRADIENT_YOFFSET: rVal <<= GetValue().GetYOffset(); break;
/*N*/ 		case MID_GRADIENT_STARTINTENSITY: rVal <<= GetValue().GetStartIntens(); break;
/*N*/ 		case MID_GRADIENT_ENDINTENSITY: rVal <<= GetValue().GetEndIntens(); break;
/*N*/ 		case MID_GRADIENT_STEPCOUNT: rVal <<= GetValue().GetSteps(); break;
/*N*/
/*N*/ 		default: DBG_ERROR("Wrong MemberId!"); return false;
/*N*/ 	}
/*N*/
/*N*/ 	return true;
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ bool XFillGradientItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/
/*N*/ 	switch ( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_NAME:
/*N*/ 		{
/*?*/ 			::rtl::OUString aName;
/*?*/ 			if (!(rVal >>= aName ))
/*?*/ 				return sal_False;
/*?*/ 			SetName( aName );
/*?*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_FILLGRADIENT:
/*N*/ 		{
/*N*/ 			::com::sun::star::awt::Gradient aGradient;
/*N*/ 			if(!(rVal >>= aGradient))
/*N*/ 				return sal_False;
/*N*/
/*N*/ 			XGradient aXGradient;
/*N*/
/*N*/ 			aXGradient.SetGradientStyle( (XGradientStyle) aGradient.Style );
/*N*/ 			aXGradient.SetStartColor( aGradient.StartColor );
/*N*/ 			aXGradient.SetEndColor( aGradient.EndColor );
/*N*/ 			aXGradient.SetAngle( aGradient.Angle );
/*N*/ 			aXGradient.SetBorder( aGradient.Border );
/*N*/ 			aXGradient.SetXOffset( aGradient.XOffset );
/*N*/ 			aXGradient.SetYOffset( aGradient.YOffset );
/*N*/ 			aXGradient.SetStartIntens( aGradient.StartIntensity );
/*N*/ 			aXGradient.SetEndIntens( aGradient.EndIntensity );
/*N*/ 			aXGradient.SetSteps( aGradient.StepCount );
/*N*/
/*N*/ 			SetValue( aXGradient );
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_GRADIENT_STARTCOLOR:
/*N*/ 		case MID_GRADIENT_ENDCOLOR:
/*N*/ 		{
/*?*/ 			sal_Int32 nVal;
/*?*/ 			if(!(rVal >>= nVal ))
/*?*/ 				return sal_False;
/*?*/
/*?*/ 			XGradient aXGradient = GetValue();
/*?*/
/*?*/ 			if ( nMemberId == MID_GRADIENT_STARTCOLOR )
/*?*/ 				aXGradient.SetStartColor( nVal );
/*?*/ 			else
/*?*/ 				aXGradient.SetEndColor( nVal );
/*?*/ 			SetValue( aXGradient );
/*?*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_GRADIENT_STYLE:
/*N*/ 		case MID_GRADIENT_ANGLE:
/*N*/ 		case MID_GRADIENT_BORDER:
/*N*/ 		case MID_GRADIENT_STARTINTENSITY:
/*N*/ 		case MID_GRADIENT_ENDINTENSITY:
/*N*/ 		case MID_GRADIENT_STEPCOUNT:
/*N*/ 		case MID_GRADIENT_XOFFSET:
/*N*/ 		case MID_GRADIENT_YOFFSET:
/*N*/ 		{
/*?*/ 			sal_Int16 nVal;
/*?*/ 			if(!(rVal >>= nVal ))
/*?*/ 				return sal_False;
/*?*/
/*?*/ 			XGradient aXGradient = GetValue();
/*?*/
/*?*/ 			switch ( nMemberId )
/*?*/ 			{
/*?*/ 				case MID_GRADIENT_STYLE:
/*?*/ 					aXGradient.SetGradientStyle( (XGradientStyle)nVal ); break;
/*?*/ 				case MID_GRADIENT_ANGLE:
/*?*/ 					aXGradient.SetAngle( nVal ); break;
/*?*/ 				case MID_GRADIENT_BORDER:
/*?*/ 					aXGradient.SetBorder( nVal ); break;
/*?*/ 				case MID_GRADIENT_STARTINTENSITY:
/*?*/ 					aXGradient.SetStartIntens( nVal ); break;
/*?*/ 				case MID_GRADIENT_ENDINTENSITY:
/*?*/ 					aXGradient.SetEndIntens( nVal ); break;
/*?*/ 				case MID_GRADIENT_STEPCOUNT:
/*?*/ 					aXGradient.SetSteps( nVal ); break;
/*?*/ 				case MID_GRADIENT_XOFFSET:
/*?*/ 					aXGradient.SetXOffset( nVal ); break;
/*?*/ 				case MID_GRADIENT_YOFFSET:
/*?*/ 					aXGradient.SetYOffset( nVal ); break;
/*?*/ 			}
/*?*/
/*?*/ 			SetValue( aXGradient );
/*?*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ BOOL XFillGradientItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
/*N*/ {
/*N*/ 	return ((XFillGradientItem*)p1)->GetValue() == ((XFillGradientItem*)p2)->GetValue();
/*N*/ }

/*N*/ XFillGradientItem* XFillGradientItem::checkForUniqueItem( SdrModel* pModel ) const
/*N*/ {
/*N*/ 	if( pModel )
/*N*/ 	{
/*N*/ 		const String aUniqueName = NameOrIndex::CheckNamedItem(	this,
/*N*/ 																XATTR_FILLGRADIENT,
/*N*/ 																&pModel->GetItemPool(),
/*N*/ 																pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
/*N*/ 																XFillGradientItem::CompareValueFunc,
/*N*/ 																RID_SVXSTR_GRADIENT,
/*N*/ 																pModel->GetGradientList() );
/*N*/ 
/*N*/ 		// if the given name is not valid, replace it!
/*N*/ 		if( aUniqueName != GetName() )
/*N*/ 		{
/*N*/ 			return new XFillGradientItem( aUniqueName, aGradient );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return (XFillGradientItem*)this;
/*N*/ }

// ----------------------------------
// class XFillFloatTransparenceItem -
// ----------------------------------

/*N*/ TYPEINIT1_AUTOFACTORY( XFillFloatTransparenceItem, XFillGradientItem );

// -----------------------------------------------------------------------------

/*N*/ XFillFloatTransparenceItem::XFillFloatTransparenceItem() :
/*N*/ 	bEnabled( FALSE )
/*N*/ {
/*N*/ 	SetWhich( XATTR_FILLFLOATTRANSPARENCE );
/*N*/ }

//------------------------------------------------------------------------

/*?*/ XFillFloatTransparenceItem::XFillFloatTransparenceItem(const XubString& rName, const XGradient& rGradient, BOOL bEnable ) :
/*?*/ 	XFillGradientItem	( rName, rGradient ),
/*?*/ 	bEnabled			( bEnable )
/*?*/ {
/*?*/ 	SetWhich( XATTR_FILLFLOATTRANSPARENCE );
/*?*/ }

//------------------------------------------------------------------------

/*N*/ XFillFloatTransparenceItem::XFillFloatTransparenceItem( const XFillFloatTransparenceItem& rItem ) :
/*N*/ 	XFillGradientItem	( rItem ),
/*N*/ 	bEnabled			( rItem.bEnabled )
/*N*/ {
/*N*/ 	SetWhich( XATTR_FILLFLOATTRANSPARENCE );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ XFillFloatTransparenceItem::XFillFloatTransparenceItem( SvStream& rIn, USHORT nVer ) :
/*N*/ 	XFillGradientItem	( rIn, nVer )
/*N*/ {
/*N*/ 	SetWhich( XATTR_FILLFLOATTRANSPARENCE );
/*N*/ 	rIn >> bEnabled;
/*N*/ }

//*************************************************************************

/*N*/ XFillFloatTransparenceItem::XFillFloatTransparenceItem(SfxItemPool* pPool, const XGradient& rTheGradient, BOOL bEnable )
/*N*/ : 	XFillGradientItem	( -1, rTheGradient ),
/*N*/ 	bEnabled			( bEnable )
/*N*/ {
/*N*/ 	SetWhich( XATTR_FILLFLOATTRANSPARENCE );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ int XFillFloatTransparenceItem::operator==( const SfxPoolItem& rItem ) const
/*N*/ {
/*N*/ 	return ( NameOrIndex::operator==(rItem) ) &&
/*N*/ 			( GetValue() == ((const XFillGradientItem&)rItem).GetValue() ) &&
/*N*/ 		     ( bEnabled == ( (XFillFloatTransparenceItem&) rItem ).bEnabled );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SfxPoolItem* XFillFloatTransparenceItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XFillFloatTransparenceItem( *this );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SfxPoolItem* XFillFloatTransparenceItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return( ( 0 == nVer ) ? Clone( NULL ) : new XFillFloatTransparenceItem( rIn, nVer ) );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ SvStream& XFillFloatTransparenceItem::Store( SvStream& rOut, USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	XFillGradientItem::Store( rOut, nItemVersion );
/*N*/ 	rOut << bEnabled;
/*N*/ 	return rOut;
/*N*/ }

//------------------------------------------------------------------------

/*N*/ USHORT XFillFloatTransparenceItem::GetVersion( USHORT nFileFormatVersion ) const
/*N*/ {
/*N*/ 	// !!! if version number of this object must be increased, please	!!!
/*N*/ 	// !!! increase version number of base class XFillGradientItem		!!!
/*N*/ 	return XFillGradientItem::GetVersion( nFileFormatVersion );
/*N*/ }

//------------------------------------------------------------------------

/*N*/ bool XFillFloatTransparenceItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	return XFillGradientItem::QueryValue( rVal, nMemberId );
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------



/*N*/ XFillFloatTransparenceItem* XFillFloatTransparenceItem::checkForUniqueItem( SdrModel* pModel ) const
/*N*/ {
/*N*/ 	// #85953# unique name only necessary when enabled
/*N*/ 	if(IsEnabled())
/*N*/ 	{
/*?*/ 		if( pModel )
/*?*/ 		{
/*?*/ 			const String aUniqueName = NameOrIndex::CheckNamedItem(	this,
/*?*/ 																	XATTR_FILLFLOATTRANSPARENCE,
/*?*/ 																	&pModel->GetItemPool(),
/*?*/ 																	pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
/*?*/ 																	XFillFloatTransparenceItem::CompareValueFunc,
/*?*/ 																	RID_SVXSTR_TRASNGR0,
/*?*/ 																	NULL );
/*?*/ 
/*?*/ 			// if the given name is not valid, replace it!
/*?*/ 			if( aUniqueName != GetName() )
/*?*/ 			{
/*?*/ 				return new XFillFloatTransparenceItem( aUniqueName, GetValue(), TRUE );
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// #85953# if disabled, force name to empty string
/*N*/ 		if(GetName().Len())
/*N*/ 		{
/*?*/ 			return new XFillFloatTransparenceItem(String(), GetValue(), FALSE);
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return (XFillFloatTransparenceItem*)this;
/*N*/ }

// -------------
// class XHatch
// -------------

/*************************************************************************
|*
|*	  XHatch::XHatch(XHatchStyle, const Color&, long, long)
|*
|*	  Beschreibung
|*	  Ersterstellung	21.11.94
|*	  Letzte Aenderung	21.11.94
|*
*************************************************************************/

/*N*/ XHatch::XHatch(const Color& rCol, XHatchStyle eTheStyle, long nTheDistance,
/*N*/ 			   long nTheAngle) :
/*N*/ 	aColor(rCol),
/*N*/ 	eStyle(eTheStyle),
/*N*/ 	nDistance(nTheDistance),
/*N*/ 	nAngle(nTheAngle)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  int XHatch::operator==(const SfxPoolItem& rItem) const
|*
|*	  Beschreibung
|*	  Ersterstellung	29.11.94
|*	  Letzte Aenderung	29.11.94
|*
*************************************************************************/

/*N*/ int XHatch::operator==(const XHatch& rHatch) const
/*N*/ {
/*N*/ 	return ( eStyle 	== rHatch.eStyle	&&
/*N*/ 			 aColor 	== rHatch.aColor	&&
/*N*/ 			 nDistance	== rHatch.nDistance &&
/*N*/ 			 nAngle 	== rHatch.nAngle );
/*N*/ }


// -----------------------
// class XFillHatchItem
// -----------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFillHatchItem, NameOrIndex);

/*************************************************************************
|*
|*	  XFillHatchItem::XFillHatchItem(const XubString& rName,
|*								   const XHatch& rTheHatch)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XFillHatchItem::XFillHatchItem(const XubString& rName,
/*N*/ 							 const XHatch& rTheHatch) :
/*N*/ 	NameOrIndex(XATTR_FILLHATCH, rName),
/*N*/ 	aHatch(rTheHatch)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillHatchItem::XFillHatchItem(const XFillHatchItem& rItem)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XFillHatchItem::XFillHatchItem(const XFillHatchItem& rItem) :
/*N*/ 	NameOrIndex(rItem),
/*N*/ 	aHatch(rItem.aHatch)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillHatchItem::XFillHatchItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ XFillHatchItem::XFillHatchItem(SvStream& rIn) :
/*N*/ 	NameOrIndex(XATTR_FILLHATCH, rIn),
/*N*/ 	aHatch(COL_BLACK)
/*N*/ {
/*N*/ 	if (!IsIndex())
/*N*/ 	{
/*N*/ 		USHORT nRed;
/*N*/ 		USHORT nGreen;
/*N*/ 		USHORT nBlue;
/*N*/ 		INT16  nITemp;
/*N*/ 		INT32  nLTemp;
/*N*/ 
/*N*/ 		rIn >> nITemp; aHatch.SetHatchStyle((XHatchStyle)nITemp);
/*N*/ 		rIn >> nRed;
/*N*/ 		rIn >> nGreen;
/*N*/ 		rIn >> nBlue;
/*N*/ 
/*N*/ 		Color aCol;
/*N*/ 		aCol = Color( (BYTE)( nRed >> 8 ), (BYTE)( nGreen >> 8 ), (BYTE)( nBlue >> 8 ) );
/*N*/ 		aHatch.SetColor(aCol);
/*N*/ 		rIn >> nLTemp; aHatch.SetDistance(nLTemp);
/*N*/ 		rIn >> nLTemp; aHatch.SetAngle(nLTemp);
/*N*/ 	}
/*N*/ }

//*************************************************************************

/*N*/ XFillHatchItem::XFillHatchItem(SfxItemPool* pPool, const XHatch& rTheHatch)
/*N*/ : 	NameOrIndex( XATTR_FILLHATCH, -1 ),
/*N*/ 	aHatch(rTheHatch)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFillHatchItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillHatchItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFillHatchItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  int XFillHatchItem::operator==(const SfxPoolItem& rItem) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ int XFillHatchItem::operator==(const SfxPoolItem& rItem) const
/*N*/ {
/*N*/ 	return ( NameOrIndex::operator==(rItem) &&
/*N*/ 			 aHatch == ((const XFillHatchItem&) rItem).aHatch );
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFillHatchItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFillHatchItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFillHatchItem(rIn);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFillHatchItem::Store(SvStream& rOut) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	15.11.94
|*
*************************************************************************/

/*N*/ SvStream& XFillHatchItem::Store( SvStream& rOut, USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	NameOrIndex::Store( rOut, nItemVersion );
/*N*/ 
/*N*/ 	if (!IsIndex())
/*N*/ 	{
/*N*/ 		rOut << (INT16)aHatch.GetHatchStyle();
/*N*/ 
/*N*/ 		USHORT nTmp;
/*N*/ 		nTmp = VCLTOSVCOL( aHatch.GetColor().GetRed() ); rOut << nTmp;
/*N*/ 		nTmp = VCLTOSVCOL( aHatch.GetColor().GetGreen() ); rOut << nTmp;
/*N*/ 		nTmp = VCLTOSVCOL( aHatch.GetColor().GetBlue() ); rOut << nTmp;
/*N*/ 
/*N*/ 		rOut << (INT32) aHatch.GetDistance();
/*N*/ 		rOut << (INT32) aHatch.GetAngle();
/*N*/ 	}
/*N*/ 
/*N*/ 	return rOut;
/*N*/ }

/*************************************************************************
|*
|*	  const XHatch& XFillHatchItem::GetValue(const XHatchTable* pTable) const
|*
|*	  Beschreibung
|*	  Ersterstellung	15.11.94
|*	  Letzte Aenderung	18.11.94
|*
*************************************************************************/

/*N*/ const XHatch& XFillHatchItem::GetValue(const XHatchTable* pTable) const
/*N*/ {
/*N*/ 	if (!IsIndex())
/*N*/ 		return aHatch;
/*N*/ 	else
/*?*/ 		return pTable->Get(GetIndex())->GetHatch();
/*N*/ }

//------------------------------------------------------------------------


//------------------------------------------------------------------------


//------------------------------------------------------------------------


// -----------------------------------------------------------------------
/*N*/ bool XFillHatchItem::QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/
/*N*/ 	switch ( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_FILLHATCH:
/*N*/ 		{
/*N*/ 			::com::sun::star::drawing::Hatch aUnoHatch;
/*N*/
/*N*/ 			aUnoHatch.Style = (::com::sun::star::drawing::HatchStyle)aHatch.GetHatchStyle();
/*N*/ 			aUnoHatch.Color = aHatch.GetColor().GetColor();
/*N*/ 			aUnoHatch.Distance = aHatch.GetDistance();
/*N*/ 			aUnoHatch.Angle = aHatch.GetAngle();
/*N*/ 			rVal <<= aUnoHatch;
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_NAME:
/*N*/ 		{
/*N*/ 			::rtl::OUString aApiName;
/*N*/ 			SvxUnogetApiNameForItem( Which(), GetName(), aApiName );
/*N*/ 			rVal <<= aApiName;
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_HATCH_STYLE:
/*N*/ 			rVal <<= (::com::sun::star::drawing::HatchStyle)aHatch.GetHatchStyle(); break;
/*N*/ 		case MID_HATCH_COLOR:
/*N*/ 			rVal <<= (sal_Int32)aHatch.GetColor().GetColor(); break;
/*N*/ 		case MID_HATCH_DISTANCE:
/*N*/ 			rVal <<= aHatch.GetDistance(); break;
/*N*/ 		case MID_HATCH_ANGLE:
/*N*/ 			rVal <<= aHatch.GetAngle(); break;
/*N*/
/*N*/ 		default: DBG_ERROR("Wrong MemberId!"); return sal_False;
/*N*/ 	}
/*N*/
/*N*/ 	return true;
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ bool XFillHatchItem::PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/     sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
/*N*/     nMemberId &= ~CONVERT_TWIPS;
/*N*/
/*N*/ 	switch ( nMemberId )
/*N*/ 	{
/*N*/ 		case MID_FILLHATCH:
/*N*/ 		{
/*N*/ 			::com::sun::star::drawing::Hatch aUnoHatch;
/*N*/ 			if(!(rVal >>= aUnoHatch))
/*N*/ 				return sal_False;
/*N*/
/*N*/ 			aHatch.SetHatchStyle( (XHatchStyle)aUnoHatch.Style );
/*N*/ 			aHatch.SetColor( aUnoHatch.Color );
/*N*/ 			aHatch.SetDistance( aUnoHatch.Distance );
/*N*/ 			aHatch.SetAngle( aUnoHatch.Angle );
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_NAME:
/*N*/ 		{
/*?*/ 			::rtl::OUString aName;
/*?*/ 			if (!(rVal >>= aName ))
/*?*/ 				return sal_False;
/*?*/ 			SetName( aName );
/*?*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_HATCH_STYLE:
/*N*/ 		{
/*?*/ 			sal_Int16 nVal;
/*?*/ 			if (!(rVal >>= nVal ))
/*?*/ 				return sal_False;
/*?*/ 			aHatch.SetHatchStyle( (XHatchStyle)nVal );
/*?*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		case MID_HATCH_COLOR:
/*N*/ 		case MID_HATCH_DISTANCE:
/*N*/ 		case MID_HATCH_ANGLE:
/*N*/ 		{
/*?*/ 			sal_Int32 nVal;
/*?*/ 			if (!(rVal >>= nVal ))
/*?*/ 				return sal_False;
/*?*/
/*?*/ 			if ( nMemberId == MID_HATCH_COLOR )
/*?*/ 				aHatch.SetColor( nVal );
/*?*/ 			else if ( nMemberId == MID_HATCH_DISTANCE )
/*?*/ 				aHatch.SetDistance( nVal );
/*?*/ 			else
/*?*/ 				aHatch.SetAngle( nVal );
/*?*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 		default: DBG_ERROR("Wrong MemberId!"); return sal_False;
/*N*/ 	}
/*N*/
/*N*/ 	return true;
/*N*/ }

/*N*/ BOOL XFillHatchItem::CompareValueFunc( const NameOrIndex* p1, const NameOrIndex* p2 )
/*N*/ {
/*N*/ 	return ((XFillHatchItem*)p1)->GetValue() == ((XFillHatchItem*)p2)->GetValue();
/*N*/ }

/*N*/ XFillHatchItem* XFillHatchItem::checkForUniqueItem( SdrModel* pModel ) const
/*N*/ {
/*N*/ 	if( pModel )
/*N*/ 	{
/*N*/ 		const String aUniqueName = NameOrIndex::CheckNamedItem(	this,
/*N*/ 																XATTR_FILLHATCH,
/*N*/ 																&pModel->GetItemPool(),
/*N*/ 																pModel->GetStyleSheetPool() ? &pModel->GetStyleSheetPool()->GetPool() : NULL,
/*N*/ 																XFillHatchItem::CompareValueFunc,
/*N*/ 																RID_SVXSTR_HATCH10,
/*N*/ 																pModel->GetHatchList() );
/*N*/ 
/*N*/ 		// if the given name is not valid, replace it!
/*N*/ 		if( aUniqueName != GetName() )
/*N*/ 		{
/*N*/ 			return new XFillHatchItem( aUniqueName, aHatch );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return (XFillHatchItem*)this;
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//							FormText-Attribute
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//-------------------------
// class XFormTextStyleItem
//-------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFormTextStyleItem, SfxEnumItem);

/*************************************************************************
|*
|*	  XFormTextStyleItem::XFormTextStyleItem()
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextStyleItem::XFormTextStyleItem(XFormTextStyle eTheStyle) :
/*N*/ 	SfxEnumItem(XATTR_FORMTXTSTYLE, eTheStyle)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextStyleItem::XFormTextStyleItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextStyleItem::XFormTextStyleItem(SvStream& rIn) :
/*N*/ 	SfxEnumItem(XATTR_FORMTXTSTYLE, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextStyleItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextStyleItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFormTextStyleItem( *this );
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFormTextStyleItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextStyleItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFormTextStyleItem(rIn);
/*N*/ }

/*************************************************************************
|*
|*
|*
\*************************************************************************/

/*N*/ USHORT XFormTextStyleItem::GetValueCount() const
/*N*/ {
/*N*/ 	return 5;
/*N*/ }

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
/*N*/ bool XFormTextStyleItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (sal_Int32)GetValue();
/*N*/ 	return true;
/*N*/ }

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
/*N*/ bool XFormTextStyleItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Int32 nValue;
/*N*/ 	rVal >>= nValue;
/*N*/ 	SetValue((XFormTextStyle)nValue);
/*N*/
/*N*/ 	return true;
/*N*/ }

//-------------------------
// class XFormTextAdjustItem
//-------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFormTextAdjustItem, SfxEnumItem);

/*************************************************************************
|*
|*	  XFormTextAdjustItem::XFormTextAdjustItem()
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextAdjustItem::XFormTextAdjustItem(XFormTextAdjust eTheAdjust) :
/*N*/ 	SfxEnumItem(XATTR_FORMTXTADJUST, eTheAdjust)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextAdjustItem::XFormTextAdjustItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextAdjustItem::XFormTextAdjustItem(SvStream& rIn) :
/*N*/ 	SfxEnumItem(XATTR_FORMTXTADJUST, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextAdjustItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextAdjustItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFormTextAdjustItem( *this );
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFormTextAdjustItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextAdjustItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFormTextAdjustItem(rIn);
/*N*/ }

/*************************************************************************
|*
|*
|*
\*************************************************************************/

/*N*/ USHORT XFormTextAdjustItem::GetValueCount() const
/*N*/ {
/*N*/ 	return 4;
/*N*/ }

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
/*N*/ bool XFormTextAdjustItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (sal_Int32)GetValue();
/*N*/ 	return true;
/*N*/ }

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
/*N*/ bool XFormTextAdjustItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Int32 nValue;
/*N*/ 	rVal >>= nValue;
/*N*/ 	SetValue((XFormTextAdjust)nValue);
/*N*/
/*N*/ 	return true;
/*N*/ }

//----------------------------
// class XFormTextDistanceItem
//----------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFormTextDistanceItem, SfxMetricItem);

/*************************************************************************
|*
|*	  XFormTextDistanceItem::XFormTextDistanceItem()
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextDistanceItem::XFormTextDistanceItem(long nDist) :
/*N*/ 	SfxMetricItem(XATTR_FORMTXTDISTANCE, nDist)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextDistanceItem::XFormTextDistanceItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextDistanceItem::XFormTextDistanceItem(SvStream& rIn) :
/*N*/ 	SfxMetricItem(XATTR_FORMTXTDISTANCE, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextDistanceItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextDistanceItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFormTextDistanceItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFormTextDistanceItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextDistanceItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFormTextDistanceItem(rIn);
/*N*/ }

//-------------------------
// class XFormTextStartItem
//-------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFormTextStartItem, SfxMetricItem);

/*************************************************************************
|*
|*	  XFormTextStartItem::XFormTextStartItem(long nStart)
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextStartItem::XFormTextStartItem(long nStart) :
/*N*/ 	SfxMetricItem(XATTR_FORMTXTSTART, nStart)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextStartItem::XFormTextStartItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextStartItem::XFormTextStartItem(SvStream& rIn) :
/*N*/ 	SfxMetricItem(XATTR_FORMTXTSTART, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextStartItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextStartItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFormTextStartItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFormTextStartItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	02.02.95 ESO
|*	  Letzte Aenderung	02.02.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextStartItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFormTextStartItem(rIn);
/*N*/ }

// -------------------------
// class XFormTextMirrorItem
// -------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFormTextMirrorItem, SfxBoolItem);

/*************************************************************************
|*
|*	  XFormTextMirrorItem::XFormTextMirrorItem(BOOL bMirror)
|*
|*	  Ersterstellung	06.02.95 ESO
|*	  Letzte Aenderung	06.02.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextMirrorItem::XFormTextMirrorItem(BOOL bMirror) :
/*N*/ 	SfxBoolItem(XATTR_FORMTXTMIRROR, bMirror)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextMirrorItem::XFormTextMirrorItem(SvStream& rIn)
|*
|*	  Ersterstellung	06.02.95 ESO
|*	  Letzte Aenderung	06.02.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextMirrorItem::XFormTextMirrorItem(SvStream& rIn) :
/*N*/ 	SfxBoolItem(XATTR_FORMTXTMIRROR, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextMirrorItem::Clone(SfxItemPool* pPool) const
|*
|*	  Ersterstellung	06.02.95 ESO
|*	  Letzte Aenderung	06.02.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextMirrorItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFormTextMirrorItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFormTextMirrorItem::Create(SvStream& rIn, USHORT nVer)
|*												const
|*
|*	  Ersterstellung	06.02.95 ESO
|*	  Letzte Aenderung	06.02.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextMirrorItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFormTextMirrorItem(rIn);
/*N*/ }


// --------------------------
// class XFormTextOutlineItem
// --------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFormTextOutlineItem, SfxBoolItem);

/*************************************************************************
|*
|*	  XFormTextOutlineItem::XFormTextOutlineItem()
|*
|*	  Ersterstellung	27.06.95 ESO
|*	  Letzte Aenderung	27.06.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextOutlineItem::XFormTextOutlineItem(BOOL bOutline) :
/*N*/ 	SfxBoolItem(XATTR_FORMTXTOUTLINE, bOutline)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextOutlineItem::XFormTextOutlineItem(SvStream& rIn)
|*
|*	  Ersterstellung	27.06.95 ESO
|*	  Letzte Aenderung	27.06.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextOutlineItem::XFormTextOutlineItem(SvStream& rIn) :
/*N*/ 	SfxBoolItem(XATTR_FORMTXTOUTLINE, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextOutlineItem::Clone(SfxItemPool* pPool) const
|*
|*	  Ersterstellung	27.06.95 ESO
|*	  Letzte Aenderung	27.06.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextOutlineItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFormTextOutlineItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFormTextOutlineItem::Create(SvStream& rIn, USHORT nVer)
|*												const
|*
|*	  Ersterstellung	27.06.95 ESO
|*	  Letzte Aenderung	27.06.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextOutlineItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFormTextOutlineItem(rIn);
/*N*/ }

//--------------------------
// class XFormTextShadowItem
//--------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFormTextShadowItem, SfxEnumItem);

/*************************************************************************
|*
|*	  XFormTextShadowItem::XFormTextShadowItem()
|*
|*	  Beschreibung
|*	  Ersterstellung	27.06.95
|*	  Letzte Aenderung	27.06.95
|*
*************************************************************************/

/*N*/ XFormTextShadowItem::XFormTextShadowItem(XFormTextShadow eFormTextShadow) :
/*N*/ 	SfxEnumItem(XATTR_FORMTXTSHADOW, eFormTextShadow)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextShadowItem::XFormTextShadowItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	27.06.95
|*	  Letzte Aenderung	27.06.95
|*
*************************************************************************/

/*N*/ XFormTextShadowItem::XFormTextShadowItem(SvStream& rIn) :
/*N*/ 	SfxEnumItem(XATTR_FORMTXTSHADOW, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextShadowItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	27.06.95
|*	  Letzte Aenderung	27.06.95
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextShadowItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFormTextShadowItem( *this );
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFormTextShadowItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	27.06.95
|*	  Letzte Aenderung	27.06.95
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextShadowItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFormTextShadowItem(rIn);
/*N*/ }


/*************************************************************************
|*
|*
|*
\*************************************************************************/

/*N*/ USHORT XFormTextShadowItem::GetValueCount() const
/*N*/ {
/*N*/ 	return 3;
/*N*/ }

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
/*N*/ bool XFormTextShadowItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (sal_Int32)GetValue();
/*N*/ 	return true;
/*N*/ }

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
/*N*/ bool XFormTextShadowItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Int32 nValue;
/*N*/ 	rVal >>= nValue;
/*N*/ 	SetValue((XFormTextShadow)nValue);
/*N*/
/*N*/ 	return true;
/*N*/ }

// -------------------------------
// class XFormTextShadowColorItem
// -------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFormTextShadowColorItem, XColorItem);

/*************************************************************************
|*
|*	  XFormTextShadowColorItem::XFormTextShadowColorItem(const XubString& rName, const Color& rTheColor)
|*
|*	  Beschreibung
|*	  Ersterstellung	27.06.95
|*	  Letzte Aenderung	27.06.95
|*
*************************************************************************/

/*N*/ XFormTextShadowColorItem::XFormTextShadowColorItem(const XubString& rName,
/*N*/ 													 const Color& rTheColor) :
/*N*/ 	XColorItem(XATTR_FORMTXTSHDWCOLOR, rName, rTheColor)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextShadowColorItem::XFormTextShadowColorItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	27.06.95
|*	  Letzte Aenderung	27.06.95
|*
*************************************************************************/

/*N*/ XFormTextShadowColorItem::XFormTextShadowColorItem(SvStream& rIn) :
/*N*/ 	XColorItem(XATTR_FORMTXTSHDWCOLOR, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextShadowColorItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	27.06.95
|*	  Letzte Aenderung	27.06.95
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextShadowColorItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFormTextShadowColorItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFormTextShadowColorItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	27.06.95
|*	  Letzte Aenderung	27.06.95
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextShadowColorItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFormTextShadowColorItem(rIn);
/*N*/ }

//------------------------------
// class XFormTextShadowXValItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFormTextShadowXValItem, SfxMetricItem);

/*************************************************************************
|*
|*	  XFormTextShadowXValItem::XFormTextShadowXValItem(long)
|*
|*	  Beschreibung
|*	  Ersterstellung	28.06.95 ESO
|*	  Letzte Aenderung	28.06.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextShadowXValItem::XFormTextShadowXValItem(long nVal) :
/*N*/ 	SfxMetricItem(XATTR_FORMTXTSHDWXVAL, nVal)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextShadowXValItem::XFormTextShadowXValItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	28.06.95 ESO
|*	  Letzte Aenderung	28.06.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextShadowXValItem::XFormTextShadowXValItem(SvStream& rIn) :
/*N*/ 	SfxMetricItem(XATTR_FORMTXTSHDWXVAL, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextShadowXValItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	28.06.95 ESO
|*	  Letzte Aenderung	28.06.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextShadowXValItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFormTextShadowXValItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFormTextShadowXValItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	28.06.95 ESO
|*	  Letzte Aenderung	28.06.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextShadowXValItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFormTextShadowXValItem(rIn);
/*N*/ }

//------------------------------
// class XFormTextShadowYValItem
//------------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFormTextShadowYValItem, SfxMetricItem);

/*************************************************************************
|*
|*	  XFormTextShadowYValItem::XFormTextShadowYValItem(long)
|*
|*	  Beschreibung
|*	  Ersterstellung	28.06.95 ESO
|*	  Letzte Aenderung	28.06.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextShadowYValItem::XFormTextShadowYValItem(long nVal) :
/*N*/ 	SfxMetricItem(XATTR_FORMTXTSHDWYVAL, nVal)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextShadowYValItem::XFormTextShadowYValItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	28.06.95 ESO
|*	  Letzte Aenderung	28.06.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextShadowYValItem::XFormTextShadowYValItem(SvStream& rIn) :
/*N*/ 	SfxMetricItem(XATTR_FORMTXTSHDWYVAL, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextShadowYValItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	28.06.95 ESO
|*	  Letzte Aenderung	28.06.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextShadowYValItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFormTextShadowYValItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFormTextShadowYValItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	28.06.95 ESO
|*	  Letzte Aenderung	28.06.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextShadowYValItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFormTextShadowYValItem(rIn);
/*N*/ }

//---------------------------
// class XFormTextStdFormItem
//---------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFormTextStdFormItem, SfxEnumItem);

/*************************************************************************
|*
|*	  XFormTextStdFormItem::XFormTextStdFormItem()
|*
|*	  Beschreibung
|*	  Ersterstellung	27.06.95
|*	  Letzte Aenderung	27.06.95
|*
*************************************************************************/

/*N*/ XFormTextStdFormItem::XFormTextStdFormItem(XFormTextStdForm eFormTextStdForm) :
/*N*/ 	SfxEnumItem(XATTR_FORMTXTSTDFORM, eFormTextStdForm)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextStdFormItem::XFormTextStdFormItem(SvStream& rIn)
|*
|*	  Beschreibung
|*	  Ersterstellung	27.06.95
|*	  Letzte Aenderung	27.06.95
|*
*************************************************************************/

/*N*/ XFormTextStdFormItem::XFormTextStdFormItem(SvStream& rIn) :
/*N*/ 	SfxEnumItem(XATTR_FORMTXTSTDFORM, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextStdFormItem::Clone(SfxItemPool* pPool) const
|*
|*	  Beschreibung
|*	  Ersterstellung	27.06.95
|*	  Letzte Aenderung	27.06.95
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextStdFormItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFormTextStdFormItem( *this );
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFormTextStdFormItem::Create(SvStream& rIn, USHORT nVer) const
|*
|*	  Beschreibung
|*	  Ersterstellung	27.06.95
|*	  Letzte Aenderung	27.06.95
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextStdFormItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFormTextStdFormItem(rIn);
/*N*/ }


/*************************************************************************
|*
|*
|*
\*************************************************************************/

/*N*/ USHORT XFormTextStdFormItem::GetValueCount() const
/*N*/ {
/*N*/ 	return 3;
/*N*/ }

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
/*N*/ bool XFormTextStdFormItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (sal_Int32)GetValue();
/*N*/ 	return true;
/*N*/ }

/*************************************************************************
|*
|*
|*
\*************************************************************************/

// #FontWork#
/*N*/ bool XFormTextStdFormItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	sal_Int32 nValue;
/*N*/ 	rVal >>= nValue;
/*N*/ 	SetValue((XFormTextStdForm)nValue);
/*N*/
/*N*/ 	return true;
/*N*/ }

// --------------------------
// class XFormTextHideFormItem
// --------------------------
/*N*/ TYPEINIT1_AUTOFACTORY(XFormTextHideFormItem, SfxBoolItem);

/*************************************************************************
|*
|*	  XFormTextHideFormItem::XFormTextHideFormItem()
|*
|*	  Ersterstellung	27.06.95 ESO
|*	  Letzte Aenderung	27.06.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextHideFormItem::XFormTextHideFormItem(BOOL bHide) :
/*N*/ 	SfxBoolItem(XATTR_FORMTXTHIDEFORM, bHide)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextHideFormItem::XFormTextHideFormItem(SvStream& rIn)
|*
|*	  Ersterstellung	27.06.95 ESO
|*	  Letzte Aenderung	27.06.95 ESO
|*
*************************************************************************/

/*N*/ XFormTextHideFormItem::XFormTextHideFormItem(SvStream& rIn) :
/*N*/ 	SfxBoolItem(XATTR_FORMTXTHIDEFORM, rIn)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	  XFormTextHideFormItem::Clone(SfxItemPool* pPool) const
|*
|*	  Ersterstellung	27.06.95 ESO
|*	  Letzte Aenderung	27.06.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextHideFormItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new XFormTextHideFormItem(*this);
/*N*/ }

/*************************************************************************
|*
|*	  SfxPoolItem* XFormTextHideFormItem::Create(SvStream& rIn, USHORT nVer)
|*												const
|*
|*	  Ersterstellung	27.06.95 ESO
|*	  Letzte Aenderung	27.06.95 ESO
|*
*************************************************************************/

/*N*/ SfxPoolItem* XFormTextHideFormItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new XFormTextHideFormItem(rIn);
/*N*/ }



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//								SetItems
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1(XLineAttrSetItem, SfxSetItem);

/*************************************************************************
|*
|* Konstruktoren fuer Linienattribute-SetItem
|*
\************************************************************************/

/*N*/ XLineAttrSetItem::XLineAttrSetItem( SfxItemSet* pItemSet ) :
/*N*/ 	SfxSetItem( XATTRSET_LINE, pItemSet)
/*N*/ {
/*N*/ }

/************************************************************************/

/*N*/ XLineAttrSetItem::XLineAttrSetItem( SfxItemPool* pItemPool ) :
/*N*/ 	SfxSetItem( XATTRSET_LINE,
/*N*/ 		new SfxItemSet( *pItemPool, XATTR_LINE_FIRST, XATTR_LINE_LAST))
/*N*/ {
/*N*/ }

/************************************************************************/

/*?*/ XLineAttrSetItem::XLineAttrSetItem( const XLineAttrSetItem& rLineAttr ) :
/*?*/ 	SfxSetItem( rLineAttr )
/*?*/ {
/*?*/ }

/************************************************************************/

/*N*/ XLineAttrSetItem::XLineAttrSetItem( const XLineAttrSetItem& rLineAttr,
/*N*/ 									SfxItemPool* pItemPool) :
/*N*/ 	SfxSetItem( rLineAttr, pItemPool )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Clone-Funktion
|*
\************************************************************************/

/*N*/ SfxPoolItem* XLineAttrSetItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XLineAttrSetItem( *this, pPool );
/*N*/ }

/*************************************************************************
|*
|* SetItem aus Stream erzeugen
|*
\************************************************************************/

/*N*/ SfxPoolItem* XLineAttrSetItem::Create( SvStream& rStream, USHORT nVersion ) const
/*N*/ {
/*N*/ 	SfxItemSet *pSet = new SfxItemSet( *GetItemSet().GetPool(),
/*N*/ 									XATTR_LINE_FIRST, XATTR_LINE_LAST);
/*N*/ 	pSet->Load( rStream );
/*N*/ 	return new XLineAttrSetItem( pSet );
/*N*/ }

/*************************************************************************
|*
|* SetItem in Stream speichern
|*
\************************************************************************/

/*N*/ SvStream& XLineAttrSetItem::Store( SvStream& rStream, USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	return SfxSetItem::Store( rStream, nItemVersion );
/*N*/ }


/*N*/ TYPEINIT1(XFillAttrSetItem, SfxSetItem);

/*************************************************************************
|*
|* Konstruktoren fuer Fuellattribute-SetItem
|*
\************************************************************************/

/*N*/ XFillAttrSetItem::XFillAttrSetItem( SfxItemSet* pItemSet ) :
/*N*/ 	SfxSetItem( XATTRSET_FILL, pItemSet)
/*N*/ {
/*N*/ }

/************************************************************************/

/*N*/ XFillAttrSetItem::XFillAttrSetItem( SfxItemPool* pItemPool ) :
/*N*/ 	SfxSetItem( XATTRSET_FILL,
/*N*/ 		new SfxItemSet( *pItemPool, XATTR_FILL_FIRST, XATTR_FILL_LAST))
/*N*/ {
/*N*/ }

/************************************************************************/

/*?*/ XFillAttrSetItem::XFillAttrSetItem( const XFillAttrSetItem& rFillAttr ) :
/*?*/ 	SfxSetItem( rFillAttr )
/*?*/ {
/*?*/ }

/************************************************************************/

/*N*/ XFillAttrSetItem::XFillAttrSetItem( const XFillAttrSetItem& rFillAttr,
/*N*/ 									SfxItemPool* pItemPool ) :
/*N*/ 	SfxSetItem( rFillAttr, pItemPool )
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Clone-Funktion
|*
\************************************************************************/

/*N*/ SfxPoolItem* XFillAttrSetItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XFillAttrSetItem( *this, pPool );
/*N*/ }

/*************************************************************************
|*
|* SetItem aus Stream erzeugen
|*
\************************************************************************/

/*N*/ SfxPoolItem* XFillAttrSetItem::Create( SvStream& rStream, USHORT nVersion ) const
/*N*/ {
/*N*/ 	SfxItemSet *pSet = new SfxItemSet( *GetItemSet().GetPool(),
/*N*/ 									XATTR_FILL_FIRST, XATTR_FILL_LAST);
/*N*/ 	pSet->Load( rStream );
/*N*/ 	return new XFillAttrSetItem( pSet );
/*N*/ }

/*************************************************************************
|*
|* SetItem in Stream speichern
|*
\************************************************************************/

/*N*/ SvStream& XFillAttrSetItem::Store( SvStream& rStream, USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	return SfxSetItem::Store( rStream, nItemVersion );
/*N*/ }


/*N*/ TYPEINIT1(XTextAttrSetItem, SfxSetItem);

/*************************************************************************
|*
|* Konstruktoren fuer Textattribute-SetItem
|*
\************************************************************************/

/*N*/ XTextAttrSetItem::XTextAttrSetItem( SfxItemSet* pItemSet ) :
/*N*/ 	SfxSetItem( XATTRSET_TEXT, pItemSet)
/*N*/ {
/*N*/ }

/************************************************************************/

/*N*/ XTextAttrSetItem::XTextAttrSetItem( SfxItemPool* pItemPool ) :
/*N*/ 	SfxSetItem( XATTRSET_TEXT,
/*N*/ 		new SfxItemSet( *pItemPool, XATTR_TEXT_FIRST, XATTR_TEXT_LAST))
/*N*/ {
/*N*/ }

/************************************************************************/

/*?*/ XTextAttrSetItem::XTextAttrSetItem( const XTextAttrSetItem& rTextAttr ) :
/*?*/ 	SfxSetItem( rTextAttr )
/*?*/ {
/*?*/ }

/************************************************************************/

/*N*/ XTextAttrSetItem::XTextAttrSetItem( const XTextAttrSetItem& rTextAttr,
/*N*/ 									SfxItemPool* pItemPool ) :
/*N*/ 	SfxSetItem( rTextAttr, pItemPool )
/*N*/ {
/*N*/ }
/*N*/ 
/*************************************************************************
|*
|* Clone-Funktion
|*
\************************************************************************/

/*N*/ SfxPoolItem* XTextAttrSetItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new XTextAttrSetItem( *this, pPool );
/*N*/ }

/*************************************************************************
|*
|* SetItem aus Stream erzeugen
|*
\************************************************************************/

/*N*/ SfxPoolItem* XTextAttrSetItem::Create( SvStream& rStream, USHORT nVersion ) const
/*N*/ {
/*N*/ 	SfxItemSet *pSet = new SfxItemSet( *GetItemSet().GetPool(),
/*N*/ 									XATTR_TEXT_FIRST, XATTR_TEXT_LAST);
/*N*/ 	pSet->Load( rStream );
/*N*/ 	return new XTextAttrSetItem( pSet );
/*N*/ }

/*************************************************************************
|*
|* SetItem in Stream speichern
|*
\************************************************************************/

/*N*/ SvStream& XTextAttrSetItem::Store( SvStream& rStream, USHORT nItemVersion ) const
/*N*/ {
/*N*/ 	return SfxSetItem::Store( rStream, nItemVersion );
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
