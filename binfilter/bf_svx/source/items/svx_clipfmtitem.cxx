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

#define _SVSTDARR_ULONGS
#define _SVSTDARR_STRINGSDTOR

#include <bf_svtools/svstdarr.hxx>
#include <clipfmtitem.hxx>
namespace binfilter {



struct SvxClipboardFmtItem_Impl
{
    SvStringsDtor aFmtNms;
    SvULongs aFmtIds;
    static String sEmptyStr;

    SvxClipboardFmtItem_Impl() : aFmtNms( 8, 8 ), aFmtIds( 8, 8 ) {}
    SvxClipboardFmtItem_Impl( const SvxClipboardFmtItem_Impl& );
};

String SvxClipboardFmtItem_Impl::sEmptyStr;

/*N*/ TYPEINIT1( SvxClipboardFmtItem, SfxPoolItem );

/*N*/ SvxClipboardFmtItem_Impl::SvxClipboardFmtItem_Impl(
/*N*/ 							const SvxClipboardFmtItem_Impl& rCpy )
/*N*/ {
/*N*/ 	aFmtIds.Insert( &rCpy.aFmtIds, 0 );
/*N*/ 	for( USHORT n = 0, nEnd = rCpy.aFmtNms.Count(); n < nEnd; ++n )
/*N*/ 	{
/*N*/ 		String* pStr = rCpy.aFmtNms[ n ];
/*N*/ 		if( pStr )
/*N*/ 			pStr = new String( *pStr );
/*N*/ 		aFmtNms.Insert( pStr, n );
/*N*/ 	}
/*N*/ }

/*N*/ SvxClipboardFmtItem::SvxClipboardFmtItem( const SvxClipboardFmtItem& rCpy )
/*N*/ 	: SfxPoolItem( rCpy.Which() ),
/*N*/ 	pImpl( new SvxClipboardFmtItem_Impl( *rCpy.pImpl ) )
/*N*/ {
/*N*/ }

/*N*/ SvxClipboardFmtItem::~SvxClipboardFmtItem()
/*N*/ {
/*N*/ 	delete pImpl;
/*N*/ }

/*N*/ int SvxClipboardFmtItem::operator==( const SfxPoolItem& rComp ) const
/*N*/ {
/*N*/ 	int nRet = 0;
/*N*/ 	const SvxClipboardFmtItem& rCmp = (SvxClipboardFmtItem&)rComp;
/*N*/ 	if( rCmp.pImpl->aFmtNms.Count() == pImpl->aFmtNms.Count() )
/*N*/ 	{
/*N*/ 		nRet = 1;
/*N*/ 		const String* pStr1, *pStr2;
/*N*/ 		for( USHORT n = 0, nEnd = rCmp.pImpl->aFmtNms.Count(); n < nEnd; ++n )
/*N*/ 		{
/*N*/ 			if( pImpl->aFmtIds[ n ] != rCmp.pImpl->aFmtIds[ n ] ||
/*N*/ 				( (0 == ( pStr1 = pImpl->aFmtNms[ n ] )) ^
/*N*/ 				  (0 == ( pStr2 = rCmp.pImpl->aFmtNms[ n ] ) )) ||
/*N*/ 				( pStr1 && *pStr1 != *pStr2 ))
/*N*/ 			{
/*N*/ 				nRet = 0;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/*N*/ SfxPoolItem* SvxClipboardFmtItem::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	return new SvxClipboardFmtItem( *this );
/*N*/ }
}
