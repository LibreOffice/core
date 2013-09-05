/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sfx2/objsh.hxx>

#include "adiasync.hxx"
#include "brdcst.hxx"
#include "global.hxx"
#include "document.hxx"
#include "sc.hrc"       // FID_DATACHANGED
#include <osl/thread.h>

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
            OSL_FAIL( "unknown AsyncType" );
            return;
    }
    p->bValid = sal_True;
    p->Broadcast( ScHint(SC_HINT_DATACHANGED, ScAddress()) );

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
