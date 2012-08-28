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


extern "C" {
void CALLTYPE ScAddInAsyncCallBack( double& nHandle, void* pData )
{
    ScAddInAsync::CallBack( sal_uLong( nHandle ), pData );
}
}



ScAddInAsync::ScAddInAsync() :
    SvtBroadcaster(),
    nHandle( 0 )
{   // nur fuer aSeekObj !
}



ScAddInAsync::ScAddInAsync(sal_uLong nHandleP, FuncData* pFuncData, ScDocument* pDoc) :
    SvtBroadcaster(),
    pStr( NULL ),
    mpFuncData(pFuncData),
    nHandle( nHandleP ),
    meType(pFuncData->GetAsyncType()),
    bValid( false )
{
    pDocs = new ScAddInDocs();
    pDocs->insert( pDoc );
    theAddInAsyncTbl.insert( this );
}



ScAddInAsync::~ScAddInAsync()
{
    // aSeekObj does not have that, handle 0 does not exist otherwise
    if ( nHandle )
    {
        // in dTor because of theAddInAsyncTbl.DeleteAndDestroy in ScGlobal::Clear
        mpFuncData->Unadvice( (double)nHandle );
        if ( meType == PTR_STRING && pStr )      // include type comparison because of union
            delete pStr;
        delete pDocs;
    }
}



ScAddInAsync* ScAddInAsync::Get( sal_uLong nHandleP )
{
    ScAddInAsync* pRet = 0;
    aSeekObj.nHandle = nHandleP;
    ScAddInAsyncs::iterator it = theAddInAsyncTbl.find( &aSeekObj );
    if ( it != theAddInAsyncTbl.end() )
        pRet = *it;
    aSeekObj.nHandle = 0;
    return pRet;
}



void ScAddInAsync::CallBack( sal_uLong nHandleP, void* pData )
{
    ScAddInAsync* p;
    if ( (p = Get( nHandleP )) == NULL )
        return;

    if ( !p->HasListeners() )
    {
        // not in dTor because of theAddInAsyncTbl.DeleteAndDestroy in ScGlobal::Clear
        theAddInAsyncTbl.erase( p );
        delete p;
        return ;
    }
    switch ( p->meType )
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
    p->bValid = sal_True;
    p->Broadcast( ScHint( SC_HINT_DATACHANGED, ScAddress(), NULL ) );

    for ( ScAddInDocs::iterator it = p->pDocs->begin(); it != p->pDocs->end(); ++it )
    {
        ScDocument* pDoc = *it;
        pDoc->TrackFormulas();
        pDoc->GetDocumentShell()->Broadcast( SfxSimpleHint( FID_DATACHANGED ) );
    }
}



void ScAddInAsync::RemoveDocument( ScDocument* pDocumentP )
{
    if ( !theAddInAsyncTbl.empty() )
    {
        for( ScAddInAsyncs::reverse_iterator iter1 = theAddInAsyncTbl.rbegin(); iter1 != theAddInAsyncTbl.rend(); ++iter1 )
        {   // backwards because of pointer-movement in array
            ScAddInAsync* pAsync = *iter1;
            ScAddInDocs* p = pAsync->pDocs;
            ScAddInDocs::iterator iter2 = p->find( pDocumentP );
            if( iter2 != p->end() )
            {
                p->erase( iter2 );
                if ( p->empty() )
                {   // this AddIn is not used anymore
                    theAddInAsyncTbl.erase( --(iter1.base()) );
                    delete pAsync;
                }
            }
        }
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
