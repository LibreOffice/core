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

#include "objsh.hxx"
#include "objitem.hxx"
namespace binfilter {

//====================================================================

/*N*/ TYPEINIT1(SfxObjectShellItem,SfxPoolItem)
/*N*/ TYPEINIT1_AUTOFACTORY(SfxObjectItem,SfxPoolItem)

//=========================================================================

/*N*/ int SfxObjectShellItem::operator==( const SfxPoolItem &rItem ) const
/*N*/ {
/*N*/ 	 return PTR_CAST(SfxObjectShellItem, &rItem)->pObjSh == pObjSh;
/*N*/ }

//--------------------------------------------------------------------

/*N*/ String SfxObjectShellItem::GetValueText() const
/*N*/ {
/*N*/ 	return String();
/*N*/ }

//--------------------------------------------------------------------

/*N*/ SfxPoolItem* SfxObjectShellItem::Clone( SfxItemPool *) const
/*N*/ {
/*N*/ 	return new SfxObjectShellItem( Which(), pObjSh );
/*N*/ }

//=========================================================================

/*N*/ SfxObjectItem::SfxObjectItem( USHORT nWhich, SfxShell *pSh )
/*N*/ :	SfxPoolItem( nWhich ),
/*N*/     _pSh( pSh )
/*N*/ {}

//--------------------------------------------------------------------

/*N*/ int SfxObjectItem::operator==( const SfxPoolItem &rItem ) const
/*N*/ {
/*N*/ 	 SfxObjectItem *pOther = PTR_CAST(SfxObjectItem, &rItem);
/*N*/      return pOther->_pSh == _pSh;
/*N*/ }

//--------------------------------------------------------------------

/*N*/ SfxPoolItem* SfxObjectItem::Clone( SfxItemPool *) const
/*N*/ {
/*N*/     return new SfxObjectItem( Which(), _pSh );
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
