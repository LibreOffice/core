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

#include <viewsh.hxx>
#include "swfntcch.hxx"
#include "fmtcol.hxx"
namespace binfilter {

// aus atrstck.cxx
extern const BYTE StackPos[];

// globale Variablen, werden in SwFntCch.Hxx bekanntgegeben
// Der FontCache wird in TxtInit.Cxx _TXTINIT erzeugt und in _TXTEXIT geloescht
SwFontCache *pSwFontCache = NULL;

/*************************************************************************
|*
|*	SwFontObj::SwFontObj(), ~SwFontObj()
|*
|*	Ersterstellung		AMA 25. Jun. 95
|*	Letzte Aenderung	AMA 25. Jun. 95
|*
|*************************************************************************/

/*N*/ SwFontObj::SwFontObj( const void *pOwner, ViewShell *pSh ) :
/*N*/ 	SwCacheObj( (void*)pOwner ),
/*N*/ 	aSwFont( &((SwTxtFmtColl *)pOwner)->GetAttrSet(), pSh ? pSh->GetDoc() : 0 )
/*N*/ {
/*N*/ 	aSwFont.GoMagic( pSh, aSwFont.GetActual() );
/*N*/     const SwAttrSet& rAttrSet = ((SwTxtFmtColl *)pOwner)->GetAttrSet();
/*N*/     for (USHORT i = RES_CHRATR_BEGIN; i < RES_CHRATR_END; i++)
/*N*/         pDefaultArray[ StackPos[ i ] ] = &rAttrSet.Get( i, TRUE );
/*N*/ }

/*N*/ SwFontObj::~SwFontObj()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	SwFontAccess::SwFontAccess()
|*
|*	Ersterstellung		AMA 25. Jun. 95
|*	Letzte Aenderung	AMA 25. Jun. 95
|*
|*************************************************************************/

/*N*/ SwFontAccess::SwFontAccess( const void *pOwner, ViewShell *pSh ) :
/*N*/ 	SwCacheAccess( *pSwFontCache, pOwner,
/*N*/ 			(BOOL) ((SwTxtFmtColl*)pOwner)->IsInSwFntCache() ),
/*N*/ 	pShell( pSh )
/*N*/ {
/*N*/ }

/*N*/ SwFontObj *SwFontAccess::Get( )
/*N*/ {
/*N*/ 	return (SwFontObj *) SwCacheAccess::Get( );
/*N*/ }

/*N*/ SwCacheObj *SwFontAccess::NewObj( )
/*N*/ {
/*N*/ 	((SwTxtFmtColl*)pOwner)->SetInSwFntCache( TRUE );
/*N*/ 	return new SwFontObj( pOwner, pShell );
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
