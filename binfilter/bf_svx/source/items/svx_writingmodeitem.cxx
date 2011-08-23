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


#include <bf_svx/xdef.hxx>

#include "writingmodeitem.hxx"


#include "svxitems.hrc"
namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;

// class SvxWritingModeItem -------------------------------------------------

/*N*/ TYPEINIT1_AUTOFACTORY(SvxWritingModeItem, SfxUInt16Item);

/*N*/ SvxWritingModeItem::SvxWritingModeItem( WritingMode eValue, USHORT nWhich )
/*N*/ 	: SfxUInt16Item( nWhich, (sal_uInt16)eValue )
/*N*/ {
/*N*/ }

/*N*/ SvxWritingModeItem::~SvxWritingModeItem()
/*N*/ {
/*N*/ }

/*N*/ int SvxWritingModeItem::operator==( const SfxPoolItem& rCmp ) const
/*N*/ {
/*N*/ 	DBG_ASSERT( SfxPoolItem::operator==(rCmp), "unequal types" );
/*N*/ 
/*N*/ 	return GetValue() == ((SvxWritingModeItem&)rCmp).GetValue();
/*N*/ }

/*N*/ SfxPoolItem* SvxWritingModeItem::Clone( SfxItemPool * ) const
/*N*/ {
/*N*/ 	return new SvxWritingModeItem( *this );
/*N*/ }



/*N*/ USHORT SvxWritingModeItem::GetVersion( USHORT nFVer ) const
/*N*/ {
/*N*/ 	return USHRT_MAX;
/*N*/ }



/*N*/ bool SvxWritingModeItem::QueryValue( ::com::sun::star::uno::Any& rVal,
/*N*/ 											BYTE ) const
/*N*/ {
/*N*/ 	rVal <<= (WritingMode)GetValue();
/*N*/ 	return true;
/*N*/ }

/*N*/ SvxWritingModeItem& SvxWritingModeItem::operator=( const SvxWritingModeItem& rItem )
/*N*/ {
/*N*/ 	SetValue( rItem.GetValue() );
/*N*/ 	return *this;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
