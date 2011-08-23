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

//------------------------------------------------------------------------

#include <bf_sfx2/objsh.hxx>

#include "adiasync.hxx"
#include "document.hxx"
#include "bf_sc.hrc"		// FID_DATACHANGED

#include <osl/thread.h>
namespace binfilter {


//------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma code_seg("SCSTATICS")
#endif

ScAddInAsyncs theAddInAsyncTbl;
static ScAddInAsync aSeekObj;

#ifdef _MSC_VER
#pragma code_seg()
#endif


/*N*/ SV_IMPL_OP_PTRARR_SORT( ScAddInAsyncs, ScAddInAsyncPtr );

/*N*/ SV_IMPL_PTRARR_SORT( ScAddInDocs, ScAddInDocPtr );




/*N*/ ScAddInAsync::ScAddInAsync() :
/*N*/ 	SfxBroadcaster(),
/*N*/ 	nHandle( 0 )
/*N*/ {	// nur fuer aSeekObj !
/*N*/ }



/*N*/ ScAddInAsync::ScAddInAsync( ULONG nHandleP, USHORT nIndex, ScDocument* pDoc ) :
/*N*/ 	SfxBroadcaster(),
/*N*/ 	pStr( NULL ),
/*N*/ 	nHandle( nHandleP ),
/*N*/ 	bValid( FALSE )
/*N*/ {
/*N*/ 	pDocs = new ScAddInDocs( 1, 1 );
/*N*/ 	pDocs->Insert( pDoc );
/*N*/ 	pFuncData = (FuncData*)ScGlobal::GetFuncCollection()->At(nIndex);
/*N*/ 	eType = pFuncData->GetAsyncType();
/*N*/ 	theAddInAsyncTbl.Insert( this );
/*N*/ }



/*N*/ ScAddInAsync::~ScAddInAsync()
/*N*/ {
/*N*/ 	// aSeekObj hat das alles nicht, Handle 0 gibt es sonst nicht
/*N*/ 	if ( nHandle )
/*N*/ 	{
/*?*/ 		// im dTor wg. theAddInAsyncTbl.DeleteAndDestroy in ScGlobal::Clear
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pFuncData->Unadvice( (double)nHandle );
/*N*/ 	}
/*N*/ }



/*N*/ ScAddInAsync* ScAddInAsync::Get( ULONG nHandleP )
/*N*/ {
/*N*/ 	USHORT nPos;
/*N*/ 	ScAddInAsync* pRet = 0;
/*N*/ 	aSeekObj.nHandle = nHandleP;
/*N*/ 	if ( theAddInAsyncTbl.Seek_Entry( &aSeekObj, &nPos ) )
/*N*/ 		pRet = theAddInAsyncTbl[ nPos ];
/*N*/ 	aSeekObj.nHandle = 0;
/*N*/ 	return pRet;
/*N*/ }






/*N*/ void ScAddInAsync::RemoveDocument( ScDocument* pDocumentP )
/*N*/ {
/*N*/ 	USHORT nPos = theAddInAsyncTbl.Count();
/*N*/ 	if ( nPos )
/*N*/ 	{
/*?*/ 		const ScAddInAsync** ppAsync =
/*?*/ 			(const ScAddInAsync**) theAddInAsyncTbl.GetData() + nPos - 1;
/*?*/ 		for ( ; nPos-- >0; ppAsync-- )
/*?*/ 		{	// rueckwaerts wg. Pointer-Aufrueckerei im Array
/*?*/ 			ScAddInDocs* p = ((ScAddInAsync*)*ppAsync)->pDocs;
/*?*/ 			USHORT nFoundPos;
/*?*/ 			if ( p->Seek_Entry( pDocumentP, &nFoundPos ) )
/*?*/ 			{
/*?*/ 				p->Remove( nFoundPos );
/*?*/ 				if ( p->Count() == 0 )
/*?*/ 				{	// dieses AddIn wird nicht mehr benutzt
/*?*/ 					ScAddInAsync* pAsync = (ScAddInAsync*)*ppAsync;
/*?*/ 					theAddInAsyncTbl.Remove( nPos );
/*?*/ 					delete pAsync;
/*?*/ 					ppAsync = (const ScAddInAsync**) theAddInAsyncTbl.GetData()
/*?*/ 						+ nPos;
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
