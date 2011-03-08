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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------------

#include <sfx2/objsh.hxx>

#include "adiasync.hxx"
#include "brdcst.hxx"
#include "global.hxx"
#include "document.hxx"
#include "sc.hrc"       // FID_DATACHANGED
#include <osl/thread.h>


//------------------------------------------------------------------------

ScAddInAsyncs theAddInAsyncTbl;
static ScAddInAsync aSeekObj;


SV_IMPL_OP_PTRARR_SORT( ScAddInAsyncs, ScAddInAsyncPtr );

SV_IMPL_PTRARR_SORT( ScAddInDocs, ScAddInDocPtr );

extern "C" {
void CALLTYPE ScAddInAsyncCallBack( double& nHandle, void* pData )
{
    ScAddInAsync::CallBack( ULONG( nHandle ), pData );
}
}



ScAddInAsync::ScAddInAsync() :
    SvtBroadcaster(),
    nHandle( 0 )
{   // nur fuer aSeekObj !
}



ScAddInAsync::ScAddInAsync( ULONG nHandleP, USHORT nIndex, ScDocument* pDoc ) :
    SvtBroadcaster(),
    pStr( NULL ),
    nHandle( nHandleP ),
    bValid( FALSE )
{
    pDocs = new ScAddInDocs( 1, 1 );
    pDocs->Insert( pDoc );
    pFuncData = (FuncData*)ScGlobal::GetFuncCollection()->At(nIndex);
    eType = pFuncData->GetAsyncType();
    theAddInAsyncTbl.Insert( this );
}



ScAddInAsync::~ScAddInAsync()
{
    // aSeekObj hat das alles nicht, Handle 0 gibt es sonst nicht
    if ( nHandle )
    {
        // im dTor wg. theAddInAsyncTbl.DeleteAndDestroy in ScGlobal::Clear
        pFuncData->Unadvice( (double)nHandle );
        if ( eType == PTR_STRING && pStr )      // mit Typvergleich wg. Union!
            delete pStr;
        delete pDocs;
    }
}



ScAddInAsync* ScAddInAsync::Get( ULONG nHandleP )
{
    USHORT nPos;
    ScAddInAsync* pRet = 0;
    aSeekObj.nHandle = nHandleP;
    if ( theAddInAsyncTbl.Seek_Entry( &aSeekObj, &nPos ) )
        pRet = theAddInAsyncTbl[ nPos ];
    aSeekObj.nHandle = 0;
    return pRet;
}



void ScAddInAsync::CallBack( ULONG nHandleP, void* pData )
{
    ScAddInAsync* p;
    if ( (p = Get( nHandleP )) == NULL )
        return;
    // keiner mehr dran? Unadvice und weg damit
    if ( !p->HasListeners() )
    {
        // nicht im dTor wg. theAddInAsyncTbl.DeleteAndDestroy in ScGlobal::Clear
        theAddInAsyncTbl.Remove( p );
        delete p;
        return ;
    }
    switch ( p->eType )
    {
        case PTR_DOUBLE :
            p->nVal = *(double*)pData;
            break;
        case PTR_STRING :
            if ( p->pStr )
                *p->pStr = String( (sal_Char*)pData, osl_getThreadTextEncoding() );
            else
                p->pStr = new String( (sal_Char*)pData, osl_getThreadTextEncoding() );
            break;
        default :
            OSL_FAIL( "unbekannter AsyncType" );
            return;
    }
    p->bValid = TRUE;
    p->Broadcast( ScHint( SC_HINT_DATACHANGED, ScAddress(), NULL ) );

    const ScDocument** ppDoc = (const ScDocument**) p->pDocs->GetData();
    USHORT nCount = p->pDocs->Count();
    for ( USHORT j=0; j<nCount; j++, ppDoc++ )
    {
        ScDocument* pDoc = (ScDocument*)*ppDoc;
        pDoc->TrackFormulas();
        pDoc->GetDocumentShell()->Broadcast( SfxSimpleHint( FID_DATACHANGED ) );
        pDoc->ResetChanged( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB) );
    }
}



void ScAddInAsync::RemoveDocument( ScDocument* pDocumentP )
{
    USHORT nPos = theAddInAsyncTbl.Count();
    if ( nPos )
    {
        const ScAddInAsync** ppAsync =
            (const ScAddInAsync**) theAddInAsyncTbl.GetData() + nPos - 1;
        for ( ; nPos-- >0; ppAsync-- )
        {   // rueckwaerts wg. Pointer-Aufrueckerei im Array
            ScAddInDocs* p = ((ScAddInAsync*)*ppAsync)->pDocs;
            USHORT nFoundPos;
            if ( p->Seek_Entry( pDocumentP, &nFoundPos ) )
            {
                p->Remove( nFoundPos );
                if ( p->Count() == 0 )
                {   // dieses AddIn wird nicht mehr benutzt
                    ScAddInAsync* pAsync = (ScAddInAsync*)*ppAsync;
                    theAddInAsyncTbl.Remove( nPos );
                    delete pAsync;
                    ppAsync = (const ScAddInAsync**) theAddInAsyncTbl.GetData()
                        + nPos;
                }
            }
        }
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
