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

#include <txtrfmrk.hxx>
#include <fmtrfmrk.hxx>
namespace binfilter {


/****************************************************************************
 *
 *  class SwFmtRefMark
 *
 ****************************************************************************/

/*N*/ SwFmtRefMark::~SwFmtRefMark( )
/*N*/ {
/*N*/ }

/*N*/ SwFmtRefMark::SwFmtRefMark( const XubString& rName )
/*N*/ 	: SfxPoolItem( RES_TXTATR_REFMARK ),
/*N*/ 	aRefName( rName ),
/*N*/ 	pTxtAttr( 0 )
/*N*/ {
/*N*/ }

/*N*/ SwFmtRefMark::SwFmtRefMark( const SwFmtRefMark& rAttr )
/*N*/ 	: SfxPoolItem( RES_TXTATR_REFMARK ),
/*N*/ 	aRefName( rAttr.aRefName ),
/*N*/ 	pTxtAttr( 0 )
/*N*/ {
/*N*/ }

int SwFmtRefMark::operator==( const SfxPoolItem& rAttr ) const
{
    {DBG_BF_ASSERT(0, "STRIP");} return 0;//STRIP001 	ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
}

/*N*/ SfxPoolItem* SwFmtRefMark::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SwFmtRefMark( *this );
/*N*/ }

/*************************************************************************
 *						class SwTxtRefMark
 *************************************************************************/

// Attribut fuer Inhalts-/Positions-Referenzen im Text

/*N*/ SwTxtRefMark::SwTxtRefMark( const SwFmtRefMark& rAttr,
/*N*/ 					xub_StrLen nStart, xub_StrLen* pEnde )
/*N*/ 	: SwTxtAttrEnd( rAttr, nStart, nStart ),
/*N*/ 	pEnd( 0 ),
/*N*/ 	pMyTxtNd( 0 )
/*N*/ {
/*N*/ 	((SwFmtRefMark&)rAttr).pTxtAttr = this;
/*N*/ 	if( pEnde )
/*N*/ 	{
/*N*/ 		nEnd = *pEnde;
/*N*/ 		pEnd = &nEnd;
/*N*/ 	}
/*N*/ 	SetDontMergeAttr( TRUE );
/*N*/ 	SetDontMoveAttr( TRUE );
/*N*/ 	SetOverlapAllowedAttr( TRUE );
/*N*/ }

/*N*/ xub_StrLen* SwTxtRefMark::GetEnd()
/*N*/ {
/*N*/ 	return pEnd;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
