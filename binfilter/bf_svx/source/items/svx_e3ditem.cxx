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

#include <com/sun/star/drawing/Direction3D.hpp>
#include <tools/stream.hxx>
#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "e3ditem.hxx"
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

// STATIC DATA -----------------------------------------------------------

/*N*/ DBG_NAME(SvxVector3DItem)

// -----------------------------------------------------------------------

/*N*/ TYPEINIT1_AUTOFACTORY(SvxVector3DItem, SfxPoolItem);

// -----------------------------------------------------------------------

/*?*/ SvxVector3DItem::SvxVector3DItem()
/*?*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 	DBG_CTOR(SvxVector3DItem, 0);
/*?*/ }

// -----------------------------------------------------------------------

/*N*/ SvxVector3DItem::SvxVector3DItem( USHORT nWhich, const Vector3D& rVal ) :
/*N*/ 	SfxPoolItem( nWhich ),
/*N*/ 	aVal( rVal )
/*N*/ {
/*N*/ 	DBG_CTOR(SvxVector3DItem, 0);
/*N*/ }

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

/*N*/ SvxVector3DItem::SvxVector3DItem( const SvxVector3DItem& rItem ) :
/*N*/ 	SfxPoolItem( rItem ),
/*N*/ 	aVal( rItem.aVal )
/*N*/ {
/*N*/ 	DBG_CTOR(SvxVector3DItem, 0);
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ int SvxVector3DItem::operator==( const SfxPoolItem &rItem ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(SvxVector3DItem, 0);
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
/*N*/ 	return ((SvxVector3DItem&)rItem).aVal == aVal;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxVector3DItem::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(SvxVector3DItem, 0);
/*N*/ 	return new SvxVector3DItem( *this );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SfxPoolItem* SvxVector3DItem::Create(SvStream &rStream, USHORT nVersion) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(SvxVector3DItem, 0);
/*N*/ 	Vector3D aStr;
/*N*/ 	rStream >> aStr;
/*N*/ 	return new SvxVector3DItem(Which(), aStr);
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ SvStream& SvxVector3DItem::Store(SvStream &rStream, USHORT nItemVersion) const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(SvxVector3DItem, 0);
/*N*/ 
/*N*/ 	// ## if (nItemVersion) 
/*N*/ 	rStream << aVal;
/*N*/ 
/*N*/ 	return rStream;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ bool SvxVector3DItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	drawing::Direction3D aDirection;
/*N*/ 
/*N*/ 	// Werte eintragen
/*N*/ 	aDirection.DirectionX = aVal.X();
/*N*/ 	aDirection.DirectionY = aVal.Y();
/*N*/ 	aDirection.DirectionZ = aVal.Z();
/*N*/ 
/*N*/ 	rVal <<= aDirection;
/*N*/ 	return( sal_True );
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ bool SvxVector3DItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	drawing::Direction3D aDirection;
/*N*/ 	if(!(rVal >>= aDirection))
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	aVal.X() = aDirection.DirectionX;
/*N*/ 	aVal.Y() = aDirection.DirectionY;
/*N*/ 	aVal.Z() = aDirection.DirectionZ;
/*N*/ 	return sal_True;
/*N*/ }

// -----------------------------------------------------------------------

/*N*/ USHORT SvxVector3DItem::GetVersion (USHORT nFileFormatVersion) const
/*N*/ {
/*N*/ 	return (nFileFormatVersion == SOFFICE_FILEFORMAT_31) ? USHRT_MAX : 0;
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
